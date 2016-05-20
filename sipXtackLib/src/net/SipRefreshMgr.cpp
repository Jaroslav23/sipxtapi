//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdlib.h>
#include <limits.h>

#if defined(_VXWORKS)
#include <taskLib.h>
#include <netinet/in.h>
#endif

#include "os/OsStatus.h"
#include "os/OsConfigDb.h"
#include "os/OsEventMsg.h"
#include "os/OsLock.h"
#include "os/OsDateTime.h"
#include "os/OsQueuedEvent.h"
#include "os/OsTimer.h"
#include "os/OsRWMutex.h"
#include "os/OsReadLock.h"
#include "os/OsWriteLock.h"
#include "utl/UtlTokenizer.h"
#include <utl/UtlHashBagIterator.h>
#include "net/SipLine.h"
#include "net/SipLineStateEventListener.h"
#include "net/SipLineMgr.h"
#include "net/SipRefreshMgr.h"
#include "net/SipMessageEvent.h"
#include "net/NameValueTokenizer.h"
#include "net/SipObserverCriteria.h"
#include "net/Url.h"
#include "net/SipUserAgent.h"
#include "net/SipTransport.h"

#define UNREGISTER_CSEQ_NUMBER   2146483648   // 2^31 - 1,000,000
#define MIN_REFRESH_TIME_SECS       20 // Floor for re-subscribes/re-registers
#define CALL_ID_PREFIX "s" // refresh manager uses s

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
SipRefreshMgr::SipRefreshMgr(SipLineStateEventListener *pListener)
: OsServerTask("SipRefreshMgr-%d")
, m_pLineMgr(NULL)
, m_mutex(OsMutex::Q_FIFO)
, m_pSipUserAgent(NULL)
, m_pLineListener(pListener)
, m_callIdGenerator(CALL_ID_PREFIX)
, m_defaultRegistryPeriod(3600)
{
}

SipRefreshMgr::~SipRefreshMgr()
{ 
   waitUntilShutDown();

   // delete all unfired timers and their SipMessages
   UtlHashBagIterator timerIterator(m_timerBag) ;
   while (OsTimer* pTimer = (OsTimer*) timerIterator())
   {
      SipMessage *pMessage = (SipMessage *)pTimer->getUserData();
      // get rid of them
      delete pMessage;
      delete pTimer;
   }
   m_timerBag.removeAll();
}

void SipRefreshMgr::setSipUserAgent(SipUserAgent *pSipUserAgent)
{
   m_pSipUserAgent = pSipUserAgent;
}

void SipRefreshMgr::startRefreshMgr()
{
   if (!isStarted())
   {
      OsSysLog::add(FAC_LINE_MGR, PRI_INFO, "SipRefreshMgr is starting...");

      // start the thread
      start();

      m_pSipUserAgent->addMessageObserver(*(this->getMessageQueue()),
         SIP_REGISTER_METHOD,
         TRUE,       // want to get requests
         TRUE,       // want to get responses
         TRUE,       // Incoming messages
         FALSE);     // Don't want to see out going messages
   }
}

UtlBoolean SipRefreshMgr::newRegisterMsg(const Url& fromUrl,
                                         const Url& contactUri,
                                         UtlBoolean bAllowContactOverride,
                                         SIP_TRANSPORT_TYPE preferredTransport,
                                         int registryPeriodSeconds)
{
   if (!isDuplicateRegister(fromUrl))
   {
      syslog(FAC_REFRESH_MGR, PRI_DEBUG, "adding registration:\nurl=%s\nperiod=%d",
         fromUrl.toString().data(), registryPeriodSeconds);

      Url requestUri = fromUrl.getUri(); // REGISTER request uri
      requestUri.setUserId(NULL);
      requestUri.setPassword(NULL); // REGISTER request uri cannot contain userinfo part of sip uri
      UtlString registerCallId = m_callIdGenerator.getNewCallId();

      registerUrl(fromUrl, // from
                  fromUrl, // to                    
                  requestUri, // sip request uri
                  contactUri.toString(), // contact
                  bAllowContactOverride,
                  preferredTransport,
                  registerCallId,
                  registryPeriodSeconds);

      return TRUE;
   }
   else
   {
      syslog(FAC_REFRESH_MGR, PRI_ERR, "unable to add new registration (dup):\nurl=%s\nperiod=%d",
         fromUrl.toString().data(), registryPeriodSeconds);
   }

   return FALSE;
}

void SipRefreshMgr::reRegister(const Url& fromUrl)
{
   SipMessage *oldMsg = m_registerList.isSameFrom(fromUrl);
   if (oldMsg)
   {
      SipMessage newMsg(*oldMsg);
      newMsg.incrementCSeqNumber();

      // Clear the DNS field, so that we retry DNS-SRV before resending.  
      // This should be performed for all failure cases, except for
      // auth challenges
      newMsg.clearDNSField() ;
      newMsg.resetTransport() ;

      addToRegisterList(&newMsg);

      if (sendRequest(newMsg , SIP_REGISTER_METHOD) != OS_SUCCESS)
      {
         removeFromRegisterList(&newMsg);
      }
   }
}

