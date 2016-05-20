//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
//
// Copyright (C) 2005-2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <utl/UtlInit.h>

#include <utl/UtlVoidPtr.h>
#include <utl/UtlHashMapIterator.h>
#include "cp/XCpCallManager.h"
#include <net/SipUserAgent.h>
#include <net/SipContact.h>
#include "tapi/SipXSubscribe.h"
#include "tapi/SipXHandleMap.h"
#include "tapi/SipXCall.h"
#include "tapi/SipXLine.h"
#include "tapi/SipXEvents.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
SipXHandleMap gSubHandleMap;  /**< Global Map of Subscribed (client) event data handles */

// GLOBAL FUNCTIONS


/* ============================ FUNCTIONS ================================= */


void sipxSubscribeClientSubCallback(SipSubscribeClient::SubscriptionState newState,
                                    const char* earlyDialogHandle,
                                    const char* dialogHandle,
                                    void* applicationData,
                                    int responseCode,
                                    const char* responseText,
                                    long expiration,
                                    const SipMessage* subscribeResponse)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxSubscribeClientSubCallback");

   SIPX_SUB hSub = (SIPX_SUB)applicationData;
   SIPX_SUBSCRIPTION_DATA* pData = 
      (SIPX_SUBSCRIPTION_DATA*)sipxSubscribeLookup(hSub, SIPX_LOCK_WRITE, stackLogger);

   if(pData)
   {
      SIPX_INSTANCE_DATA* pInst = pData->pInst;
      assert(pInst);
      SIPX_SUBSTATUS_INFO pInfo;
      pInfo.nSize = sizeof(SIPX_SUBSTATUS_INFO);
      UtlString userAgent;
      UtlString errorState;

      if(subscribeResponse)
      {
         subscribeResponse->getUserAgentField(&userAgent);
      }

      pInfo.szSubServerUserAgent = userAgent;
      pInfo.hSub = hSub;
      // TODO: Should probably set some cause codes based upon
      // the response code from the sip message
      pInfo.cause = SUBSCRIPTION_CAUSE_NORMAL;

      switch(newState)
      {
      case SipSubscribeClient::SUBSCRIPTION_INITIATED: // Early dialog
         pInfo.state = SIPX_SUBSCRIPTION_PENDING;
         break;
      case SipSubscribeClient::SUBSCRIPTION_SETUP:     // Established dialog
         pInfo.state = SIPX_SUBSCRIPTION_ACTIVE;
         break;
      case SipSubscribeClient::SUBSCRIPTION_FAILED:    // Failed dialog setup or refresh
         // Could potentially differentiate between failed active
         // and failed expired based upon the expiration and the 
         // current time
         pInfo.state = SIPX_SUBSCRIPTION_FAILED;
         break;
      case SipSubscribeClient::SUBSCRIPTION_TERMINATED:
         pInfo.state = SIPX_SUBSCRIPTION_EXPIRED;
         break;
      case SipSubscribeClient::SUBSCRIPTION_UNKNOWN:
         errorState = "SUBSCRIPTION_UNKNOWN";
         pInfo.state = SIPX_SUBSCRIPTION_FAILED;
         break;
      default:
         {
            pInfo.state = SIPX_SUBSCRIPTION_FAILED;
            errorState ="unknown: ";
            char numBuf[20];
            SNPRINTF(numBuf, sizeof(numBuf), "%d", newState);
            errorState.append(numBuf);
         }
         break;
      }

      // If the dialog changed from and early dialog to an 
      // established dialog, update the dialog handle in the
      // subcription data structure
      if(earlyDialogHandle && dialogHandle && 
         SipDialog::isInitialDialog(pData->dialogHandle))
      {
         pData->dialogHandle = dialogHandle;
      }

      sipxSubscribeReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);

      // Fire the event if it is a supported state change
      if(errorState.isNull())
      {
         sipxFireSubscriptionStatusEvent(pInst, &pInfo);
      }
      else
      {
         OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
            "sipxSubscribeClientSubCallback: invalid SubscriptionState: %s",
            errorState.data());
      }
   }
   else
   {
      // Cannot find subsription data for this handle
      OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
         "sipxSubscribeClientSubCallback: cannot find subscription data for handle: %p",
         applicationData);
   }
}