void SipRefreshMgr::unRegisterUser(const Url& fromUrl)
{
   SipMessage sipMsg;
   if (isDuplicateRegister(fromUrl, sipMsg))
   {
      //dont set a common expires - then you need to send * in contact field
      //sipMsg.setExpiresField(0);
      UtlString contactField;
      sipMsg.getContactField(0,contactField);
      Url contact(contactField);
      contact.setFieldParameter(SIP_EXPIRES_FIELD,"0");
      sipMsg.setContactField(contact.toString());
      sipMsg.removeHeader(SIP_EXPIRES_FIELD,0);

      Url lineUri = SipLine::getLineUri(fromUrl);

      if (m_pLineListener) 
      {
         m_pLineListener->OnLineUnregistering(SipLineStateEvent(lineUri.toString(), SIPXTACK_LINESTATE_UNREGISTERING_NORMAL));
      }

      // clear out any pending register requests
      removeAllFromRequestList(&sipMsg);
      sendRequest(sipMsg, SIP_REGISTER_METHOD);
      addToRegisterList(&sipMsg);
   }
}

void SipRefreshMgr::deleteUser(const Url& fromUrl)
{
  SipMessage sipMsg;
  if (isDuplicateRegister(fromUrl, sipMsg))
  {
    //dont set a common expires - then you need to send * in contact field
    //sipMsg.setExpiresField(0);
    UtlString contactField;
    sipMsg.getContactField(0,contactField);
    Url contact(contactField);
    contact.setFieldParameter(SIP_EXPIRES_FIELD,"0");
    sipMsg.setContactField(contact.toString());
    sipMsg.removeHeader(SIP_EXPIRES_FIELD,0);
    // Don't let prepareContact override the contact field - that will destroy the expires filed parameter
    sipMsg.allowContactOverride(FALSE);

    Url lineUri = SipLine::getLineUri(fromUrl);

    if (m_pLineListener) 
    {
      m_pLineListener->OnLineUnregistering(SipLineStateEvent(lineUri.toString(), SIPXTACK_LINESTATE_UNREGISTERING_NORMAL));
    }

    // clear out any pending register requests
    removeAllFromRequestList(&sipMsg);
  }
}

UtlBoolean SipRefreshMgr::isDuplicateRegister(const Url& fromUrl, 
                                              SipMessage &oldMsg)
{
   OsLock lock(m_mutex); // scoped lock

   // call copy constructor on the oldMsg
   SipMessage* pduplicate = m_registerList.isSameFrom(fromUrl);
   if ( pduplicate != NULL )
   {
      oldMsg = *pduplicate;
      return true;
   }
   return false;
}

UtlBoolean SipRefreshMgr::isDuplicateRegister(const Url& fromUrl)
{
   OsLock lock(m_mutex); // scoped lock

   SipMessage* oldMsg = m_registerList.isSameFrom(fromUrl);

   return oldMsg != NULL;
}

OsStatus 
SipRefreshMgr::sendRequest (
                            SipMessage& request,
                            const char* method )
{    
   OsStatus    retval = OS_UNSPECIFIED ;   // Sucess of operation
   UtlString    methodName(method) ;        // Method name fo request
   int         refreshPeriod = -1 ;        // Refresh period used when resubscribing
   UtlBoolean   bIsUnregOrUnsub ;           // Is this an unregister or unsubscribe?

   // Reset the transport data and the via fields
   request.resetTransport();
   request.removeLastVia();
   request.setDateField();

   bIsUnregOrUnsub = isExpiresZero(&request) ;

   if ( !m_pSipUserAgent->send( request, getMessageQueue() ) )
   {
      UtlString toField ;    
      request.getToField(&toField) ;

      syslog(FAC_REFRESH_MGR, PRI_ERR, "unable to send %s message (send failed):\nto: %s",
         method, toField.data()) ;

      Url toUrl;
      request.getToUrl(toUrl);
      Url lineUri = SipLine::getLineUri(toUrl);

      if ( methodName.compareTo(SIP_REGISTER_METHOD) == 0 && !isExpiresZero(&request)) 
      {
         if (m_pLineMgr)
         {
            m_pLineMgr->setStateForLine(lineUri, SipLine::LINE_STATE_FAILED);
         }

         if (m_pLineListener) 
         {
            m_pLineListener->OnLineRegisterFailed(SipLineStateEvent(lineUri.toString(), SIPXTACK_LINESTATE_REGISTER_FAILED_COULD_NOT_CONNECT));
         }
         rescheduleAfterTime(&request, FAILED_PERCENTAGE_TIMEOUT);
      }
      else if ( methodName.compareTo(SIP_REGISTER_METHOD) == 0 && isExpiresZero(&request)) 
      {
         if (m_pLineListener)
         {
            m_pLineListener->OnLineUnregisterFailed(SipLineStateEvent(lineUri.toString(), SIPXTACK_LINESTATE_UNREGISTER_FAILED_COULD_NOT_CONNECT));
         }
      }

      // @JC Added Comments: create a message on the queue with a quarter
      // lease period timeout if the timer triggers and we've not received
      // a good response from the server within FAILED_PERCENTAGE_TIMEOUT
      // secs resubscribe
      // LBA: Whats the point???:
      /*
      SipMessage* message = new SipMessage( request );

      if (!message)
         assert(0);

      if ( request.getResponseListenerData() )
      {
         message->setResponseListenerData( request.getResponseListenerData() );
      }
      */
   }
   else
   {
      int sequenceNum = 0;
      UtlString tmpMethod;
      request.getCSeqField(&sequenceNum, &tmpMethod);
      Url toUrl;
      request.getToUrl(toUrl);
      Url lineUri = SipLine::getLineUri(toUrl);
      if ( methodName.compareTo(SIP_REGISTER_METHOD) == 0 && !isExpiresZero(&request)) 
      {
         if (m_pLineListener)
         {
            m_pLineListener->OnLineRegistering(SipLineStateEvent(lineUri.toString(), SIPXTACK_LINESTATE_REGISTERING_NORMAL));
         }
      }
      else if ( methodName.compareTo(SIP_REGISTER_METHOD) == 0 && isExpiresZero(&request)) 
      {

      }
      retval = OS_SUCCESS;
   }

   return retval;
}


void 
SipRefreshMgr::rescheduleRequest(
                                 SipMessage* request,
                                 int secondsFromNow,
                                 const char *method,
                                 int percentage,
                                 UtlBoolean sendImmediate)
{
   UtlString seqMethod;
   SipMessage* listMessage = NULL;
   UtlString methodStr(method);
   int defaultTime = -1; //set according to the requested method to default
   UtlString lineId;

   // Log reschedule attempt
   syslog(FAC_REFRESH_MGR, PRI_DEBUG, "rescheduling %s request:\nsecs=%d\npercent=%d\nsendNow=%d",
      method, secondsFromNow, percentage, sendImmediate) ;

   if (!request)
      assert(0);

   if ( methodStr.compareTo(SIP_REGISTER_METHOD) == 0 )
   {
      OsLock lock(m_mutex); // scoped lock
      listMessage = m_registerList.getDuplicate(request);
      // May not have a To tag set in the list because it was sent the first time
      if ( !listMessage )
      {
         UtlString fromUri;
         request->getFromUri(&fromUri);
         Url uri(fromUri);
         uri.removeAngleBrackets();
         if ( !fromUri.isNull() )
         {
            SipMessage sipMsg;
            if ( isDuplicateRegister(uri, sipMsg) )
            {
               listMessage = m_registerList.getDuplicate(&sipMsg);
            }
         }
      }
      defaultTime = m_defaultRegistryPeriod;
   }

   // if it is an immediate send then it is either a re-register or unregister
   // The Request has already incremented CSEQ number is that case so don't
   // increment, but increase the number only to the message that is added
   // to the list because that will be used for next timer register in case
   // of re-register in case of unregister, the message will be deleted from 
   // the list upon getting a response
   if ( !sendImmediate )
   {
      request->incrementCSeqNumber();

      // Clear the DNS field, so that we retry DNS-SRV before resending.  
      // This should be performed for all failure cases, except for
      // auth challenges
      request->clearDNSField() ;
      request->resetTransport() ;
   }

   // Remove the old list message and add the new one
   if ( methodStr.compareTo(SIP_REGISTER_METHOD) == 0 )
      addToRegisterList(request);        

   // There will always be a copy - if there is no copy then don't reschedule 
   // because reregister may have removed the copy deliberately.
   if ( secondsFromNow > 0 )
   {
      request->setSendProtocol(OsSocket::UNKNOWN);
      request->setTimesSent(0);
      // add the request back to the list
      UtlString contact;
      request->getContactEntry(0,&contact);
      if ( contact.isNull() )
      {
         UtlString toField;
         request->getToField(&toField);
         Url toFieldTmp(toField);
         UtlString contactStr = buildContactField(toFieldTmp, lineId, NULL);
         request->setContactField(contactStr.data());
      }

      // empty the via headers
      while ( request->removeHeader(SIP_VIA_FIELD, 0) )
      {}

      // Make a copy for the timer
      SipMessage* timerRegisterMessage = new SipMessage(*request);

      OsTimer* timer = new OsTimer(&mIncomingQ,
         (int)timerRegisterMessage);
      m_timerBag.insert(timer);

      int maxSipTransactionTimeSecs = 
         (m_pSipUserAgent->getSipStateTransactionTimeout()/1000);

      secondsFromNow = (secondsFromNow * percentage)/100;

      // ensure that the time that the transaction times out
      // is at least the max transaction time (preventing duplicate
      // retransmits
      if ( secondsFromNow < MIN_REFRESH_TIME_SECS )
         secondsFromNow = MIN_REFRESH_TIME_SECS;

      // check for minumum and maximum values.
      if ( !sendImmediate )
      {
         //mseconds to seconds
         if ( secondsFromNow < MIN_REFRESH_TIME_SECS )
         {
            secondsFromNow = MIN_REFRESH_TIME_SECS;
         }
         else if ( secondsFromNow > defaultTime )
         {
            secondsFromNow = (defaultTime * percentage)/100;
         }
      }

      // Log reschedule attempt
      syslog(FAC_REFRESH_MGR, PRI_DEBUG, "rescheduled %s in %d second(s)",
         method, secondsFromNow) ;

      OsTime timerTime(secondsFromNow, 0);        
      timer->oneshotAfter(timerTime);
   }
   return;
}