/**
 * @brief Finds subscription and locks it.
 */

SIPX_SUBSCRIPTION_DATA* sipxSubscribeLookup(const SIPX_SUB hSub,
                                            SIPX_LOCK_TYPE type,
                                            const OsStackTraceLogger& oneBackInStack)
{
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxSubscribeLookup", oneBackInStack);
   SIPX_SUBSCRIPTION_DATA* pRC = NULL;    
   OsStatus status = OS_FAILED;

   gSubHandleMap.lock(); // global lock will enable us to delete mutex safely
   pRC = (SIPX_SUBSCRIPTION_DATA*) gSubHandleMap.findHandle(hSub);

   if(pRC)
   {
      switch (type)
      {
      case SIPX_LOCK_READ:
         status = pRC->mutex.acquire();
         assert(status == OS_SUCCESS);
         break;
      case SIPX_LOCK_WRITE:
         status = pRC->mutex.acquire();
         assert(status == OS_SUCCESS);
         break;
      default:
         break;
      }
   }

   gSubHandleMap.unlock();

   return pRC;
}

/**
 * @brief Releases subscription lock.
 */

void sipxSubscribeReleaseLock(SIPX_SUBSCRIPTION_DATA* pData,
                              SIPX_LOCK_TYPE type,
                              const OsStackTraceLogger& oneBackInStack) 
{
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxSubscribeReleaseLock", oneBackInStack);
   OsStatus status;

   if (pData)
   {
      switch (type)
      {
      case SIPX_LOCK_READ:
         status = pData->mutex.release();
         assert(status == OS_SUCCESS);
         break;
      case SIPX_LOCK_WRITE:
         status = pData->mutex.release();
         assert(status == OS_SUCCESS);
         break;
      default:
         break;
      }
   }
}

/**
 * @brief Remove/Destroy all subscriptions
 */ 

void sipxSubscribeDestroyAll(const SIPX_INST hInst) 
{
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxSubscribeDestroyAll");
   gSubHandleMap.lock();

   UtlHashMapIterator iter(gSubHandleMap);
   UtlInt* pKey;
   UtlVoidPtr* pValue;
   SIPX_SUB hSub;

   while ((pKey = dynamic_cast<UtlInt*>(iter())))
   {
      pValue = dynamic_cast<UtlVoidPtr*>(gSubHandleMap.findValue(pKey));
      assert(pValue);

      hSub = (SIPX_SUB)pValue->getValue();

      if (hSub)
      {
         bool bRemove = false;

         SIPX_SUBSCRIPTION_DATA* pData = sipxSubscribeLookup(hSub, SIPX_LOCK_READ, logItem);
         if (pData)
         {
            if (pData->pInst == hInst)
            {
               bRemove = true;
            }
            sipxSubscribeReleaseLock(pData, SIPX_LOCK_READ, logItem);
         }

         if (bRemove)
         {
            sipxCallUnsubscribe(hSub);
         }
      }
   }

   gSubHandleMap.unlock();
}