void 
SipRefreshMgr::processResponse(
                               const OsMsg& eventMessage,
                               SipMessage *request)
{
   SipMessage* response = (SipMessage*)((SipMessageEvent&)eventMessage).getMessage();
   SipMessage* requestCopy = m_registerList.getRequestFor(response);
   if (requestCopy)
   {
      requestCopy = new SipMessage(*requestCopy);
   }


   SipMessage* responseCopy = new SipMessage(*response);

   if (!request)
      assert(0);

   UtlString method;
   requestCopy->getRequestMethod( &method) ;

   // ensure that this is a response first
   if ( responseCopy->isResponse() )
   {
      int responseCode = responseCopy->getResponseStatusCode();

      if ( request && responseCode < SIP_2XX_CLASS_CODE )
      {   
         // provisional response codes
      }
      else if ( ( (responseCode >= SIP_2XX_CLASS_CODE) && 
         (responseCode < SIP_3XX_CLASS_CODE) ) )
      {
         // Success Class response 2XX
         removeAllFromRequestList(response);
         processOKResponse(responseCopy, requestCopy );
      }
      else  // failure case 
      {
         removeAllFromRequestList(response);
         // unregister/unsubscribe?
         if ( isExpiresZero(requestCopy) )
         {
            // reschedule only if expires value id not zero otherwise 
            // it means we just did an unregister
            if ( method.compareTo(SIP_REGISTER_METHOD) == 0 )
            {
               Url toUrl;
               requestCopy->getToUrl(toUrl);
               Url lineUri = SipLine::getLineUri(toUrl);
               UtlString responseStatusText;
               response->getResponseStatusText(&responseStatusText);
               if (responseCode == 401 || responseCode == 403 || responseCode == 407)
               {
                  if (m_pLineListener)
                  {
                     m_pLineListener->OnLineUnregisterFailed(SipLineStateEvent(lineUri.toString(),
                        SIPXTACK_LINESTATE_UNREGISTER_FAILED_NOT_AUTHORIZED,
                        responseCode,
                        responseStatusText));
                  }
               }
               else if (responseCode == 408)
               {
                  if (m_pLineListener)
                  {
                     m_pLineListener->OnLineUnregisterFailed(SipLineStateEvent(lineUri.toString(),
                        SIPXTACK_LINESTATE_UNREGISTER_FAILED_TIMEOUT,
                        responseCode,
                        responseStatusText));
                  }
               }
               else
               {
                  if (m_pLineListener)
                  {
                     m_pLineListener->OnLineUnregisterFailed(SipLineStateEvent(lineUri.toString(),
                        SIPXTACK_LINESTATE_CAUSE_UNKNOWN,
                        responseCode,
                        responseStatusText));
                  }
               }
            }
         }
         else // it is a register or subscribe
         {
            if ( method.compareTo(SIP_REGISTER_METHOD) == 0 )
            {
               Url toUrl;
               requestCopy->getToUrl(toUrl);
               Url lineUri = SipLine::getLineUri(toUrl);

               if (m_pLineMgr)
               {
                  m_pLineMgr->setStateForLine(lineUri, SipLine::LINE_STATE_FAILED);
               }

               UtlString responseStatusText;
               response->getResponseStatusText(&responseStatusText);

               if (responseCode == 401 || responseCode == 403 || responseCode == 407)
               {
                  if (m_pLineListener)
                  {
                     m_pLineListener->OnLineRegisterFailed(SipLineStateEvent(lineUri.toString(),
                        SIPXTACK_LINESTATE_REGISTER_FAILED_NOT_AUTHORIZED,
                        responseCode,
                        responseStatusText));
                  }
               }
               else if (responseCode == 408)
               {
                  if (m_pLineListener)
                  {
                     m_pLineListener->OnLineRegisterFailed(SipLineStateEvent(lineUri.toString(),
                        SIPXTACK_LINESTATE_REGISTER_FAILED_TIMEOUT,
                        responseCode,
                        responseStatusText));
                  }
                  rescheduleAfterTime(requestCopy, FAILED_PERCENTAGE_TIMEOUT );
               }
               else
               {
                  if (m_pLineListener)
                  {
                     m_pLineListener->OnLineRegisterFailed(SipLineStateEvent(lineUri.toString(),
                        SIPXTACK_LINESTATE_CAUSE_UNKNOWN,
                        responseCode,
                        responseStatusText));
                  }
                  // Reschedule in case of failure
                  rescheduleAfterTime(requestCopy, FAILED_PERCENTAGE_TIMEOUT );
               }
            }
         }
      }
   }
   else
   {
      Url toUrl;
      requestCopy->getToUrl(toUrl);
      Url lineUri = SipLine::getLineUri(toUrl);

      if (m_pLineMgr)
      {
         m_pLineMgr->setStateForLine(lineUri, SipLine::LINE_STATE_FAILED);
      }
      if (m_pLineListener)
      {
         m_pLineListener->OnLineRegisterFailed(SipLineStateEvent(lineUri.toString(), SIPXTACK_LINESTATE_CAUSE_UNKNOWN));
      }
   }

   delete responseCopy;
   delete requestCopy;
}

void 
SipRefreshMgr::processOKResponse(
                                 SipMessage* response, 
                                 SipMessage* request )
{
   int responseRefreshPeriod = -1;

   if (!request)
      assert(0);

   if ( !response->getExpiresField(&responseRefreshPeriod) )
   {   
      // this method looks at the request/response pair
      // the response may have multiple contacts so it searched
      // for the expires header corresponding to the request
      parseContactFields( response, request, responseRefreshPeriod );
   }
   int requestRefreshPeriod = -1;
   if ( !request->getExpiresField(&requestRefreshPeriod) )
   {
      // to get expires value @JC whi request 2 times
      parseContactFields( request, request, requestRefreshPeriod );
   }

   //get to To Tag from the 200 ok response and add it to the request
   UtlString toAddr;
   UtlString toProto;
   int toPort;
   UtlString toTag;
   response->getToAddress(&toAddr, &toPort, &toProto, NULL, NULL, &toTag);

   UtlString method;
   request->getRequestMethod( &method) ;

   if ( method.compareTo(SIP_REGISTER_METHOD) == 0 )
   {
      // Reschedule only if expires value != 0, otherwise it means we just did an unregister
      if ( requestRefreshPeriod == 0 )
      {

         // if its an unregister, remove all related messages 
         // from the appropriate request list
         response->setCSeqField(-1, method);
         // TODO - should also destroy the timer now

         Url toUrl;
         request->getToUrl(toUrl);
         Url lineUri = SipLine::getLineUri(toUrl);

         if (m_pLineMgr)
         {
            m_pLineMgr->setStateForLine(lineUri, SipLine::LINE_STATE_DISABLED);
         }
         if (m_pLineListener)
         {
            int responseCode = response->getResponseStatusCode();
            UtlString responseStatusText;
            response->getResponseStatusText(&responseStatusText);

            m_pLineListener->OnLineUnregistered(SipLineStateEvent(lineUri.toString(),
               SIPXTACK_LINESTATE_UNREGISTERED_NORMAL,
               responseCode,
               responseStatusText));
         }
      } 
      else
      {
         // we don't set to tag even if it arrives in 200 response to REGISTER.
         // to,from to tags with callid uniquely identify a sip dialog. But REGISTER
         // doesn't create a sip dialog. Out of dialog requests must not have to tag.
         Url toUrl;
         request->getToUrl(toUrl);
         Url lineUri = SipLine::getLineUri(toUrl);//get lineUri for toUrl

         // extract the Message body and pass to apps
         const char *bodyBytes = NULL;
         int   nBodySize = 0;
         const HttpBody *body = response->getBody();
         if (body)
         {
            body->getBytes( &bodyBytes, &nBodySize );
         }
         if (m_pLineMgr)
         {
            m_pLineMgr->setStateForLine(lineUri, SipLine::LINE_STATE_REGISTERED);
         }
         if (m_pLineListener)
         {
            UtlString responseStatusText;
            response->getResponseStatusText(&responseStatusText);

            m_pLineListener->OnLineRegistered(SipLineStateEvent(lineUri.toString(),
               SIPXTACK_LINESTATE_REGISTERED_NORMAL,
               response->getResponseStatusCode(),
               responseStatusText));
         }

         if (responseRefreshPeriod > 0)
         {
            rescheduleRequest(request, responseRefreshPeriod, SIP_REGISTER_METHOD);
         }
         else
         {
            // could not find expires in 200 ok response , reschedule after default time
            rescheduleAfterTime(request);
         }
      }
   } else // subscribe
   {
      // reschedule according to expires value
      if ( requestRefreshPeriod == 0 )
      {
         // if its an unregister, remove all related messasges 
         // from the appropriate request list
         response->setCSeqField(-1, method);
         removeAllFromRequestList(response);
         // TODO - should also destroy the timer now
      }
      else if ( responseRefreshPeriod > 0 )
      {
         if ( !toTag.isNull() )
         {
            request->setToFieldTag(toTag);
         }

         rescheduleRequest(
            request, 
            responseRefreshPeriod, 
            SIP_SUBSCRIBE_METHOD);
      }
      else 
      {
         // could not find expires in 200 ok response , reschedule after default time   
         // copying from response (this is why we set the To Field
         request->setToFieldTag(toTag);
         rescheduleAfterTime(request);
      }
   }
   return;
}