void sipxSubscribeClientNotifyCallback(const char* earlyDialogHandle,
                                       const char* dialogHandle,
                                       void* applicationData,
                                       const SipMessage* notifyRequest)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxSubscribeClientNotifyCallback");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxSubscribeClientNotifyCallback earlyDialogHandle=%s dialogHandle=%s",
      earlyDialogHandle ? earlyDialogHandle : "<NULL>",
      dialogHandle ? dialogHandle : "<NULL>");

   SIPX_SUB hSub = (SIPX_SUB)applicationData;

   if (hSub)
   {
      SIPX_SUBSCRIPTION_DATA* pData = sipxSubscribeLookup(hSub, SIPX_LOCK_WRITE, stackLogger);

      if (pData)
      {
         assert(pData->pInst);
         SIPX_INSTANCE_DATA* pInst = pData->pInst;

         UtlString userAgent;
         UtlString contentType;
         const HttpBody* contentBody = NULL;
         int bodyLength = 0;
         const char* bodyBytes = NULL;

         // If the dialog changed from and early dialog to an 
         // established dialog, update the dialog handle in the
         // subcription data structure
         if(earlyDialogHandle && dialogHandle &&
            SipDialog::isInitialDialog(pData->dialogHandle))
         {
            pData->dialogHandle = dialogHandle;
         }

         sipxSubscribeReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);

         if(notifyRequest)
         {
            notifyRequest->getUserAgentField(&userAgent);
            notifyRequest->getContentType(&contentType);
            contentBody = notifyRequest->getBody();

            if(contentBody)
            {
               contentBody->getBytes(&bodyBytes, &bodyLength);
            }
         }

         if (bodyLength > 0)
         {
            SIPX_NOTIFY_INFO pInfo;
            pInfo.nSize = sizeof(SIPX_NOTIFY_INFO);
            pInfo.hSub = (SIPX_SUB)applicationData;
            pInfo.szNotiferUserAgent = userAgent;
            pInfo.nContentLength = bodyLength;
            pInfo.pContent = bodyBytes;
            pInfo.szContentType = contentType;

            // we make a deep copy of pInfo inside
            sipxFireNotifyEvent(pInst, &pInfo);
         }
      }
      else
      {
         // No data for the subscription handle
         OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
            "sipxSubscribeClientNotifyCallback: cannot find subscription data for handle: %p",
            applicationData);
      }
   }   
}

/***************************************************************************
* Public Subscription Functions
***************************************************************************/

/**
* @brief Cancels subscription and deletes the object.
*/

SIPXTAPI_API SIPX_RESULT sipxCallUnsubscribe(const SIPX_SUB hSub)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallUnsubscribe");
   SIPX_RESULT sipXresult = SIPX_RESULT_FAILURE;
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallSubscribe hSub=%x", hSub);

   SIPX_SUBSCRIPTION_DATA* pData = sipxSubscribeLookup(hSub, SIPX_LOCK_READ, stackLogger);

   if(pData)
   {
      assert(pData->pInst); // this should never happen
      UtlString dialogHandle(pData->dialogHandle);
      SIPX_INSTANCE_DATA *pInst = pData->pInst;

      // release lock to prevent deadlock with pSubscribeClient
      sipxSubscribeReleaseLock(pData, SIPX_LOCK_READ, stackLogger);

      if(pInst->pSubscribeClient->endSubscription(dialogHandle))
      {
         gSubHandleMap.lock(); // we need global lock to delete mutex
         SIPX_SUBSCRIPTION_DATA* pTmpData = sipxSubscribeLookup(hSub, SIPX_LOCK_WRITE, stackLogger);

         if (pTmpData)
         {
            // Remove and free up the subscription handle and data
            gSubHandleMap.removeHandle(hSub);

            delete pTmpData;
            sipXresult = SIPX_RESULT_SUCCESS;
         }
         else
         {
            sipXresult = SIPX_RESULT_INVALID_STATE;
         }

         gSubHandleMap.unlock(); // we can release global lock now
      }
      else
      {
         OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
            "sipxCallUnsubscribe endSubscription failed for subscription handle: %d dialog handle: \"%s\"",
            hSub, dialogHandle);
         sipXresult = SIPX_RESULT_INVALID_ARGS;
      }
   }
   else  // Invalid subscription handle, possibly already deleted
   {
      OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
         "sipxCallUnsubscribe: cannot find subscription data for handle: %d", hSub);
      sipXresult = SIPX_RESULT_INVALID_ARGS;
      // no other lock to release, since handle wasn't found
   }

   return(sipXresult);
}