void 
SipRefreshMgr::parseContactFields(
                                  SipMessage* registerResponse, 
                                  SipMessage* requestMessage, 
                                  int &serverRegPeriod)
{
   // get the request contact uri ...so that we can find out 
   // the expires subfield value for this contact from the list 
   // of contacts returned by the registration server
   UtlString requestContactEntry;
   requestMessage->getContactEntry(0 , &requestContactEntry);
   Url requestContactUrl(requestContactEntry);
   UtlString requestContactIdentity;
   requestContactUrl.getIdentity(requestContactIdentity);

   UtlString contactField;
   int indexContactField = 0;

   while ( registerResponse->getContactEntry(indexContactField , &contactField) )
   {
      Url returnedContact(contactField);
      UtlString returnedIdentity;
      returnedContact.getIdentity(returnedIdentity);

      if ( returnedIdentity.compareTo(requestContactIdentity) == 0 )
      {
         UtlString subfieldText;
         int subfieldIndex = 0;
         UtlString subfieldName;
         UtlString subfieldValue;
         NameValueTokenizer::getSubField(contactField.data(), subfieldIndex, ";", &subfieldText);
         while ( !subfieldText.isNull() )
         {
            NameValueTokenizer::getSubField(subfieldText.data(), 0, "=", &subfieldName);
            NameValueTokenizer::getSubField(subfieldText.data(), 1, "=", &subfieldValue);
            subfieldName.toUpper();
            if ( subfieldName.compareTo(SIP_EXPIRES_FIELD) == 0 )
            {

               //see if more than one token in the expire value
               NameValueTokenizer::getSubField(
                  subfieldValue, 1,
                  " \t:;,", &subfieldText);

               // if not ...time is in seconds
               if ( subfieldText.isNull() )
               {
                  serverRegPeriod = atoi(subfieldValue);
               }
               // If there is more than one token assume it is a text date
               else
               {
                  // Get the expiration date
                  long dateExpires = OsDateTime::convertHttpDateToEpoch(subfieldValue);
                  long dateSent = 0;
                  // If the date was not set in the message
                  if ( !registerResponse->getDateField(&dateSent) )
                  {
                     // Assume date sent is now
                     dateSent = OsDateTime::getSecsSinceEpoch();
                  }
                  serverRegPeriod = dateExpires - dateSent;
               }
               break;
            }
            subfieldIndex++;
            NameValueTokenizer::getSubField(contactField.data(), subfieldIndex, ";", &subfieldText);
         }
      }
      indexContactField ++;
   }
   return ;

}

void SipRefreshMgr::registerUrl(const Url& fromUrl,
                                const Url& toUrl,
                                const Url& requestUri,
                                const UtlString& contactUrl,
                                UtlBoolean bAllowContactOverride,
                                SIP_TRANSPORT_TYPE preferredTransport,
                                const UtlString& callId,
                                int registerPeriod)
{
   SipMessage regMessage;
   int startSequence = 1;
   Url newFromUrl(fromUrl);

   // add Tag to from field
   newFromUrl.setFieldParameter("tag", createTagValue());

   regMessage.setRegisterData (
      newFromUrl.toString(), // from
      toUrl.toString(),   // to
      requestUri.toString(),// uri
      contactUrl,         // contact
      callId,             // callid
      startSequence,
      registerPeriod >= 0 ? registerPeriod : m_defaultRegistryPeriod );
   regMessage.allowContactOverride(bAllowContactOverride);
   regMessage.setPreferredTransport(SipTransport::getSipTransport(preferredTransport));

   // Add to the register list
   addToRegisterList(&regMessage);

   if (sendRequest(regMessage , SIP_REGISTER_METHOD) != OS_SUCCESS)
   {
      // if we couldn't send, go ahead and remove the register request from the list
      removeFromRegisterList(&regMessage);
      syslog(FAC_REFRESH_MGR, PRI_ERR, "unable to send register sip message: \nfromUrl: %s\ntoUrl: %s\nrequestUri: %s\ncallId %s",
         fromUrl.toString().data(), toUrl.toString().data(), requestUri.toString().data(), callId.data());
   }
}