SIPXTAPI_API SIPX_RESULT sipxCallSubscribe(const SIPX_CALL hCall,
                                           const char* szEventType,
                                           const char* szAcceptType,
                                           SIPX_SUB* phSub,
                                           int bRemoteContactIsGruu,
                                           int subscriptionPeriod,
                                           SIPX_TRANSPORT_TYPE transport)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallSubscribe");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallSubscribe hCall=%d szEventType=\"%s\" szAcceptType=\"%s\"",
      hCall,
      szEventType ? szEventType : "<null>",
      szAcceptType ? szAcceptType : "<null>");

   SIPX_RESULT sipXresult = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA *pInst;
   UtlString sessionCallId; // test
   UtlString remoteAddress; // test
   SipDialog callSipDialog;
   Url fullLineUrl;

   if (hCall != SIPX_CALL_NULL && phSub && szEventType)
   {
      SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);
      if (pData)
      {
         callSipDialog = pData->m_sipDialog;
         pInst = pData->m_pInst;
         fullLineUrl = pData->m_fullLineUrl;
         sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
         pData = NULL;

         // dialog must be established - early/confirmed, or it will not contain correct data
         if (callSipDialog.isEstablishedDialog())
         {
            // create new subscription
            SIPX_SUBSCRIPTION_DATA* subscriptionData = new SIPX_SUBSCRIPTION_DATA();
            subscriptionData->mutex.acquire();
            UtlBoolean res = gSubHandleMap.allocHandle(*phSub, subscriptionData);

            if (res)
            {
               subscriptionData->pInst = pInst;
               // Need to get the resourceId, To, From and Contact from
               // the associated call
               Url fromUrl = fullLineUrl;
               Url toUrl;
               callSipDialog.getRemoteField(toUrl);
               toUrl.removeFieldParameters(); // get rid of to tag
               Url localContactUrl;
               callSipDialog.getLocalContact(localContactUrl);

               // If remoteContactIsGruu is set we assume the
               // remote contact is a globally routable unique URI (GRUU).
               // Normally one cannot assume the Contact header is a
               // publicly addressable address and we assume that the
               // To or From from the remote side has a better chance of
               // being globally routable as it typically is an address
               // of record (AOR).
               Url requestUri;
               if(bRemoteContactIsGruu)
               {
                  Url remoteContactUrl;
                  callSipDialog.getRemoteContact(remoteContactUrl);
                  requestUri = remoteContactUrl.getUri();
               }
               else
               {
                  requestUri = toUrl.getUri();
               }

               // Subscribe and keep the subscription refreshed
               if(pInst->pSubscribeClient->addSubscription(requestUri.toString(),// resourceId 
                           szEventType,
                           szAcceptType,
                           fromUrl.toString(), // fromFieldValue
                           toUrl.toString(), // toFieldValue
                           localContactUrl.toString(),                                                         
                           subscriptionPeriod, 
                           (void*)*phSub, 
                           sipxSubscribeClientSubCallback,
                           sipxSubscribeClientNotifyCallback, 
                           subscriptionData->dialogHandle,
                           (SIP_TRANSPORT_TYPE)transport))
               {
                  sipXresult = SIPX_RESULT_SUCCESS;
               }

               subscriptionData->mutex.release();
            } 
            else
            {
               // handle allocation failure
               OsSysLog::add(FAC_SIPXTAPI, PRI_ERR, 
                  "allocHandle failed to allocate a handle");
               delete subscriptionData;
               subscriptionData = NULL;
            }        
         }
      }
      else
      {
         OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
            "sipxCallSubscribe: could not find call data for call handle: %d",
            hCall);
         sipXresult = SIPX_RESULT_INVALID_ARGS;
      }
   }
   else
   {
      sipXresult = SIPX_RESULT_INVALID_ARGS;
   }

   return sipXresult;
}