UtlBoolean 
SipRefreshMgr::handleMessage( OsMsg& eventMessage )
{
   UtlBoolean messageProcessed = FALSE;
   int msgType = eventMessage.getMsgType();
   int msgSubType = eventMessage.getMsgSubType();
   UtlString method;

   if ( msgType == OsMsg::PHONE_APP )
   {
      SipMessage* sipMsg = (SipMessage*)((SipMessageEvent&)eventMessage).getMessage();
      int messageType = ((SipMessageEvent&)eventMessage).getMessageStatus();
      UtlString callid;
      int cseq;
      sipMsg->getCallIdField(&callid);
      sipMsg->getCSeqField(&cseq, &method);        

      // if transport error and no response from remote machine. Unable to send to remote host
      if ( !sipMsg->isResponse() && messageType == SipMessageEvent::TRANSPORT_ERROR )
      {            
         SipMessage * msgInList = NULL;

         // Log Failures
         syslog(FAC_REFRESH_MGR, PRI_ERR, "unable to send %s (transport):\ncallid=%s",
            method.data(), callid.data()) ;

         //reschedule only if expires value is not zero otherwise it means we just did an unregister
         if ( !isExpiresZero(sipMsg) )
         {
            if (msgInList)
            {
               // try again after default time out
               rescheduleAfterTime(msgInList, FAILED_PERCENTAGE_TIMEOUT);
            }
            else
            {
               // It is possible to have msgInList=NULL if removed by another task+ICMP
               // osPrintf("SipRefreshMgr::handleMessage-L1274: msgInList=NULL; why ??*********");
               // do nothing here.
               ;
            }
         }
         messageProcessed = TRUE;
      }
      // If this is a response,
      else if ( sipMsg->isResponse() )
      {
         SipMessage *request = NULL;
         SipMessage *requestFound = NULL;

         // Is this a register msg?
         {
            OsLock lock(m_mutex); // scoped lock
            // Find the request which goes with this response
            requestFound = m_registerList.getRequestFor(sipMsg);
            //make a dupe
            if ( requestFound )
               request = new SipMessage(*requestFound);
         }

         if ( request )
         {
            UtlBoolean retryWithAuthentication = FALSE;
            request->getRequestMethod( &method) ;

            if ( messageType == SipMessageEvent::AUTHENTICATION_RETRY )
            {
               syslog(FAC_REFRESH_MGR, PRI_INFO, "authentication requested for %s request:\ncallid=%s",
                  method.data(), callid.data()) ;

               if ( strcmp(method.data(), SIP_REGISTER_METHOD) == 0 )
               {
                  // Find the request which goes with this response
                  SipMessage* request = m_registerList.getRequestFor(sipMsg);

                  // increment the CSeq number in the stored request
                  if ( request )
                  {
                     request->incrementCSeqNumber();
                     addToRegisterList(request);

                     retryWithAuthentication = TRUE;
                  }
               } 
            }

            if ( request && retryWithAuthentication == FALSE )
            {
               processResponse(eventMessage, request);
            }                
         } 
         else
         {

            // Bob 2/10/03 Do not complain if we cannot find the 
            // message.  Because we add observers for both requests
            // and incoming messages, we *WILL* receive duplicate
            // responses- and yes, we won't find them here.
            /*
            // Report that we were unable to find this request
            UtlString response ;
            int      respLen ;
            UtlString msgContents ;

            // Log Failure
            sipMsg->getBytes(&response, &respLen) ;                
            dumpMessageLists(msgContents) ;                
            syslog(FAC_REFRESH_MGR, PRI_ERR, "unable to find request for %s response:\ncallid=%s\nResponse:\n%s\nLists:\n%s",
            method.data(), callid.data(), response.data(), msgContents.data()) ;
            */

         }

         if ( request )
         {
            delete request;
            request = NULL;
         }
      }//end if isresponse

      messageProcessed = TRUE;
   } 
   else if ( (msgType == OsMsg::OS_EVENT) && (msgSubType == OsEventMsg::NOTIFY) )
   {   
      // A timer expired
      SipMessage* sipMessage;
      OsTimer* timer;
      int protocolType;

      ((OsEventMsg&)eventMessage).getUserData((int&)sipMessage);
      ((OsEventMsg&)eventMessage).getEventData((int&)timer);

      if ( timer )
      {
         // remove timer from mTimerBag
         m_timerBag.removeReference(timer);
         delete timer;
         timer = NULL;
      }


      if ( sipMessage )
      {
         UtlString callId;
         protocolType = sipMessage->getSendProtocol();
         sipMessage->getCallIdField(&callId);
         sipMessage->getRequestMethod(&method);

         // Log Timeout
         syslog(FAC_REFRESH_MGR, PRI_DEBUG, "timeout for %s:\ncallid=%s",
            method.data(), callId.data())  ;

         // check if a duplicate request is in the list, 
         // if not then it means that it was unregistered 
         // before the timer expired
         UtlString fromUri;
         sipMessage->getFromUri(&fromUri);
         Url uri(fromUri);
         uri.removeAngleBrackets();

         SipMessage sipMsg;
         if ( !fromUri.isNull() )
         {
            int num;
            UtlString method;
            sipMessage->getCSeqField(&num , &method);

            if ( method.compareTo(SIP_REGISTER_METHOD) == 0 )
            {
               if ( isDuplicateRegister(uri, sipMsg) )
               {
                  int listNum;
                  UtlString listMethod;
                  sipMsg.getCSeqField(&listNum , &listMethod);

                  // check if CSeq is less than what is in the list ..if less, then it is because
                  // reregister must have incremented it and this rescheduling from the previous
                  // msg
                  if ( num >= listNum )
                  {                            
                     if (sendRequest(*sipMessage, SIP_REGISTER_METHOD) != OS_SUCCESS)
                     {
                        removeFromRegisterList(sipMessage);
                     }
                  }
               }
               else
               {
                  syslog(FAC_REFRESH_MGR, PRI_ERR, "unable to refresh %s (not found):\ncallid=%s",
                     method.data(), callId.data()) ;                  
               }
            } 
         }

         // The timer made its own copy of this message.Delete it now that we are done with it.
         delete sipMessage;
         sipMessage = NULL;            

         messageProcessed = TRUE;
      }
   }

   return (messageProcessed);
}

// Appends the message contents of both the mRegisterList and mSubscribeList.
void SipRefreshMgr::dumpMessageLists(UtlString& results)
{
   UtlString temp ;

   // Dump Register List
   results.append("\nRegister List:\n\n") ;
   m_registerList.remove(0) ;
   m_registerList.toString(temp) ;
   results.append(temp) ;
}

UtlBoolean 
SipRefreshMgr::removeFromRegisterList(SipMessage* message)
{
   UtlBoolean bRemovedOk = FALSE;

   OsLock lock(m_mutex); // scoped lock

   if ( m_registerList.remove(message) == FALSE )
   {
   }
   else
      bRemovedOk = TRUE;
   return bRemovedOk;
}

void 
SipRefreshMgr::addToRegisterList(SipMessage *message)
{
   OsLock lock(m_mutex); // scoped lock

   if (m_registerList.getDuplicate( message, TRUE ))
   {
      // osPrintf("****We already have the message in mRegisterList[]");
   }
   else
   {
      SipMessage *msg = new SipMessage (*message);
      m_registerList.add(msg);
   }
}