SIPXTAPI_API SIPX_RESULT sipxConfigSubscribe(const SIPX_INST hInst,
                                             const SIPX_LINE hLine,
                                             const char* szTargetUrl,
                                             const char* szEventType,
                                             const char* szAcceptType,
                                             const SIPX_CONTACT_ID contactId,
                                             SIPX_SUB* phSub,
                                             int subscriptionPeriod,
                                             SIPX_TRANSPORT_TYPE transport)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfigSubscribe");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxConfigSubscribe hInst=%p szTargetUrl=\"%s\" szEventType=\"%s\" szAcceptType=\"%s\"",
      hInst,
      szTargetUrl ?  szTargetUrl  : "<null>",
      szEventType ? szEventType : "<null>",
      szAcceptType ? szAcceptType : "<null>");

   SIPX_RESULT sipXresult = SIPX_RESULT_FAILURE;

   if (hInst && phSub && szTargetUrl && szEventType && szAcceptType)
   {
      UtlString sTargetUrl(szTargetUrl);
      SIPX_INSTANCE_DATA *pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

      // try to find contact by ID
      SipContact* pContact = pInst->pSipUserAgent->getContactDb().find(contactId);
      if (!pContact)
      {
         // contact was not found by contactId, use the first local contact, will be overridden later anyway
         // when sending via SipUserAgent
         pContact = pInst->pSipUserAgent->getContactDb().find(SIP_CONTACT_LOCAL,
            (SIP_TRANSPORT_TYPE)transport);
         if (!pContact)
         {
            return SIPX_RESULT_INVALID_ARGS;
         }
      }
      if (transport != TRANSPORT_AUTO &&
         (SIPX_TRANSPORT_TYPE)pContact->getTransportType() != transport)
      {
         delete pContact;
         pContact = NULL;
         return SIPX_RESULT_INVALID_ARGS;
      }
      SIPX_TRANSPORT_TYPE subscribeTransport = (SIPX_TRANSPORT_TYPE)pContact->getTransportType();

      SIPX_LINE_DATA* pLineData = sipxLineLookup(hLine, SIPX_LOCK_READ, stackLogger);

      if(pLineData)
      {
         // Need to get the resourceId, To, From and Contact from
         // the associated call
         UtlString resourceId = Url(sTargetUrl).getUri().toString();
         UtlString fromField;
         UtlString toField(sTargetUrl);
         UtlString contactField;
         // build a contact field 
         UtlString userId(""); 

         fromField = pLineData->m_fullLineUrl.toString();
         pLineData->m_fullLineUrl.getUserId(userId);

         sipxLineReleaseLock(pLineData, SIPX_LOCK_READ, stackLogger);

         SIPX_SUBSCRIPTION_DATA* subscriptionData = new SIPX_SUBSCRIPTION_DATA();
         subscriptionData->mutex.acquire();
         UtlBoolean res = gSubHandleMap.allocHandle(*phSub, subscriptionData);

         if (res)
         {
            subscriptionData->pInst = pInst;

            Url contactUri;
            pContact->buildContactUri(contactUri);
            contactUri.toString(contactField);
            UtlBoolean bAllowContactOverride = (contactId == AUTOMATIC_CONTACT_ID);

            if(resourceId.length() <= 1 || 
               fromField.length() <= 4 || 
               toField.length() <= 4 || 
               contactField.length() <= 4) 
            {
               OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
                  "sipxConfigSubscribe bad parameters");
            }
            else 
            {
               // Session data is big enough 
               // Subscribe and keep the subscription refreshed 
               if(pInst->pSubscribeClient->addSubscription(resourceId,
                        szEventType,
                        szAcceptType,
                        fromField,
                        toField,
                        contactField,
                        subscriptionPeriod,
                        (void*)*phSub,
                        sipxSubscribeClientSubCallback,
                        sipxSubscribeClientNotifyCallback,
                        subscriptionData->dialogHandle,
                        (SIP_TRANSPORT_TYPE)subscribeTransport,
                        bAllowContactOverride))
               {
                  sipXresult = SIPX_RESULT_SUCCESS;
               }

            }

            subscriptionData->mutex.release();
         }
         else
         {
            // handle allocation failure
            OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
               "allocHandle failed to allocate a handle");
            delete subscriptionData;
         }
      }// if(pLineData)

      delete pContact;
      pContact = NULL;
   } // if (hInst)
   else
   {
      sipXresult = SIPX_RESULT_INVALID_ARGS;
   }

   return sipXresult;
}


SIPXTAPI_API SIPX_RESULT sipxConfigUnsubscribe(const SIPX_SUB hSub) 
{ 
   return sipxCallUnsubscribe(hSub); 
}