void 
SipRefreshMgr::rescheduleAfterTime(
                                   SipMessage *request, 
                                   int percentage )
{
   int iOriginalExpiration ;
   UtlString method ;
   int dummy;
   request->getCSeqField(&dummy, &method);

   if (!request)
      assert(0);

   if (request)
      request->getRequestMethod(&method);

   // Figure out expiration
   if (request && !request->getExpiresField(&iOriginalExpiration))
   {
      if ( method.compareTo(SIP_REGISTER_METHOD) == 0 )
      {
         iOriginalExpiration = m_defaultRegistryPeriod ;
      }
   }
   if (request)
   {
      rescheduleRequest( 
         request, 
         iOriginalExpiration, 
         method.data(), 
         percentage);
   }        
}

void SipRefreshMgr::createTagNameValuePair( UtlString& tagNamevaluePair )
{
   // Build a from tag
   char fromTagBuffer[24];
   SNPRINTF(fromTagBuffer, sizeof(fromTagBuffer), "%0x%0x", m_randomNumGenerator.rand(), m_randomNumGenerator.rand());
   tagNamevaluePair = "tag=" ;
   tagNamevaluePair.append(fromTagBuffer);
}

UtlString SipRefreshMgr::createTagValue()
{
   UtlString result;
   char fromTagBuffer[24];
   // generate tag value
   SNPRINTF(fromTagBuffer, sizeof(fromTagBuffer), "%0x%0x", m_randomNumGenerator.rand(), m_randomNumGenerator.rand());
   result = fromTagBuffer;

   return result;
}

UtlString SipRefreshMgr::buildContactField(const Url& registerToField, 
                                           const UtlString& lineId,
                                           const Url* pPreferredContactUri)
{
   UtlString contact;
   UtlString tempContact;
   UtlString displayName;
   UtlString userId;

   // First look at the passed contact uri
   if (pPreferredContactUri)
   {
      UtlString host ;
      int port ;

      pPreferredContactUri->getHostAddress(host) ;
      port = pPreferredContactUri->getHostPort() ;

      if (host.length() > 0)
      {
         tempContact = host ;
         // Only include port number if non-standard
         if ((port != 5060) && (port != PORT_NONE))
         {
            char cPort[32] ;
            SNPRINTF(cPort, sizeof(cPort), "%d", port) ;

            tempContact.append(':') ;
            tempContact.append(cPort) ;
         }
      }               
   }

   // Use default contact from SipUserAgent if preferred is not supplied
   if (tempContact.length() == 0)
   {
      m_pSipUserAgent->getDefaultContactUri(&tempContact);
   }

   // The contact URI does not have the correct urserId information in it ...
   // Get the user ID and display name from To field and stick it in
   Url contactUrl(tempContact);
   registerToField.getDisplayName(displayName);
   registerToField.getUserId(userId);

   contactUrl.setDisplayName(displayName);
   contactUrl.setUserId(userId);

   int index = 0;
   UtlString paramName;
   UtlString paramValue;

   while ( ((Url&)(registerToField)).getFieldParameter( index, paramName , paramValue ) )
   {
      if ( paramName.compareTo(SIP_Q_FIELD, UtlString::ignoreCase) == 0 )
      {
         contactUrl.setFieldParameter(SIP_Q_FIELD, paramValue);
      }
      index ++;
   }
   contact.append(contactUrl.toString().data());

   return contact;
}



// Is the expire field set to zero for the specified request?
UtlBoolean SipRefreshMgr::isExpiresZero(SipMessage* pRequest) 
{
   UtlBoolean bRC = FALSE ;     // Is the expire field zero?

   if ((pRequest != NULL) && !pRequest->isResponse())
   {
      int iExpires = -1;
      if (!pRequest->getExpiresField(&iExpires))
      {
         parseContactFields(pRequest, pRequest, iExpires) ;
      }

      if (iExpires == 0)
         bRC = TRUE ;
   }

   return bRC ;
}

void SipRefreshMgr::setRegistryPeriod(int periodInSeconds)
{
   m_defaultRegistryPeriod = periodInSeconds;
}

void SipRefreshMgr::removeAllFromRequestList(SipMessage* response)
{
   OsLock lock(m_mutex); // scoped lock
   UtlString methodName;
   int seqNum = 0;

   response->getCSeqField(&seqNum, &methodName);
   if (methodName.compareTo(SIP_REGISTER_METHOD) == 0)
   {
      removeAllFromRequestList(response, &m_registerList);
   }
}

void SipRefreshMgr::removeAllFromRequestList(SipMessage* response, SipMessageList* pRequestList)
{
   SipMessage* listMessage = NULL;
   int iteratorHandle = pRequestList->getIterator();
   UtlString methodName;
   int seqNum = 0;

   response->getCSeqField(&seqNum, &methodName);

   while ((listMessage = (SipMessage*) pRequestList->getSipMessageForIndex(iteratorHandle)))
   {
      int requestSeqNum = 0;
      UtlString dummy;
      listMessage->getCSeqField(&requestSeqNum, &dummy);
      if (response->isSameSession(listMessage) && (seqNum == -1 || requestSeqNum <= seqNum) )
      {
         m_registerList.releaseIterator(iteratorHandle);
         m_registerList.remove(listMessage);
         delete listMessage;
         listMessage = NULL;
         iteratorHandle = pRequestList->getIterator();
      }
   }
   pRequestList->releaseIterator(iteratorHandle);
}

void SipRefreshMgr::setLineMgr(SipLineMgr* lineMgr)
{
   m_pLineMgr = lineMgr;
   return;
}
