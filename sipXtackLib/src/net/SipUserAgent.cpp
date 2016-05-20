//  
// Copyright (C) 2007 SIPez LLC. 
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

// Author: Daniel Petrie <dpetrie AT SIPez DOT com>

// SYSTEM INCLUDES

//#define TEST_PRINT

#include <assert.h>

// APPLICATION INCLUDES
#if defined(_WIN32)
#       include "resparse/wnt/nterrno.h"
#elif defined(__pingtel_on_posix__)
#	include <sys/types.h>
#       include <sys/socket.h>
#       include <stdlib.h>
#endif

#include <utl/UtlHashBagIterator.h>
#include <net/SipSrvLookup.h>
#include <net/SipUserAgent.h>
#include <net/SipMessageEvent.h>
#include <net/NameValueTokenizer.h>
#include <net/SipObserverCriteria.h>
#include <net/NetMd5Codec.h>
#include <os/HostAdapterAddress.h>
#include <net/Url.h>
#include <net/SipDialog.h>
#ifdef HAVE_SSL
#include <net/SipTlsServer.h>
#endif
#include <net/SipTcpServer.h>
#include <net/SipUdpServer.h>
#include <net/SipLineProvider.h>
#include <net/SipLineCredential.h>
#include <net/SipContact.h>
#include <net/SipContactSelector.h>
#include <os/OsDateTime.h>
#include <os/OsEvent.h>
#include <os/OsQueuedEvent.h>
#include <os/OsTimer.h>
#include <os/OsTimerTask.h>
#include <os/OsEventMsg.h>
#include <os/OsPtrMsg.h>
#include <os/OsRpcMsg.h>
#include <os/OsConfigDb.h>
#include <os/OsRWMutex.h>
#include <os/OsReadLock.h>
#include <os/OsWriteLock.h>
#include <os/OsProcess.h>
#ifndef _WIN32
// version.h is generated as part of the build by other platforms.  For
// windows, the sip stack version is defined under the project settings.
#include <net/version.h>
#endif
#include <os/OsSysLog.h>
#include <os/OsFS.h>
#include <utl/UtlTokenizer.h>
#include <os/OsNatAgentTask.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

//#define PRINT_SIP_MESSAGE
#define MAXIMUM_SIP_LOG_SIZE 100000
#define SIP_UA_LOG "sipuseragent.log"
#define CONFIG_LOG_DIR SIPX_LOGDIR

#ifndef  VENDOR
# define VENDOR "sipX"
#endif

#ifndef PLATFORM_UA_PARAM
#if defined(_WIN32)
#  define PLATFORM_UA_PARAM " (WinNT)"
#elif defined(_VXWORKS)
#  define PLATFORM_UA_PARAM " (VxWorks)"
#elif defined(__MACH__)
#  define PLATFORM_UA_PARAM " (Darwin)"
#elif defined(__linux__)
#  define PLATFORM_UA_PARAM " (Linux)"
#elif defined(sun)
#  define PLATFORM_UA_PARAM " (Solaris)"
#endif
#endif /* PLATFORM_UA_PARAM */

//#define TEST_PRINT 1
//#define LOG_TIME

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipUserAgent::SipUserAgent(int sipTcpPort,
                           int sipUdpPort,
                           int sipTlsPort,
                           const char* bindIpAddress,// may be NULL
                           const UtlString& defaultUser,
                           const char* sipProxyServers,
                           const char* sipDirectoryServers,
                           const char* authenticationScheme,
                           const char* authenticateRealm,
                           OsConfigDb* authenticateDb,
                           OsConfigDb* authorizeUserIds,
                           OsConfigDb* authorizePasswords,
                           SipLineProvider* lineProvider,
                           int sipFirstResendTimeout,
                           UtlBoolean defaultToUaTransactions,
                           int readBufferSize,
                           int queueSize,
                           UtlBoolean bUseNextAvailablePort,
                           UtlBoolean doUaMessageChecks
                           ) 
: SipUserAgentBase(sipTcpPort, sipUdpPort, sipTlsPort, queueSize)
, mSipTcpServer(NULL)
, mSipUdpServer(NULL)
#ifdef HAVE_SSL
, mSipTlsServer(NULL)
#endif
, mDefaultUser(defaultUser)
, mMessageLogRMutex(OsRWMutex::Q_FIFO)
, mMessageLogWMutex(OsRWMutex::Q_FIFO)
, m_pLineProvider(NULL)
, mIsUaTransactionByDefault(defaultToUaTransactions)
, mbUseRport(FALSE)
, mbUseLocationHeader(FALSE)
, mbIncludePlatformInUserAgentName(TRUE)
, mDoUaMessageChecks(doUaMessageChecks)
, mbShuttingDown(FALSE)
, mRegisterTimeoutSeconds(4)        
, mbAllowHeader(true)
, mbSupportedHeader(true)
, mbDateHeader(true)
, mbShortNames(false)
, mAcceptLanguage("")
, mDefaultPort(PORT_NONE)
{    
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
      "SipUserAgent::_ sipTcpPort = %d, sipUdpPort = %d, sipTlsPort = %d",
      sipTcpPort, sipUdpPort, sipTlsPort);

   assert(mUdpPort != PORT_NONE);

   // Get pointer to line manager
   m_pLineProvider = lineProvider;

   // Create and start the SIP TLS, TCP and UDP Servers
   mSipUdpServer = new SipUdpServer(mUdpPort, this,
      readBufferSize, bUseNextAvailablePort, bindIpAddress );
   mSipUdpServer->startListener();
   mUdpPort = mSipUdpServer->getServerPort() ;
   mDefaultPort = mDefaultPort;

   if (mTcpPort != PORT_NONE)
   {
      mSipTcpServer = new SipTcpServer(mTcpPort, this, SIP_TRANSPORT_TCP_STR, 
         "SipTcpServer-%d", bUseNextAvailablePort, bindIpAddress);
      mSipTcpServer->startListener();
      mTcpPort = mSipTcpServer->getServerPort();
   }

#ifdef HAVE_SSL
   if (mTlsPort != PORT_NONE)
   {
      mSipTlsServer = new SipTlsServer(mTlsPort,
         this,
         bUseNextAvailablePort,
         bindIpAddress);
      mSipTlsServer->startListener();
      mTlsPort = mSipTlsServer->getServerPort();
   }
#endif

   mMaxMessageLogSize = MAXIMUM_SIP_LOG_SIZE;
   mMaxForwards = SIP_DEFAULT_MAX_FORWARDS;

   // TCP sockets not used for an hour are garbage collected
   mMaxTcpSocketIdleTime = 3600;

   // INVITE transactions need to stick around for a minimum of
   // 3 minutes
   mMinInviteTransactionTimeout = 180;

   mForkingEnabled = TRUE;
   mRecurseOnlyOne300Contact = FALSE;

   // By default copy all of the Vias from incoming requests that have
   // a max-forwards == 0
   mReturnViasForMaxForwards = TRUE;

   mMaxSrvRecords = 4;
   mDnsSrvTimeout = 4; // seconds

#ifdef TEST_PRINT
   // Default the log on
   startMessageLog();
#else
   // Disable the message log
   stopMessageLog();
#endif

   // Authentication
   if(authenticationScheme)
   {
      mAuthenticationScheme.append(authenticationScheme);
      HttpMessage::cannonizeToken(mAuthenticationScheme);
      // Do not require authentication if not BASIC or DIGEST
      if(   0 != mAuthenticationScheme.compareTo(HTTP_BASIC_AUTHENTICATION,
         UtlString::ignoreCase
         )
         && 0 !=mAuthenticationScheme.compareTo(HTTP_DIGEST_AUTHENTICATION,
         UtlString::ignoreCase
         )
         )
      {
         mAuthenticationScheme.remove(0);
      }

   }
   if(authenticateRealm)
   {
      mAuthenticationRealm.append(authenticateRealm);
   }

   if(authenticateDb)
   {
      mpAuthenticationDb = authenticateDb;
   }
   else
   {
      mpAuthenticationDb = new OsConfigDb();
   }

   if(authorizeUserIds)
   {
      mpAuthorizationUserIds = authorizeUserIds;
   }
   else
   {
      mpAuthorizationUserIds = new OsConfigDb();
   }

   if(authorizePasswords)
   {
      mpAuthorizationPasswords = authorizePasswords;
   }
   else
   {
      mpAuthorizationPasswords = new OsConfigDb();
   }

   // SIP Server info
   if(sipProxyServers)
   {
      m_defaultProxyServers.append(sipProxyServers);
   }
   if(sipDirectoryServers)
   {
      directoryServers.append(sipDirectoryServers);
   }

   if (!bindIpAddress || strcmp(bindIpAddress, "0.0.0.0") == 0)
   {
      // get the first CONTACT entry in the Db
      SipContact* pContact = mContactDb.find(SIP_CONTACT_LOCAL, SIP_TRANSPORT_UDP); 
      assert(pContact);
      // Bind to the contact's Ip
      if (pContact)
      {
         mDefaultIpAddress = pContact->getIpAddress();
         delete pContact;
         pContact = NULL;
      }
   }
   else
   {
      mDefaultIpAddress = bindIpAddress;
   }

   //Timers
   if ( sipFirstResendTimeout <= 0)
   {
      mFirstResendTimeoutMs = SIP_DEFAULT_RTT;
   }
   else if ( sipFirstResendTimeout > 0  && sipFirstResendTimeout < 100)
   {
      mFirstResendTimeoutMs = SIP_MINIMUM_RTT;
   }
   else
   {
      mFirstResendTimeoutMs = sipFirstResendTimeout;
   }
   mLastResendTimeoutMs = 8 * mFirstResendTimeoutMs;
   mReliableTransportTimeoutMs = 2 * mLastResendTimeoutMs;
   mTransactionStateTimeoutMs = 10 * mLastResendTimeoutMs;

   // How long before we expire transactions by default
   mDefaultExpiresSeconds = 180; // mTransactionStateTimeoutMs / 1000;
   mDefaultSerialExpiresSeconds = 20;

   SipMessage::buildSipUrl(&mDefaultContactAddress,
      mDefaultIpAddress.data(),
      mDefaultPort,
      NULL, mDefaultUser.data());

   // generate seed for branch ids
   char branchIdSeed[500];
   UtlRandom randomNumGenerator;
   int currentPid = OsProcess::getCurrentPID();
   SNPRINTF(branchIdSeed, sizeof(branchIdSeed), "%d%d", randomNumGenerator.rand(), currentPid);
   // Initialize the transaction id seed
   SipTransaction::smBranchIdBase = branchIdSeed;

   // Allow the default SIP methods
   allowMethod(SIP_INVITE_METHOD);
   allowMethod(SIP_ACK_METHOD);
   allowMethod(SIP_CANCEL_METHOD);
   allowMethod(SIP_BYE_METHOD);
   allowMethod(SIP_OPTIONS_METHOD);

   defaultUserAgentName.append( VENDOR );
   defaultUserAgentName.append( "/" );
   defaultUserAgentName.append( SIP_STACK_VERSION );

   OsMsgQ* incomingQ = getMessageQueue();
   mpTimer = new OsTimer(incomingQ, 0);
   // Convert from mSeconds to uSeconds
   OsTime lapseTime(0, mTransactionStateTimeoutMs * 1000);
   mpTimer->periodicEvery(lapseTime, lapseTime);

   OsTime time;
   OsDateTime::getCurTimeSinceBoot(time);
   mLastCleanUpTime = time.seconds();

   // bandreasen: This was removed on main -- not sure why
   //     given that this boolean is passed in
   mIsUaTransactionByDefault = defaultToUaTransactions;
}

// Destructor
SipUserAgent::~SipUserAgent()
{
   mpTimer->stop();
   delete mpTimer;
   mpTimer = NULL;

   // Wait until this OsServerTask has stopped or handleMethod
   // might access something we are about to delete here.
   waitUntilShutDown();

   if(mSipTcpServer)
   {
      mSipTcpServer->shutdownListener();
      mSipTcpServer->requestShutdown();
      delete mSipTcpServer;
      mSipTcpServer = NULL;
   }

   if(mSipUdpServer)
   {
      mSipUdpServer->shutdownListener();
      mSipUdpServer->requestShutdown();
      delete mSipUdpServer;
      mSipUdpServer = NULL;
   }

#ifdef HAVE_SSL
   if(mSipTlsServer)
   {
      mSipTlsServer->shutdownListener();
      mSipTlsServer->requestShutdown();
      delete mSipTlsServer;
      mSipTlsServer = NULL;
   }
#endif

   if(mpAuthenticationDb)
   {
      delete mpAuthenticationDb;
      mpAuthenticationDb = NULL;
   }

   if(mpAuthorizationUserIds)
   {
      delete mpAuthorizationUserIds;
      mpAuthorizationUserIds = NULL;
   }

   if(mpAuthorizationPasswords)
   {
      delete mpAuthorizationPasswords;
      mpAuthorizationPasswords = NULL;
   }

   allowedSipMethods.destroyAll();
   mMessageObservers.destroyAll();
   allowedSipExtensions.destroyAll();
}

/* ============================ MANIPULATORS ============================== */


// Assignment operator
SipUserAgent&
SipUserAgent::operator=(const SipUserAgent& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

void SipUserAgent::shutdown(UtlBoolean blockingShutdown)
{
   mbShuttingDown = TRUE;
   mSipTransactions.stopTransactionTimers();

   if(blockingShutdown == TRUE)
   {
      OsEvent shutdownEvent;
      OsStatus res;
      int rpcRetVal;

      mbBlockingShutdown = TRUE;

      OsRpcMsg shutdownMsg(OsMsg::PHONE_APP, SipUserAgent::SHUTDOWN_MESSAGE, shutdownEvent);
      postMessage(shutdownMsg);
      res = shutdownEvent.wait();
      assert(res == OS_SUCCESS);

      res = shutdownEvent.getEventData(rpcRetVal);
      assert(res == OS_SUCCESS && rpcRetVal == OS_SUCCESS);

      mbShutdownDone = TRUE;
   }
   else
   {
      mbBlockingShutdown = FALSE;
      OsMsg shutdownMsg(OsMsg::PHONE_APP, SipUserAgent::SHUTDOWN_MESSAGE);
      postMessage(shutdownMsg);
   }
}

void SipUserAgent::enableStun(const char* szStunServer, 
                              int iStunPort,
                              int refreshPeriodInSecs,                               
                              OsMsgQ* pNotificationQueue,
                              const char* szIp) 
{
   if (mSipUdpServer)
   {
      mSipUdpServer->enableStun(szStunServer, 
         iStunPort, szIp, refreshPeriodInSecs, pNotificationQueue);
   }
}

void SipUserAgent::addMessageObserver(OsMsgQ& messageQueue,
                                      const char* sipMethod,
                                      UtlBoolean wantRequests,
                                      UtlBoolean wantResponses,
                                      UtlBoolean wantIncoming,
                                      UtlBoolean wantOutGoing,
                                      const char* eventName,
                                      const SipDialog* pSipDialog,
                                      void* observerData)
{
   SipObserverCriteria* observer = new SipObserverCriteria(observerData,
      &messageQueue,
      sipMethod, wantRequests, wantResponses, wantIncoming,
      wantOutGoing, eventName, pSipDialog);

   {
      // Add the observer and its filter criteria to the list lock scope
      OsWriteLock lock(mObserverMutex);
      mMessageObservers.insert(observer);

      // Allow the specified method
      if(sipMethod && *sipMethod && wantRequests)
         allowMethod(sipMethod);
   }
}


UtlBoolean SipUserAgent::removeMessageObserver(OsMsgQ& messageQueue, void* pObserverData /*=NULL*/)
{
   OsWriteLock lock(mObserverMutex);
   SipObserverCriteria* pObserver = NULL ;
   UtlBoolean bRemovedObservers = FALSE ;

   // Traverse all of the observers and remove any that match the
   // message queue/observer data.  If the pObserverData is null, all
   // matching message queue/observers will be removed.  Otherwise, only
   // those observers that match both the message queue and observer data
   // are removed.
   UtlHashBagIterator iterator(mMessageObservers);
   while ((pObserver = (SipObserverCriteria*) iterator()))
   {
      if (pObserver->getObserverQueue() == &messageQueue)
      {
         if ((pObserverData == NULL) ||
            (pObserverData == pObserver->getObserverData()))
         {
            bRemovedObservers = true ;
            UtlContainable* wasRemoved = mMessageObservers.removeReference(pObserver);

            if(wasRemoved)
            {
               delete wasRemoved;
            }

         }
      }
   }

   return bRemovedObservers ;
}

void SipUserAgent::allowMethod(const char* methodName, const bool bAllow)
{
   if(methodName)
   {
      UtlString matchName(methodName);
      // Do not add the name if it is already in there
      if(NULL == allowedSipMethods.find(&matchName))
      {
         if (bAllow)
         {
            allowedSipMethods.append(new UtlString(methodName));
         }
      }
      else
      {
         if (!bAllow)
         {
            allowedSipMethods.destroy(allowedSipMethods.find(&matchName));
         }
      }
   }
}


UtlBoolean SipUserAgent::send(SipMessage& message,
                              OsMsgQ* responseListener,
                              void* responseListenerData)
{
#ifdef PRINT_SIP_MESSAGE
   enableConsoleOutput(TRUE);
   osPrintf("\nSipUserAgent::send %s\n-----------------------------------\n", message.toString().data());
#endif

   if(mbShuttingDown)
   {
      return FALSE;
   }

   UtlBoolean sendSucceeded = FALSE;
   UtlBoolean isResponse = message.isResponse();

   // ===========================================

   // Do all the stuff that does not require transaction locking first

   // Make sure the date field is set
   long epochDate;
   if(!message.getDateField(&epochDate) && mbDateHeader)
   {
      message.setDateField();
   }

   if (mbUseLocationHeader)
   {
      message.setLocationField(mLocationHeader.data());
   }

   // Make sure the message includes a contact if required and
   // update it to the best possible known contact.
   prepareContact(message, NULL, PORT_NONE);

   // Get Method
   UtlString method;
   if(isResponse)
   {
      int num = 0;
      message.getCSeqField(&num , &method);
   }
   else
   {
      message.getRequestMethod(&method);

      // Make sure that max-forwards is set
      int maxForwards;
      if(!message.getMaxForwards(maxForwards))
      {
         message.setMaxForwards(mMaxForwards);
      }
   }

   // ===========================================

   // Do the stuff that requires the transaction type knowledge
   // i.e. UA verse proxy transaction

   if(!isResponse)
   {
      // This should always be true now:
      if(message.isFirstSend())
      {
         // Save the transaction listener info
         if (responseListener)
         {
            message.setResponseListenerQueue(responseListener);
         }
         if (responseListenerData)
         {
            message.setResponseListenerData(responseListenerData);
         }
      }

      // This is not the first time this message has been sent
      else
      {
         // Should not be getting here.
         OsSysLog::add(FAC_SIP, PRI_WARNING, "SipUserAgent::send message being resent");
      }
   }

   // ===========================================

   // Find or create a transaction:
   UtlBoolean isUaTransaction = TRUE;
   enum SipTransaction::messageRelationship relationship;

   //mSipTransactions.lock();

#if 0 // TODO enable only for transaction match debugging - log is confusing otherwise
   OsSysLog::add(FAC_SIP, PRI_DEBUG
      ,"SipUserAgent::send searching for existing transaction"
      );
#endif
   // verify that the transaction does not already exist
   SipTransaction* transaction = mSipTransactions.findTransactionFor(
      message,
      TRUE, // outgoing
      relationship);

   // Found a transaction for this message
   if(transaction)
   {
      isUaTransaction = transaction->isUaTransaction();

      // Response for which a transaction already exists
      if(isResponse)
      {
         if(isUaTransaction)
         {
            // It seems that the polite thing to do is to add the
            // allowed methods to all final responses
            addAgentCapabilities(message);
         }
      }

      // Request for which a transaction already exists
      else
      {
         // should not get here unless this is a CANCEL or ACK
         // request
         if((method.compareTo(SIP_CANCEL_METHOD) == 0) ||
            (method.compareTo(SIP_ACK_METHOD) == 0))
         {
         }

         // A request for which a transaction already exists
         // other than ACK and CANCEL
         else
         {
            // Should not be getting here
            OsSysLog::add(FAC_SIP, PRI_WARNING,
               "SipUserAgent::send %s request matches existing transaction",
               method.data());

            // We pretend there is no match so this becomes a
            // new transaction branch.  Make sure we unlock the
            // transaction before we reset to NULL.
            mSipTransactions.markAvailable(*transaction);
            transaction = NULL;
         }
      }
   }

   // No existing transaction for this message
   if(transaction == NULL)
   {
      if(isResponse)
      {
         // Should not get here except possibly on a server
         OsSysLog::add(FAC_SIP, PRI_WARNING,
            "SipUserAgent::send response without an existing transaction"
            );
      }
      else
      {
         // If there is already a via in the request this must
         // be a proxy transaction
         UtlString viaField;
         SipTransaction* parentTransaction = NULL;
         enum SipTransaction::messageRelationship parentRelationship;
         if(message.getViaField(&viaField, 0))
         {
            isUaTransaction = FALSE;

            // See if there is a parent server proxy transaction
#if 0 // TODO enable only for transaction match debugging - log is confusing otherwise
            OsSysLog::add(FAC_SIP, PRI_DEBUG
               ,"SipUserAgent::send searching for parent transaction"
               );
#endif
            parentTransaction =
               mSipTransactions.findTransactionFor(message,
               FALSE, // incoming
               parentRelationship);
         }

         // Create a new transactions
         // This should only be for requests
         transaction = new SipTransaction(&message, TRUE,
            isUaTransaction);
         transaction->markBusy();
         mSipTransactions.addTransaction(transaction);

         if(!isUaTransaction &&
            parentTransaction)
         {
            if(parentRelationship ==
               SipTransaction::MESSAGE_DUPLICATE)
            {
               // Link the parent server transaction to the
               // child client transaction
               parentTransaction->linkChild(*transaction);
               // The parent will be unlocked with the transaction
            }
            else
            {
               UtlString parentRelationshipString;
               SipTransaction::getRelationshipString(parentRelationship, parentRelationshipString);
               OsSysLog::add(FAC_SIP, PRI_WARNING,
                  "SipUserAgent::send proxied client transaction not "
                  "part of server transaction, parent relationship: %s",
                  parentRelationshipString.data());

               if(parentTransaction)
               {
                  mSipTransactions.markAvailable(*parentTransaction);
               }
            }
         }
         else if(!isUaTransaction)
         {
            // this happens all the time in the authproxy, so log only at debug
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
               "SipUserAgent::send proxied client transaction does not have parent");
         }
         else if(parentTransaction)
         {
            mSipTransactions.markAvailable(*parentTransaction);
         }

         relationship = SipTransaction::MESSAGE_UNKNOWN;
      }
   }

   if(transaction)
   {
      // Make sure the User Agent field is set
      if(isUaTransaction)
      {
         setSelfHeader(message);

         // Make sure the accept language is set
         UtlString language;
         message.getAcceptLanguageField(&language);
         if(language.isNull())
         {
            // Beware that this value does not describe the desired media
            // sessions, but rather the preferred languages for reason
            // phrases, etc. (RFC 3261 sec. 20.3)  Thus, it is useful to
            // have a value for this header even in requests like
            // SUBSCRIBE/NOTIFY which are expected to not be seen by a human.
            // This value should be configurable, though.
            message.setAcceptLanguageField(mAcceptLanguage);
         }

         // maybe add Allow: and Supported: fields
         addAgentCapabilities(message);
      }

      // If this is the top most parent and it is a client transaction
      //  There is no server transaction, so cancel all of the children
      if(   !isResponse         && (method.compareTo(SIP_CANCEL_METHOD) == 0)
         && transaction->getTopMostParent() == NULL
         && !transaction->isServerTransaction()
         )
      {
         transaction->cancel(*this, mSipTransactions);
      }
      else
      {
         //  All other messages just get sent.

         sendSucceeded = transaction->handleOutgoing(message,
            *this,
            mSipTransactions,
            relationship);
      }

      mSipTransactions.markAvailable(*transaction);
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_ERR,"SipUserAgent::send failed to construct new transaction");
   }

   return(sendSucceeded);
}

UtlBoolean SipUserAgent::sendUdp(SipMessage* message,
                                 const char* serverAddress,
                                 int port)
{
   UtlBoolean isResponse = message->isResponse();
   UtlString method;
   int seqNum;
   UtlString seqMethod;
   int responseCode = 0;
   UtlBoolean sentOk = FALSE;
   UtlString msgBytes;
   UtlString messageStatusString = "SipUserAgent::sendUdp ";
   int timesSent = message->getTimesSent();

   prepareContact(*message, serverAddress, port);

   if(!isResponse)
   {
      message->getRequestMethod(&method);
   }
   else
   {
      message->getCSeqField(&seqNum, &seqMethod);
      responseCode = message->getResponseStatusCode();
   }

   if(timesSent == 0)
   {
#ifdef TEST_PRINT
      osPrintf("First UDP send of message\n");
#endif

      message->touchTransportTime();

#ifdef TEST_PRINT
      osPrintf("SipUserAgent::sendUdp Sending UDP message\n");
#endif
   }
   // get the message if it was previously sent.
   else
   {
      char buffer[20];
      SNPRINTF(buffer, sizeof(buffer), "%d", timesSent);
      messageStatusString.append("resend ");
      messageStatusString.append(buffer);
      messageStatusString.append(" of UDP message\n");
   }

   // Send the message
   if (mbShortNames || message->getUseShortFieldNames())
   {
      message->replaceLongFieldNames();
   }

   // Disallow an address begining with * as it gets broadcasted on NT
   if(! strchr(serverAddress, '*') && *serverAddress)
   {
      sentOk = mSipUdpServer->send(message, serverAddress, port);
   }
   else if(*serverAddress == '\0')
   {
      // Only bother processing if the logs are enabled
      if (    isMessageLoggingEnabled() ||
         OsSysLog::willLog(FAC_SIP_OUTGOING, PRI_INFO))
      {
         UtlString msgBytes;
         int msgLen;
         message->getBytes(&msgBytes, &msgLen);
         msgBytes.insert(0, "No send address\n");
         msgBytes.append("--------------------END--------------------\n");
         logMessage(msgBytes.data(), msgBytes.length());
         OsSysLog::add(FAC_SIP_OUTGOING, PRI_INFO, "%s", msgBytes.data());
      }
      sentOk = FALSE;
   }
   else
   {
      sentOk = FALSE;
   }

#ifdef TEST_PRINT
   osPrintf("SipUserAgent::sendUdp sipUdpServer send returned: %d\n",
      sentOk);
   osPrintf("SipUserAgent::sendUdp isResponse: %d method: %s seqmethod: %s responseCode: %d\n",
      isResponse, method.data(), seqMethod.data(), responseCode);
#endif
   // If we have not failed schedule a resend
   if(sentOk)
   {
      messageStatusString.append("UDP SIP User Agent sent message:\n");
      messageStatusString.append("----Remote Host:");
      messageStatusString.append(serverAddress);
      messageStatusString.append("---- Port: ");
      char buff[10];
      SNPRINTF(buff, sizeof(buff), "%d", !portIsValid(port) ? 5060 : port);
      messageStatusString.append(buff);
      messageStatusString.append("----\n");

#ifdef TEST_PRINT
      osPrintf("%s", messageStatusString.data());
#endif
   }
   else
   {
      messageStatusString.append("UDP SIP User Agent failed to send message:\n");
      messageStatusString.append("----Remote Host:");
      messageStatusString.append(serverAddress);
      messageStatusString.append("---- Port: ");
      char buff[10];
      SNPRINTF(buff, sizeof(buff), "%d", !portIsValid(port) ? 5060 : port);
      messageStatusString.append(buff);
      messageStatusString.append("----\n");
      message->logTimeEvent("FAILED");
   }

   // Only bother processing if the logs are enabled
   if (    isMessageLoggingEnabled() ||
      OsSysLog::willLog(FAC_SIP_OUTGOING, PRI_INFO))
   {
      int len;
      message->getBytes(&msgBytes, &len);
      msgBytes.insert(0, messageStatusString.data());
      msgBytes.append("--------------------END--------------------\n");
#ifdef TEST_PRINT
      osPrintf("%s", msgBytes.data());
#endif
      logMessage(msgBytes.data(), msgBytes.length());
      if (msgBytes.length())
      {
         OsSysLog::add(FAC_SIP_OUTGOING, PRI_INFO, "%s", msgBytes.data());
      }
   }

   // if we failed to send it is the calling functions problem to deal with the error

   return(sentOk);
}

UtlBoolean SipUserAgent::sendSymmetricUdp(SipMessage& message,
                                          const char* serverAddress,
                                          int port)
{
   prepareContact(message, serverAddress, port);

   // Update Via
   UtlString bestKnownAddress;
   int bestKnownPort;

   SipContactSelector contactSelector(*this); // can also select via ip
   contactSelector.getBestContactAddress(bestKnownAddress, bestKnownPort,
      SIP_TRANSPORT_UDP, message.getLocalIp(), serverAddress, port);

   message.removeLastVia() ;
   message.addVia(bestKnownAddress, bestKnownPort, SIP_TRANSPORT_UDP_STR);
   message.setLastViaTag("", "rport");

   // Send away
   UtlBoolean sentOk = mSipUdpServer->sendTo(message,
      serverAddress,
      port);

   // Don't bother processing unless the logs are enabled
   if (    isMessageLoggingEnabled() ||
      OsSysLog::willLog(FAC_SIP_OUTGOING, PRI_INFO))
   {
      UtlString msgBytes;
      int msgLen;
      message.getBytes(&msgBytes, &msgLen);
      UtlString outcomeMsg;
      char portString[20];
      SNPRINTF(portString, sizeof(portString), "%d", !portIsValid(port) ? 5060 : port);

      if(sentOk)
      {
         outcomeMsg.append("UDP SIP User Agent sentTo message:\n----Remote Host:");
         outcomeMsg.append(serverAddress);
         outcomeMsg.append("---- Port: ");
         outcomeMsg.append(portString);
         outcomeMsg.append("----\n");
         msgBytes.insert(0, outcomeMsg);
         msgBytes.append("--------------------END--------------------\n");
      }
      else
      {
         outcomeMsg.append("SIP User agent FAILED sendTo message:\n----Remote Host:");
         outcomeMsg.append(serverAddress);
         outcomeMsg.append("---- Port: ");
         outcomeMsg.append(portString);
         outcomeMsg.append("----\n");
         msgBytes.insert(0, outcomeMsg);
         msgBytes.append("--------------------END--------------------\n");
      }

      logMessage(msgBytes.data(), msgBytes.length());
      OsSysLog::add(FAC_SIP_OUTGOING, PRI_INFO, "%s", msgBytes.data());
   }

   return(sentOk);
}

UtlBoolean SipUserAgent::sendStatelessResponse(SipMessage& rresponse)
{
   UtlBoolean sendSucceeded = FALSE;

   // Forward via the server transaction
   SipMessage responseCopy(rresponse);
   responseCopy.removeLastVia();
   responseCopy.resetTransport();
   responseCopy.clearDNSField();

   UtlString sendProtocol;
   UtlString sendAddress;
   int sendPort;
   int receivedPort;
   UtlBoolean receivedSet;
   UtlBoolean maddrSet;
   UtlBoolean receivedPortSet;

   // use the via as the place to send the response
   responseCopy.getLastVia(&sendAddress, &sendPort, &sendProtocol,
      &receivedPort, &receivedSet, &maddrSet,
      &receivedPortSet);

   // If the sender of the request indicated support of
   // rport (i.e. received port) send this response back to
   // the same port it came from
   if(portIsValid(receivedPort) && receivedPortSet)
   {
      sendPort = receivedPort;
   }

   if(sendProtocol.compareTo(SIP_TRANSPORT_UDP_STR, UtlString::ignoreCase) == 0)
   {
      sendSucceeded = sendUdp(&responseCopy, sendAddress.data(), sendPort);
   }
   else if(sendProtocol.compareTo(SIP_TRANSPORT_TCP_STR, UtlString::ignoreCase) == 0)
   {
      sendSucceeded = sendTcp(&responseCopy, sendAddress.data(), sendPort);
   }
#ifdef HAVE_SSL
   else if(sendProtocol.compareTo(SIP_TRANSPORT_TLS_STR, UtlString::ignoreCase) == 0)
   {
      sendSucceeded = sendTls(&responseCopy, sendAddress.data(), sendPort);
   }
#endif

   return(sendSucceeded);
}

UtlBoolean SipUserAgent::sendStatelessRequest(SipMessage& request,
                                              UtlString& address,
                                              int port,
                                              OsSocket::IpProtocolSocketType protocol,
                                              UtlString& branchId)
{
   // Convert the enum to a protocol string
   UtlString viaProtocolString;
   SipMessage::convertProtocolEnumToString(protocol,
      viaProtocolString);

   // Get via info
   UtlString viaAddress;
   int viaPort;

   SipContactSelector contactSelector(*this); // can also select via ip
   contactSelector.getBestContactAddress(viaAddress, viaPort,
      SipTransport::getSipTransport(protocol), request.getLocalIp(), address, port);

   // Add the via field data
   request.addVia(viaAddress.data(),
      viaPort,
      viaProtocolString,
      branchId.data());

   // Send using the correct protocol
   UtlBoolean sendSucceeded = FALSE;
   if(protocol == OsSocket::UDP)
   {
      sendSucceeded = sendUdp(&request, address.data(), port);
   }
   else if(protocol == OsSocket::TCP)
   {
      sendSucceeded = sendTcp(&request, address.data(), port);
   }
#ifdef HAVE_SSL
   else if(protocol == OsSocket::SSL_SOCKET)
   {
      sendSucceeded = sendTls(&request, address.data(), port);
   }
#endif

   return(sendSucceeded);
}

UtlBoolean SipUserAgent::sendTcp(SipMessage* message,
                                 const char* serverAddress,
                                 int port)
{
   UtlBoolean sendSucceeded = FALSE;
   int len;
   UtlString msgBytes;
   UtlString messageStatusString = "SipUserAgent::sendTcp ";

   prepareContact(*message, serverAddress, port);

   if (mbShortNames || message->getUseShortFieldNames())
   {
      message->replaceLongFieldNames();
   }

   // Disallow an address begining with * as it gets broadcasted on NT
   if(!strchr(serverAddress,'*') && *serverAddress)
   {
      if (mSipTcpServer)
      {
         sendSucceeded = mSipTcpServer->send(message, serverAddress, port);
      }
   }
   else if(*serverAddress == '\0')
   {
      if (    isMessageLoggingEnabled() ||
         OsSysLog::willLog(FAC_SIP_OUTGOING, PRI_INFO))
      {
         message->getBytes(&msgBytes, &len);
         msgBytes.insert(0, "No send address\n");
         msgBytes.append("--------------------END--------------------\n");
         logMessage(msgBytes.data(), msgBytes.length());
         OsSysLog::add(FAC_SIP_OUTGOING, PRI_INFO, "%s", msgBytes.data());
      }
      sendSucceeded = FALSE;
   }
   else
   {
      sendSucceeded = FALSE;
   }

   if(sendSucceeded)
   {
      messageStatusString.append("TCP SIP User Agent sent message:\n");
      //osPrintf("%s", messageStatusString.data());
   }
   else
   {
      messageStatusString.append("TCP SIP User Agent failed to send message:\n");
      //osPrintf("%s", messageStatusString.data());
      message->logTimeEvent("FAILED");
   }

   if (   isMessageLoggingEnabled()
      || OsSysLog::willLog(FAC_SIP_OUTGOING, PRI_INFO)
      )
   {
      message->getBytes(&msgBytes, &len);
      messageStatusString.append("----Remote Host:");
      messageStatusString.append(serverAddress);
      messageStatusString.append("---- Port: ");
      char buff[10];
      SNPRINTF(buff, sizeof(buff), "%d", !portIsValid(port) ? 5060 : port);
      messageStatusString.append(buff);
      messageStatusString.append("----\n");

      msgBytes.insert(0, messageStatusString.data());
      msgBytes.append("--------------------END--------------------\n");
#ifdef TEST_PRINT
      osPrintf("%s", msgBytes.data());
#endif
      logMessage(msgBytes.data(), msgBytes.length());
      OsSysLog::add(FAC_SIP_OUTGOING , PRI_INFO, "%s", msgBytes.data());
   }

   return(sendSucceeded);
}


UtlBoolean SipUserAgent::sendTls(SipMessage* message,
                                 const char* serverAddress,
                                 int port)
{
#ifdef HAVE_SSL
   int sendSucceeded = FALSE;
   int len;
   UtlString msgBytes;
   UtlString messageStatusString;

   prepareContact(*message, serverAddress, port);

   if (mbShortNames || message->getUseShortFieldNames())
   {
      message->replaceLongFieldNames();
   }

   // Disallow an address begining with * as it gets broadcasted on NT
   if(!strchr(serverAddress,'*') && *serverAddress)
   {
      sendSucceeded = mSipTlsServer->send(message, serverAddress, port);
   }
   else if(*serverAddress == '\0')
   {
      if (    isMessageLoggingEnabled() ||
         OsSysLog::willLog(FAC_SIP_OUTGOING, PRI_INFO))
      {
         message->getBytes(&msgBytes, &len);
         msgBytes.insert(0, "No send address\n");
         msgBytes.append("--------------------END--------------------\n");
         logMessage(msgBytes.data(), msgBytes.length());
         OsSysLog::add(FAC_SIP_OUTGOING, PRI_INFO, "%s", msgBytes.data());
      }
      sendSucceeded = FALSE;
   }
   else
   {
      sendSucceeded = FALSE;
   }

   if(sendSucceeded)
   {
      messageStatusString.append("TLS SIP User Agent sent message:\n");
      //osPrintf("%s", messageStatusString.data());

   }
   else
   {
      messageStatusString.append("TLS SIP User Agent failed to send message:\n");
      //osPrintf("%s", messageStatusString.data());
      message->logTimeEvent("FAILED");
   }

   if (    isMessageLoggingEnabled() ||
      OsSysLog::willLog(FAC_SIP_OUTGOING, PRI_INFO))
   {
      message->getBytes(&msgBytes, &len);
      messageStatusString.append("----Remote Host:");
      messageStatusString.append(serverAddress);
      messageStatusString.append("---- Port: ");
      char buff[10];
      SNPRINTF(buff, sizeof(buff), "%d", !portIsValid(port) ? 5060 : port);
      messageStatusString.append(buff);
      messageStatusString.append("----\n");

      msgBytes.insert(0, messageStatusString.data());
      msgBytes.append("--------------------END--------------------\n");
#ifdef TEST_PRINT
      osPrintf("%s", msgBytes.data());
#endif
      logMessage(msgBytes.data(), msgBytes.length());
      OsSysLog::add(FAC_SIP_OUTGOING , PRI_INFO, "%s", msgBytes.data());
   }

   return(sendSucceeded);
#else
   return FALSE ;
#endif
}

void SipUserAgent::dispatch(SipMessage* message, int messageType)
{
   if(mbShuttingDown)
   {
      delete message;
      return;
   }
#ifdef PRINT_SIP_MESSAGE
   enableConsoleOutput(TRUE);
   osPrintf("\nSipUserAgent::dispatch\n%s\n-----------------------------------\n", message->toString().data());
#endif
   int len;
   UtlString msgBytes;
   UtlString messageStatusString;
   UtlBoolean resentWithAuth = FALSE;
   UtlBoolean isResponse = message->isResponse();
   UtlBoolean shouldDispatch = FALSE;
   SipMessage* delayedDispatchMessage = NULL;

#ifdef LOG_TIME
   OsTimeLog eventTimes;
   eventTimes.addEvent("start");
#endif

   // Get the message bytes for logging before the message is
   // potentially deleted or nulled out.
   if (   isMessageLoggingEnabled()
      || OsSysLog::willLog(FAC_SIP_INCOMING_PARSED, PRI_DEBUG)
      || OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
   {
      message->getBytes(&msgBytes, &len);
   }

   if (isResponse)
   {
      UtlString viaAddr ;
      int viaPort = -1 ;
      int receivedPort = -1 ;
      UtlString viaProtocol ;
      UtlBoolean receivedSet = false; // "received" Via parameter was set
      UtlBoolean maddrSet = false;
      UtlBoolean receivedPortSet = false; // "rport" Via parameter was set

      message->getLastVia(&viaAddr, &viaPort, &viaProtocol, &receivedPort,
         &receivedSet, &maddrSet, &receivedPortSet) ;
      if (receivedPortSet && portIsValid(receivedPort))
      {
         // rport=[port] was set, create NAT binding
         UtlString sendAddress;
         int sendPort;
         viaPort = receivedPort;

         // Inform NAT agent (used for lookups)
         message->getSendAddress(&sendAddress, &sendPort);
         OsNatAgentTask::getInstance()->addExternalBinding(NULL, 
            sendAddress, sendPort, viaAddr, viaPort); // viaAddr will be from received=[ip]

         // Inform UDP server (used for events)
         if (mSipUdpServer)
         {
            UtlString method ;
            int cseq;           
            message->getCSeqField(&cseq, &method);

            mSipUdpServer->updateSipKeepAlive(message->getLocalIp(),
               method, sendAddress, sendPort, viaAddr, viaPort) ;
         }
      }
   }

   if(messageType == SipMessageEvent::APPLICATION)
   {
#ifdef TEST_PRINT
      osPrintf("SIP User Agent received message via protocol: %d\n",
         message->getSendProtocol());
      message->logTimeEvent("DISPATCHING");
#endif

      UtlBoolean isUaTransaction = mIsUaTransactionByDefault;
      enum SipTransaction::messageRelationship relationship;
      SipTransaction* transaction =
         mSipTransactions.findTransactionFor(*message,
         FALSE, // incoming
         relationship);

#ifdef LOG_TIME
      eventTimes.addEvent("found TX");
#endif
      if(transaction == NULL)
      {
         if(isResponse)
         {
            OsSysLog::add(FAC_SIP, PRI_WARNING,"SipUserAgent::dispatch "
               "received response without transaction");

#ifdef TEST_PRINT
            if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
            {
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "=Response w/o request=>\n%s\n======================>\n",
                  msgBytes.data());

               UtlString transString;
               mSipTransactions.toStringWithRelations(transString, *message, FALSE);
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "Transaction list:\n%s\n===End transaction list===",
                  transString.data());
            }
#endif
         }
         // New transaction for incoming request
         else
         {
            transaction = new SipTransaction(message, FALSE /* incoming */,
               isUaTransaction);

            // Add the new transaction to the list
            transaction->markBusy();
            mSipTransactions.addTransaction(transaction);

            UtlString method;
            message->getRequestMethod(&method);

            if(method.compareTo(SIP_ACK_METHOD) == 0)
            {
               // This may be normal - it will occur whenever the ACK is not traversing
               // the same proxy where the transaction is completing was origniated.
               // This happens on each call setup in the authproxy, for example, because
               // the original transaction was in the forking proxy.
               relationship = SipTransaction::MESSAGE_ACK;
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipUserAgent::dispatch received ACK without transaction");
            }
            else if(method.compareTo(SIP_CANCEL_METHOD) == 0)
            {
               relationship = SipTransaction::MESSAGE_CANCEL;
               OsSysLog::add(FAC_SIP, PRI_WARNING,
                  "SipUserAgent::dispatch received CANCEL without transaction");
            }
            else
            {
               relationship = SipTransaction::MESSAGE_REQUEST;
            }
         }
      }

#ifdef LOG_TIME
      eventTimes.addEvent("handling TX");
#endif
      // This is a message that was already recieved once
      if (transaction && relationship == SipTransaction::MESSAGE_DUPLICATE)
      {
         UtlString seqMethod;
         int seqNum;
         message->getCSeqField(&seqNum, &seqMethod);
         // Resends of final INVITE responses need to be
         // passed through if they are 2xx class or the ACk
         // needs to be resent if it was a failure (i.e. 3xx,4xx,5xx,6xx)
         if (message->isResponse())
         {
            int responseCode = message->getResponseStatusCode();

            if (responseCode >= SIP_2XX_CLASS_CODE &&
               seqMethod.compareTo(SIP_INVITE_METHOD) == 0)
            {
               transaction->handleIncoming(*message,
                  *this,
                  relationship,
                  mSipTransactions,
                  delayedDispatchMessage);

               // Should never dispatch a resendof a 2xx
               if(delayedDispatchMessage)
               {
                  delete delayedDispatchMessage;
                  delayedDispatchMessage = NULL;
               }
            }
         }
         else
         {
            // is a request, but not ACK
            if (seqMethod.compareTo(SIP_ACK_METHOD) != 0)
            {
               // resend last final response
               transaction->handleIncoming(*message,
                  *this,
                  relationship,
                  mSipTransactions,
                  delayedDispatchMessage);
               if(delayedDispatchMessage)
               {
                  delete delayedDispatchMessage;
                  delayedDispatchMessage = NULL;
               }
            }
         }

         messageStatusString.append("Received duplicate message\n");
#ifdef TEST_PRINT
         osPrintf("%s", messageStatusString.data());
#endif
      }

      // The first time we received this message
      else if(transaction)
      {
         switch (relationship)
         {
         case SipTransaction::MESSAGE_FINAL:
         case SipTransaction::MESSAGE_PROVISIONAL:
         case SipTransaction::MESSAGE_CANCEL_RESPONSE:
            {
               int delayedResponseCode = -1;
               SipMessage* request = transaction->getRequest();
               isUaTransaction = transaction->isUaTransaction();

               shouldDispatch =
                  transaction->handleIncoming(*message,
                  *this,
                  relationship,
                  mSipTransactions,
                  delayedDispatchMessage);

               if(delayedDispatchMessage)
               {
                  delayedResponseCode =
                     delayedDispatchMessage->getResponseStatusCode();
               }

               // Check for Authentication Error
               if(   request
                  && delayedDispatchMessage
                  && delayedResponseCode == HTTP_UNAUTHORIZED_CODE
                  && isUaTransaction
                  )
               {
                  resentWithAuth =
                     resendWithAuthorization(delayedDispatchMessage,
                     request,
                     &messageType,
                     HttpMessage::SERVER);
               }

               // Check for Proxy Authentication Error
               if(   request
                  && delayedDispatchMessage
                  && delayedResponseCode == HTTP_PROXY_UNAUTHORIZED_CODE
                  && isUaTransaction
                  )
               {
                  resentWithAuth =
                     resendWithAuthorization(delayedDispatchMessage,
                     request,
                     &messageType,
                     HttpMessage::PROXY);
               }

               // If we requested authentication for this response,
               // validate the authorization
               UtlString requestAuthScheme;
               if(   request
                  && request->getAuthenticationScheme(&requestAuthScheme,
                  HttpMessage::SERVER))
               {
                  UtlString reqUri;
                  request->getRequestUri(&reqUri);

                  if(authorized(message, reqUri.data()))
                  {
#ifdef TEST_PRINT
                     osPrintf("response is authorized\n");
#endif
                  }

                  // What do we do with an unauthorized response?
                  // For now we just let it through.
                  else
                  {
                     OsSysLog::add(FAC_SIP, PRI_WARNING, "UNAUTHORIZED RESPONSE");
#                 ifdef TEST_PRINT
                     osPrintf("WARNING: UNAUTHORIZED RESPONSE\n");
#                 endif
                  }
               }

               // If we have a request for this incoming response
               // Forward it on to interested applications
               if (   request
                  && (shouldDispatch || delayedDispatchMessage)
                  )
               {
                  UtlString method;
                  request->getRequestMethod(&method);
                  OsMsgQ* responseQ = NULL;
                  responseQ =  request->getResponseListenerQueue();
                  if (responseQ  && shouldDispatch)
                  {
                     SipMessage * msg = new SipMessage(*message);
                     msg->setResponseListenerData(request->getResponseListenerData() );
                     SipMessageEvent eventMsg(msg);
                     eventMsg.setMessageStatus(messageType);
                     responseQ->send(eventMsg);
                     // The SipMessage gets freed with the SipMessageEvent
                     msg = NULL;
                  }

                  if(responseQ  && delayedDispatchMessage)
                  {
                     SipMessage* tempDelayedDispatchMessage =
                        new SipMessage(*delayedDispatchMessage);

                     tempDelayedDispatchMessage->setResponseListenerData(
                        request->getResponseListenerData());

                     SipMessageEvent eventMsg(tempDelayedDispatchMessage);
                     eventMsg.setMessageStatus(messageType);
                     if (!mbShuttingDown)
                     {
                        responseQ->send(eventMsg);
                     }
                     // The SipMessage gets freed with the SipMessageEvent
                     tempDelayedDispatchMessage = NULL;
                  }
               }
            }
            break;

         case SipTransaction::MESSAGE_REQUEST:
            {
               // if this is a request check if it is supported
               SipMessage* response = NULL;
               UtlString disallowedExtensions;
               UtlString method;
               UtlString allowedMethods;
               UtlString contentEncoding;
               UtlString toAddress;
               UtlString fromAddress;
               UtlString uriAddress;
               UtlString protocol;
               UtlString sipVersion;
               int port;
               int seqNumber;
               UtlString seqMethod;
               UtlString callIdField;
               int maxForwards;

               message->getRequestMethod(&method);
               if(isUaTransaction)
               {
                  getAllowedMethods(&allowedMethods);
                  whichExtensionsNotAllowed(message, &disallowedExtensions);
                  message->getContentEncodingField(&contentEncoding);

                  //delete leading and trailing white spaces
                  disallowedExtensions = disallowedExtensions.strip(UtlString::both);
                  allowedMethods = allowedMethods.strip(UtlString::both);
                  contentEncoding = contentEncoding.strip(UtlString::both);
               }

               message->getToAddress(&toAddress, &port, &protocol);
               message->getFromAddress(&fromAddress, &port, &protocol);
               message->getUri(&uriAddress, &port, &protocol);
               message->getRequestProtocol(&sipVersion);
               sipVersion.toUpper();
               message->getCSeqField(&seqNumber, &seqMethod);
               seqMethod.toUpper();
               message->getCallIdField(&callIdField);

               // Check if the method is supported
               if(   isUaTransaction
                  && !isMethodAllowed(method.data())
                  )
               {
                  response = new SipMessage();

                  response->setRequestUnimplemented(message);
               }

               // Check if the extensions are supported
               else if(   mDoUaMessageChecks
                  && isUaTransaction
                  && !disallowedExtensions.isNull()
                  )
               {
                  response = new SipMessage();
                  response->setRequestBadExtension(message,
                     disallowedExtensions);
               }

               // Check if the encoding is supported
               // i.e. no encoding
               else if(   mDoUaMessageChecks
                  && isUaTransaction
                  && !contentEncoding.isNull()
                  )
               {
                  response = new SipMessage();
                  response->setRequestBadContentEncoding(message,"");
               }

               // Check the addresses are present
               else if(toAddress.isNull() || fromAddress.isNull() ||
                  uriAddress.isNull())
               {
                  response = new SipMessage();
                  response->setRequestBadAddress(message);
               }

               // Check SIP version
               else if(strcmp(sipVersion.data(), SIP_PROTOCOL_VERSION) != 0)
               {
                  response = new SipMessage();
                  response->setRequestBadVersion(message);
               }

               // Check for missing CSeq or Call-Id
               else if(callIdField.isNull() || seqNumber < 0 ||
                  strcmp(seqMethod.data(), method.data()) != 0)
               {
                  response = new SipMessage();
                  response->setRequestBadRequest(message);
               }

               // Authentication Required
               else if(isUaTransaction &&
                  shouldAuthenticate(message))
               {
                  if(!authorized(message))
                  {
#ifdef TEST_PRINT
                     osPrintf("SipUserAgent::dispatch message Unauthorized\n");
#endif
                     response = new SipMessage();
                     response->setRequestUnauthorized(message,
                        mAuthenticationScheme.data(),
                        mAuthenticationRealm.data(),
                        "1234567890", // :TODO: nonce should be generated by SipNonceDb
                        "abcdefghij"  // opaque
                        );
                  }
#ifdef TEST_PRINT
                  else
                  {
                     osPrintf("SipUserAgent::dispatch message Authorized\n");
                  }
#endif //TEST_PRINT
               }
               else if(message->getMaxForwards(maxForwards))
               {
                  if(maxForwards <= 0)
                  {
                     response = new SipMessage();
                     response->setResponseData(message,
                        SIP_TOO_MANY_HOPS_CODE,
                        SIP_TOO_MANY_HOPS_TEXT);

                     response->setWarningField(SIP_WARN_MISC_CODE, mDefaultIpAddress.data(),
                        SIP_TOO_MANY_HOPS_TEXT
                        );

                     setSelfHeader(*response);

                     // If we are suppose to return the vias in the
                     // error response for Max-Forwards exeeded
                     if(mReturnViasForMaxForwards)
                     {

                        // The setBody method frees up the body before
                        // setting the new one, if there is a body
                        // We remove the body so that we can serialize
                        // the message without getting the body
                        message->setBody(NULL);

                        UtlString sipFragString;
                        int sipFragLen;
                        message->getBytes(&sipFragString, &sipFragLen);

                        // Create a body to contain the Vias from the request
                        HttpBody* sipFragBody =
                           new HttpBody(sipFragString.data(),
                           sipFragLen,
                           CONTENT_TYPE_MESSAGE_SIPFRAG);

                        // Attach the body to the response
                        response->setBody(sipFragBody);

                        // Set the content type of the body to be sipfrag
                        response->setContentType(CONTENT_TYPE_MESSAGE_SIPFRAG);
                     }

                     delete(message);
                     message = NULL;
                  }
               }
               else
               {
                  message->setMaxForwards(mMaxForwards);
               }

               // Process Options requests :TODO: - this does not route in the redirect server
               if(!response && isUaTransaction &&
                  !message->isResponse() &&
                  method.compareTo(SIP_OPTIONS_METHOD) == 0)
               {
                  UtlString toFieldTag;
                  message->getToFieldTag(toFieldTag);
                  if (toFieldTag.isNull())
                  {
                     // Send an OK, the allowed field will get added to all final responces.
                     response = new SipMessage();
                     response->setResponseData(message,
                        SIP_OK_CODE,
                        SIP_OK_TEXT);

                     delete(message);
                     message = NULL;
                  }
               }

               // If the request is invalid
               if(response)
               {
                  addAgentCapabilities(*response);
                  // Send the error response
                  transaction->handleOutgoing(*response,
                     *this,
                     mSipTransactions,
                     SipTransaction::MESSAGE_FINAL);
                  delete response;
                  response = NULL;
                  if(message) delete message;
                  message = NULL;
               }
               else if(message)
               {
                  shouldDispatch =
                     transaction->handleIncoming(*message,
                     *this,
                     relationship,
                     mSipTransactions,
                     delayedDispatchMessage);
               }
               else
               {
                  OsSysLog::add(FAC_SIP, PRI_ERR, "SipUserAgent::dispatch NULL message to handle");
                  //osPrintf("ERROR: SipUserAgent::dispatch NULL message to handle\n");
               }
            }
            break;

         case SipTransaction::MESSAGE_ACK:
         case SipTransaction::MESSAGE_2XX_ACK:
         case SipTransaction::MESSAGE_CANCEL:
            {
               int maxForwards;

               // Check the ACK max-forwards has not gone too many hopes
               if(!isResponse &&
                  (relationship == SipTransaction::MESSAGE_ACK ||
                  relationship == SipTransaction::MESSAGE_2XX_ACK) &&
                  message->getMaxForwards(maxForwards) &&
                  maxForwards <= 0 )
               {

                  // Drop ACK on the floor.
                  if(message) delete(message);
                  message = NULL;
               }

               else if(message)
               {
                  shouldDispatch =
                     transaction->handleIncoming(*message,
                     *this,
                     relationship,
                     mSipTransactions,
                     delayedDispatchMessage);
               }
            }
            break;

         case SipTransaction::MESSAGE_NEW_FINAL:
            {
               // Forward it on to interested applications
               SipMessage* request = transaction->getRequest();
               shouldDispatch = TRUE;
               if( request)
               {
                  UtlString method;
                  request->getRequestMethod(&method);
                  OsMsgQ* responseQ = NULL;
                  responseQ =  request->getResponseListenerQueue();
                  if (responseQ)
                  {
                     SipMessage * msg = new SipMessage(*message);
                     msg->setResponseListenerData(request->getResponseListenerData() );
                     SipMessageEvent eventMsg(msg);
                     eventMsg.setMessageStatus(messageType);
                     responseQ->send(eventMsg);
                     // The SipMessage gets freed with the SipMessageEvent
                     msg = NULL;
                  }
               }
            }
            break;

         default:
            {
               if (OsSysLog::willLog(FAC_SIP, PRI_WARNING))
               {
                  UtlString relationString;
                  SipTransaction::getRelationshipString(relationship, relationString);
                  OsSysLog::add(FAC_SIP, PRI_WARNING, 
                     "SipUserAgent::dispatch unhandled incoming message: %s",
                     relationString.data());
               }
            }
            break;
         }
      }

      if(transaction)
      {
         mSipTransactions.markAvailable(*transaction);
      }
   }
   else
   {
      shouldDispatch = TRUE;
      messageStatusString.append("SIP User agent FAILED to send message:\n");
   }

#ifdef LOG_TIME
   eventTimes.addEvent("queuing");
#endif

   if (    isMessageLoggingEnabled()
      || OsSysLog::willLog(FAC_SIP_INCOMING_PARSED, PRI_DEBUG)
      )
   {
      msgBytes.insert(0, messageStatusString.data());
      msgBytes.append("++++++++++++++++++++END++++++++++++++++++++\n");
#ifdef TEST_PRINT
      osPrintf("%s", msgBytes.data());
#endif
      logMessage(msgBytes.data(), msgBytes.length());
      OsSysLog::add(FAC_SIP_INCOMING_PARSED, PRI_DEBUG, "%s", msgBytes.data());
   }

   if(message && shouldDispatch)
   {
#ifdef TEST_PRINT
      osPrintf("DISPATCHING message\n");
#endif

      queueMessageToObservers(message, messageType);
   }
   else
   {
      delete message;
      message = NULL;
   }

   if(delayedDispatchMessage)
   {
      if (   isMessageLoggingEnabled()
         || OsSysLog::willLog(FAC_SIP_INCOMING_PARSED, PRI_DEBUG)
         )
      {
         UtlString delayMsgString;
         int delayMsgLen;
         delayedDispatchMessage->getBytes(&delayMsgString,
            &delayMsgLen);
         delayMsgString.insert(0, "SIP User agent delayed dispatch message:\n");
         delayMsgString.append("++++++++++++++++++++END++++++++++++++++++++\n");
#ifdef TEST_PRINT
         osPrintf("%s", delayMsgString.data());
#endif
         logMessage(delayMsgString.data(), delayMsgString.length());
         OsSysLog::add(FAC_SIP_INCOMING_PARSED, PRI_DEBUG, "%s",
            delayMsgString.data());
      }

      queueMessageToObservers(delayedDispatchMessage, messageType);
   }

#ifdef LOG_TIME
   eventTimes.addEvent("GC");
#endif

   // All garbage collection should now be done in the
   // context of the SipUserAgent to prevent hickups in
   // the reading of SipMessages off the sockets.
   //garbageCollection();

#ifdef LOG_TIME
   eventTimes.addEvent("finish");
   UtlString timeString;
   eventTimes.getLogString(timeString);
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipUserAgent::dispatch time log: %s",
      timeString.data());
#endif
}

#undef LOG_TIME

void SipUserAgent::queueMessageToObservers(SipMessage* message,
                                           int messageType)
{
   UtlString callId;
   message->getCallIdField(&callId);
   UtlString method;
   message->getRequestMethod(&method);

   // Create a new message event
   SipMessageEvent event(message);
   event.setMessageStatus(messageType);

   // Find all of the observers which are interested in
   // this method and post the message
   UtlBoolean isRsp = message->isResponse();
   if(isRsp)
   {
      int cseq;
      message->getCSeqField(&cseq, &method);
   }

   queueMessageToInterestedObservers(event, method);
   // send it to those with no method descrimination as well
   queueMessageToInterestedObservers(event, "");

   // Do not delete the message it gets deleted with the event
   message = NULL;
}

void SipUserAgent::queueMessageToInterestedObservers(SipMessageEvent& event,
                                                     const UtlString& method)
{
   const SipMessage* message;
   if((message = event.getMessage()))
   {
      // Find all of the observers which are interested in
      // this method and post the message
      UtlString messageEventName;
      message->getEventField(messageEventName);

      // do these constructors before taking the lock
      UtlString observerMatchingMethod(method);

      // lock the message observer list
      OsReadLock lock(mObserverMutex);

      UtlHashBagIterator observerIterator(mMessageObservers, &observerMatchingMethod);
      SipObserverCriteria* observerCriteria;
      while ((observerCriteria = (SipObserverCriteria*) observerIterator()))
      {
         // Check message direction and type
         if (   (  message->isResponse() && observerCriteria->wantsResponses())
            || (! message->isResponse() && observerCriteria->wantsRequests())
            )
         {
            // Decide if the event filter applies
            bool useEventFilter = false;
            bool matchedEvent = false;
            if (! message->isResponse()) // events apply only to requests
            {
               UtlString criteriaEventName;
               observerCriteria->getEventName(criteriaEventName);

               useEventFilter = ! criteriaEventName.isNull();
               if (useEventFilter)
               {
                  // see if the event type matches
                  matchedEvent = (   (   method.compareTo(SIP_SUBSCRIBE_METHOD,
                     UtlString::ignoreCase)
                     == 0
                     || method.compareTo(SIP_NOTIFY_METHOD,
                     UtlString::ignoreCase)
                     == 0
                     )
                     && 0==messageEventName.compareTo(criteriaEventName,
                     UtlString::ignoreCase
                     )
                     );
               }
            } // else - this is a response - event filter is not applicable

            // Check to see if the session criteria matters
            const SipDialog* pCriteriaSipDialog = observerCriteria->getSipDialog();
            bool useSessionFilter = (NULL != pCriteriaSipDialog);
            UtlBoolean matchedSession = FALSE;
            if (useSessionFilter)
            {
               // it matters; see if it matches
               matchedSession = pCriteriaSipDialog->isInitialDialogOf(*message);
            }

            // We have a message type (req|rsp) the observer wants - apply filters
            if (   (! useSessionFilter || matchedSession)
               && (! useEventFilter   || matchedEvent)
               )
            {
               // This event is interesting, so send it up...
               OsMsgQ* observerQueue = observerCriteria->getObserverQueue();
               void* observerData = observerCriteria->getObserverData();

               // Cheat a little and set the observer data to be passed back
               ((SipMessage*) message)->setResponseListenerData(observerData);

               // Put the message in the observers queue
               if (!mbShuttingDown)
               {
                  int numMsgs = observerQueue->numMsgs();
                  int maxMsgs = observerQueue->maxMsgs();
                  if (numMsgs < maxMsgs)
                  {
                     observerQueue->send(event);
                  }
                  else
                  {
                     OsSysLog::add(FAC_SIP, PRI_ERR,
                        "queueMessageToInterestedObservers - queue full (name=%s, numMsgs=%d)",
                        observerQueue->getName(), numMsgs);
                  }
               }
            }
         }
         else
         {
            // either direction or req/rsp not a match
         }
      } // while observers
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT, "queueMessageToInterestedObservers - no message");
   }
}


UtlBoolean checkMethods(SipMessage* message)
{
   return(TRUE);
}

UtlBoolean checkExtensions(SipMessage* message)
{
   return(TRUE);
}


UtlBoolean SipUserAgent::handleMessage(OsMsg& eventMessage)
{
   UtlBoolean messageProcessed = FALSE;
   //osPrintf("SipUserAgent: handling message\n");
   int msgType = eventMessage.getMsgType();
   int msgSubType = eventMessage.getMsgSubType();
   // Print message if input queue to SipUserAgent exceeds 100.
   if (getMessageQueue()->numMsgs() > 50)
   {
      SipMessageEvent* sipEvent;

      OsSysLog::add(FAC_SIP, PRI_DEBUG,
         "SipUserAgent::handleMessage msgType = %d, msgSubType = %d, msgEventType = %d, "
         "queue length = %d",
         msgType, msgSubType, 
         // Only extract msgEventType if msgType and msgSubType are right.
         msgType == OsMsg::OS_EVENT && msgSubType == OsEventMsg::NOTIFY ?
         (((OsEventMsg&)eventMessage).getUserData((int&)sipEvent),
         sipEvent ? sipEvent->getMessageStatus() : -98) :
         -99 /* dummy value */,
         getMessageQueue()->numMsgs());
   }

   if(msgType == OsMsg::PHONE_APP)
   {
      // Final message from SipUserAgent::shutdown - all timers are stopped and are safe to delete
      if(msgSubType == SipUserAgent::SHUTDOWN_MESSAGE)
      {
#ifdef TEST_PRINT
         osPrintf("SipUserAgent::handleMessage shutdown complete message.\n");
#endif
         mSipTransactions.deleteTransactionTimers();

         if(mbBlockingShutdown == TRUE)
         {
            OsEvent* pEvent = ((OsRpcMsg&)eventMessage).getEvent();

            OsStatus res = pEvent->signal(OS_SUCCESS);
            assert(res == OS_SUCCESS);
         }
         else
         {
            mbShutdownDone = TRUE;
         }
      }
      else if (msgSubType == SipUserAgent::KEEPALIVE_MESSAGE)
      {
         OsPtrMsg& msg = (OsPtrMsg&) eventMessage ;

         SipUdpServer* pUdpServer = (SipUdpServer*) msg.getPtr() ;
         OsTimer* pTimer = (OsTimer*) msg.getPtr2() ;

         if (pUdpServer && pTimer)
         {
            pUdpServer->sendSipKeepAlive(pTimer) ;
         }
      } 
      else
      {
         SipMessage* sipMsg = (SipMessage*)((SipMessageEvent&)eventMessage).getMessage();
         if(sipMsg)
         {
            //messages for which the UA is consumer will end up here.
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipUserAgent::handleMessage posting message");

            // I cannot remember what kind of message ends up here???
            if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
            {
               int len;
               UtlString msgBytes;
               sipMsg->getBytes(&msgBytes, &len);
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "??????????????????????????????????????\n"
                  "%s???????????????????????????????????\n",
                  msgBytes.data());
            }
         }
      }
      messageProcessed = TRUE;
   }

   // A timer expired
   else if(msgType == OsMsg::OS_EVENT &&
      msgSubType == OsEventMsg::NOTIFY)
   {
      OsTimer* timer;
      SipMessageEvent* sipEvent = NULL;

      ((OsEventMsg&)eventMessage).getUserData((int&)sipEvent);
      ((OsEventMsg&)eventMessage).getEventData((int&)timer);

      if(sipEvent)
      {
         const SipMessage* sipMessage = sipEvent->getMessage();
         int msgEventType = sipEvent->getMessageStatus();

         // Resend timeout
         if(msgEventType == SipMessageEvent::TRANSACTION_RESEND)
         {
            if(sipMessage)
            {
               // Note: only delete the timer and notifier if there
               // is a message AND we can get a lock on the transaction.  
               //  WARNING: you cannot touch the contents of the transaction
               // attached to the message until the transaction has been
               // locked (via findTransactionFor, if no transaction is 
               // returned, it either no longer exists or we could not get
               // a lock for it.

#              ifdef TEST_PRINT
               {
                  UtlString callId;
                  int protocolType = sipMessage->getSendProtocol();
                  sipMessage->getCallIdField(&callId);

                  if(sipMessage->getSipTransaction() == NULL)
                  {
                     osPrintf("SipUserAgent::handleMessage "
                        "resend Timeout message with NULL transaction\n");
                  }
                  osPrintf("SipUserAgent::handleMessage "
                     "resend Timeout of message for %d protocol, callId: \"%s\" \n",
                     protocolType, callId.data());
               }
#              endif


               int nextTimeout = -1;
               enum SipTransaction::messageRelationship relationship;
               //mSipTransactions.lock();
               SipTransaction* transaction =
                  mSipTransactions.findTransactionFor(*sipMessage,
                  TRUE, // timers are only set for outgoing messages I think
                  relationship);
               if(transaction)
               {
                  if(timer)
                  {
                     transaction->removeTimer(timer);

                     delete timer;
                     timer = NULL;
                  }

                  // If we are in shutdown mode, unlock the transaction
                  // and set it to null.  We pretend that the transaction
                  // does not exist (i.e. noop).
                  if(mbShuttingDown)
                  {
                     mSipTransactions.markAvailable(*transaction);
                     transaction = NULL;
                  }
               }


               // If we cannot lock it, it does not exist (or atleast
               // pretend it does not exist.  The transaction will be
               // null if it has been deleted or we cannot get a lock
               // on the transaction.  
               if(transaction)
               {
                  SipMessage* delayedDispatchMessage = NULL;
                  const UtlString transportName = sipMessage->getTransportName();
                  transaction->handleResendEvent(*sipMessage,
                     *this,
                     relationship,
                     mSipTransactions,
                     nextTimeout,
                     delayedDispatchMessage);

                  if(nextTimeout == 0)
                  {
                     if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
                     {
                        UtlString transactionString;
                        transaction->toString(transactionString, TRUE);
                        transactionString.insert(0,
                           "SipUserAgent::handleMessage "
                           "timeout send failed\n"
                           );
                        OsSysLog::add(FAC_SIP, PRI_DEBUG, "%s\n", transactionString.data());
                        //osPrintf("%s\n", transactionString.data());
                     }
                  }

                  if(delayedDispatchMessage)
                  {
                     // Only bother processing if the logs are enabled
                     if (    isMessageLoggingEnabled() ||
                        OsSysLog::willLog(FAC_SIP_INCOMING, PRI_DEBUG))
                     {
                        UtlString delayMsgString;
                        int delayMsgLen;
                        delayedDispatchMessage->getBytes(&delayMsgString,
                           &delayMsgLen);
                        delayMsgString.insert(0, "SIP User agent delayed dispatch message:\n");
                        delayMsgString.append("++++++++++++++++++++END++++++++++++++++++++\n");
#ifdef TEST_PRINT
                        osPrintf("%s", delayMsgString.data());
#endif
                        logMessage(delayMsgString.data(), delayMsgString.length());
                        OsSysLog::add(FAC_SIP_INCOMING_PARSED, PRI_DEBUG,"%s",
                           delayMsgString.data());
                     }

                     // if the request has a responseQueue, post the response.
                     OsMsgQ* responseQ = NULL;
                     responseQ =  sipMessage->getResponseListenerQueue();
                     if ( responseQ &&
                        !sipMessage->isResponse() &&
                        delayedDispatchMessage->isResponse())
                     {
                        SipMessage *messageToQ = new SipMessage(*delayedDispatchMessage);

                        messageToQ->setResponseListenerData(sipMessage->getResponseListenerData());
                        SipMessageEvent eventMsg(messageToQ);
                        eventMsg.setMessageStatus(SipMessageEvent::APPLICATION);
                        responseQ->send(eventMsg);
                        // The SipMessage gets freed with the SipMessageEvent
                        messageToQ = NULL;
                     }

                     queueMessageToObservers(delayedDispatchMessage,
                        SipMessageEvent::APPLICATION
                        );

                     // delayedDispatchMessage gets freed in queueMessageToObservers
                     delayedDispatchMessage = NULL;
                  }
               }

               // No transaction for this timeout
               else
               {
                  OsSysLog::add(FAC_SIP, PRI_ERR, "SipUserAgent::handleMessage "
                     "SIP message timeout expired with no matching transaction");

                  // Somehow the transaction got deleted perhaps it timed
                  // out and there was a log jam that prevented the handling
                  // of the timeout ????? This should not happen.
               }

               if(transaction)
               {
                  mSipTransactions.markAvailable(*transaction);
               }

               // Do this outside so that we do not get blocked
               // on locking or delete the transaction out
               // from under ouselves
               if(nextTimeout == 0)
               {
                  // Make a copy and dispatch it
                  dispatch(new SipMessage(*sipMessage),
                     SipMessageEvent::TRANSPORT_ERROR);
               }

               // The timer made its own copy of this message.
               // It is deleted by dispatch ?? if it is not
               // rescheduled.
            } // End if sipMessage
         } // End SipMessageEvent::TRANSACTION_RESEND

         // Timeout for garbage collection
         else if(msgEventType == SipMessageEvent::TRANSACTION_GARBAGE_COLLECTION)
         {
#ifdef TEST_PRINT
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
               "SipUserAgent::handleMessage garbage collecting");
            osPrintf("SipUserAgent::handleMessage garbage collecting\n");
#endif
         }

         // Timeout for an transaction to expire
         else if(msgEventType == SipMessageEvent::TRANSACTION_EXPIRATION)
         {
#ifdef TEST_PRINT
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipUserAgent::handleMessage transaction expired");
#endif
            if(sipMessage)
            {
               // Note: only delete the timer and notifier if there
               // is a message AND we can get a lock on the transaction.  
               //  WARNING: you cannot touch the contents of the transaction
               // attached to the message until the transaction has been
               // locked (via findTransactionFor, if no transaction is 
               // returned, it either no longer exists or we could not get
               // a lock for it.

#ifdef TEST_PRINT
               if(sipMessage->getSipTransaction() == NULL)
               {
                  osPrintf("SipUserAgent::handleMessage expires Timeout message with NULL transaction\n");
               }
#endif
               int nextTimeout = -1;
               enum SipTransaction::messageRelationship relationship;
               //mSipTransactions.lock();
               SipTransaction* transaction =
                  mSipTransactions.findTransactionFor(*sipMessage,
                  TRUE, // timers are only set for outgoing?
                  relationship);
               if(transaction)
               {
                  if(timer)
                  {
                     transaction->removeTimer(timer);

                     delete timer;
                     timer = NULL;
                  }

                  // If we are in shutdown mode, unlock the transaction
                  // and set it to null.  We pretend that the transaction
                  // does not exist (i.e. noop).
                  if(mbShuttingDown)
                  {
                     mSipTransactions.markAvailable(*transaction);
                     transaction = NULL;
                  }
               }

               if(transaction)
               {
                  SipMessage* delayedDispatchMessage = NULL;
                  transaction->handleExpiresEvent(*sipMessage,
                     *this,
                     relationship,
                     mSipTransactions,
                     nextTimeout,
                     delayedDispatchMessage);

                  mSipTransactions.markAvailable(*transaction);

                  if(delayedDispatchMessage)
                  {
                     // Only bother processing if the logs are enabled
                     if (    isMessageLoggingEnabled() ||
                        OsSysLog::willLog(FAC_SIP_INCOMING_PARSED, PRI_DEBUG))
                     {
                        UtlString delayMsgString;
                        int delayMsgLen;
                        delayedDispatchMessage->getBytes(&delayMsgString,
                           &delayMsgLen);
                        delayMsgString.insert(0, "SIP User agent delayed dispatch message:\n");
                        delayMsgString.append("++++++++++++++++++++END++++++++++++++++++++\n");
#ifdef TEST_PRINT
                        osPrintf("%s", delayMsgString.data());
#endif
                        logMessage(delayMsgString.data(), delayMsgString.length());
                        OsSysLog::add(FAC_SIP_INCOMING_PARSED, PRI_DEBUG,"%s",
                           delayMsgString.data());
                     }

                     // wdn - if the request has a responseQueue, post the response.
                     OsMsgQ* responseQ = NULL;
                     responseQ =  sipMessage->getResponseListenerQueue();
                     if ( responseQ &&
                        !sipMessage->isResponse() &&
                        delayedDispatchMessage->isResponse())
                     {
                        SipMessage *messageToQ = new SipMessage(*delayedDispatchMessage);

                        messageToQ->setResponseListenerData(sipMessage->getResponseListenerData());
                        SipMessageEvent eventMsg(messageToQ);
                        eventMsg.setMessageStatus(SipMessageEvent::APPLICATION);
                        responseQ->send(eventMsg);
                        // The SipMessage gets freed with the SipMessageEvent
                        messageToQ = NULL;
                     }

                     // delayedDispatchMessage gets freed in queueMessageToObservers
                     queueMessageToObservers(delayedDispatchMessage,
                        SipMessageEvent::APPLICATION
                        );

                     //delayedDispatchMessage gets freed in queueMessageToObservers
                     delayedDispatchMessage = NULL;
                  }
               }
               else // Could not find a transaction for this exired message
               {
                  if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
                  {
                     UtlString noTxMsgString;
                     int noTxMsgLen;
                     sipMessage->getBytes(&noTxMsgString, &noTxMsgLen);

                     OsSysLog::add(FAC_SIP, PRI_DEBUG,
                        "SipUserAgent::handleMessage "
                        "event timeout with no matching transaction: %s",
                        noTxMsgString.data());
                  }
               }
            }
         }

         // Unknown timeout
         else
         {
            OsSysLog::add(FAC_SIP, PRI_WARNING,
               "SipUserAgent::handleMessage unknown timeout event: %d.", msgEventType);
#           ifdef TEST_PRINT
            osPrintf("ERROR: SipUserAgent::handleMessage unknown timeout event: %d.\n",
               msgEventType);
#           endif
         }

         // As this is OsMsg is attached as a void* to the timeout event
         // it must be explicitly deleted.  The attached SipMessage
         // will get freed with it.
         delete sipEvent;
         sipEvent = NULL;
      } // end if sipEvent
      messageProcessed = TRUE;
   }

   else
   {
#ifdef TEST_PRINT
      osPrintf("SipUserAgent: Unknown message type: %d\n", msgType);
#endif
      messageProcessed = TRUE;
   }

   // Only GC if no messages are waiting -- othewise we may delete a timer 
   // that is queued up for us.
   if (getMessageQueue()->isEmpty())
   {
      garbageCollection();
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
         "SipUserAgent::handleMessage after GC, queue size = %d",
         getMessageQueue()->numMsgs());
   }
   return(messageProcessed);
}

void SipUserAgent::garbageCollection()
{
   OsTime time;
   OsDateTime::getCurTimeSinceBoot(time);
   long bootime = time.seconds();

   long then = bootime - (mTransactionStateTimeoutMs / 1000);
   long tcpThen = bootime - mMaxTcpSocketIdleTime;
   long oldTransaction = then - (mTransactionStateTimeoutMs / 1000);
   long oldInviteTransaction = then - mMinInviteTransactionTimeout;

   // If the timeout is negative we never timeout or garbage collect
   // tcp connections
   if(mMaxTcpSocketIdleTime < 0)
   {
      tcpThen = -1;
   }

   if(mLastCleanUpTime < then)
   {
#      ifdef LOG_TIME
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
         "SipUserAgent::garbageCollection"
         " bootime: %ld then: %ld tcpThen: %ld"
         " oldTransaction: %ld oldInviteTransaction: %ld",
         bootime, then, tcpThen, oldTransaction,
         oldInviteTransaction);
#endif
      mSipTransactions.removeOldTransactions(oldTransaction,
         oldInviteTransaction);
#      ifdef LOG_TIME
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
         "SipUserAgent::garbageCollection starting removeOldClients(udp)");
#      endif

      // Changed by Udit for null pointer check
      if (mSipUdpServer)
      {
         mSipUdpServer->removeOldClients(then);
      }

      if (mSipTcpServer)
      {
#         ifdef LOG_TIME
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
            "SipUserAgent::garbageCollection starting removeOldClients(tcp)");
#         endif
         mSipTcpServer->removeOldClients(tcpThen);
      }
#ifdef HAVE_SSL
      if (mSipTlsServer)
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
            "SipUserAgent::garbageCollection starting removeOldClients(tls)");
         mSipTlsServer->removeOldClients(tcpThen);
      }
#endif
#      ifdef LOG_TIME
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
         "SipUserAgent::garbageCollection done");
#      endif
      mLastCleanUpTime = bootime;
   }
}

UtlBoolean SipUserAgent::addCrLfKeepAlive(const char* szLocalIp,
                                          const char* szRemoteIp,
                                          const int   remotePort,
                                          const int   keepAliveSecs,
                                          OsNatKeepaliveListener* pListener)
{
   UtlBoolean bSuccess = false ;

   if (mSipUdpServer)
   {
      bSuccess = mSipUdpServer->addCrLfKeepAlive(szLocalIp, szRemoteIp, 
         remotePort, keepAliveSecs, pListener) ;
   }

   return bSuccess ;
}


UtlBoolean SipUserAgent::removeCrLfKeepAlive(const char* szLocalIp,
                                             const char* szRemoteIp,
                                             const int   remotePort) 
{
   UtlBoolean bSuccess = false ;

   if (mSipUdpServer)
   {
      bSuccess = mSipUdpServer->removeCrLfKeepAlive(szLocalIp, szRemoteIp, 
         remotePort) ;
   }

   return bSuccess ;
}

UtlBoolean SipUserAgent::addStunKeepAlive(const char* szLocalIp,
                                          const char* szRemoteIp,
                                          const int   remotePort,
                                          const int   keepAliveSecs,
                                          OsNatKeepaliveListener* pListener)
{
   UtlBoolean bSuccess = false ;

   if (mSipUdpServer)
   {
      bSuccess = mSipUdpServer->addStunKeepAlive(szLocalIp, szRemoteIp, 
         remotePort, keepAliveSecs, pListener) ;
   }

   return bSuccess ;
}

UtlBoolean SipUserAgent::removeStunKeepAlive(const char* szLocalIp,
                                             const char* szRemoteIp,
                                             const int   remotePort) 
{
   UtlBoolean bSuccess = false ;

   if (mSipUdpServer)
   {
      bSuccess = mSipUdpServer->removeStunKeepAlive(szLocalIp, szRemoteIp, 
         remotePort) ;
   }

   return bSuccess ;
}


UtlBoolean SipUserAgent::addSipKeepAlive(const char* szLocalIp,
                                         const char* szRemoteIp,
                                         const int   remotePort,
                                         const char* szMethod,
                                         const int   keepAliveSecs,
                                         OsNatKeepaliveListener* pListener)
{
   UtlBoolean bSuccess = false ;

   if (mSipUdpServer)
   {
      bSuccess = mSipUdpServer->addSipKeepAlive(szLocalIp, szRemoteIp, 
         remotePort, szMethod, keepAliveSecs, pListener) ;
   }

   return bSuccess ;
}

UtlBoolean SipUserAgent::removeSipKeepAlive(const char* szLocalIp,
                                            const char* szRemoteIp,
                                            const int   remotePort,
                                            const char* szMethod) 
{
   UtlBoolean bSuccess = false ;

   if (mSipUdpServer)
   {
      bSuccess = mSipUdpServer->removeSipKeepAlive(szLocalIp, szRemoteIp, 
         remotePort, szMethod) ;
   }

   return bSuccess ;
}

/* ============================ ACCESSORS ================================= */

// Enable or disable the outbound use of rport (send packet to actual
// port -- not advertised port).
UtlBoolean SipUserAgent::setUseRport(UtlBoolean bEnable)
{
   UtlBoolean bOld = mbUseRport ;

   mbUseRport = bEnable ;

   return bOld ;
}

// Is use report set?
UtlBoolean SipUserAgent::getUseRport() const
{
   return mbUseRport ;
}

void SipUserAgent::setUserAgentName(const UtlString& name)
{
   defaultUserAgentName = name;
   return;
}

const UtlString& SipUserAgent::getUserAgentName() const
{
   return defaultUserAgentName;
}

// Get the local address and port
UtlBoolean SipUserAgent::getLocalAddress(UtlString* pIpAddress, int* pPort, SIP_TRANSPORT_TYPE protocol, const UtlString& preferredIp)
{
   if (pIpAddress)
   {
      *pIpAddress = NULL;

      if (!preferredIp.isNull())
      {
         // we have preferred ip address, try to use it if possible
         switch (protocol)
         {
         default:
         case SIP_TRANSPORT_UDP:
            if (mSipUdpServer && mSipUdpServer->isBoundTo(preferredIp))
            {
               // server is bound to preferred ip, we can use it
               *pIpAddress = preferredIp;
            }
            break;
         case SIP_TRANSPORT_TCP:
            if (mSipTcpServer && mSipTcpServer->isBoundTo(preferredIp))
            {
               // server is bound to preferred ip, we can use it
               *pIpAddress = preferredIp;
            }
            break;
#ifdef HAVE_SSL
         case SIP_TRANSPORT_TLS:
            if (mSipTlsServer && mSipTlsServer->isBoundTo(preferredIp))
            {
               // server is bound to preferred ip, we can use it
               *pIpAddress = preferredIp;
            }
            break;
#endif
         }
      }

      if (pIpAddress->isNull())
      {
         // if ip address was not selected from preferred ip
         if (!mDefaultIpAddress.isNull())
         {
            *pIpAddress = mDefaultIpAddress;
         }
         else
         {
            OsSocket::getHostIp(pIpAddress);
         }
      }
   }

   if (pPort)
   {
      switch (protocol)
      {
      case SIP_TRANSPORT_UDP:
         if (mSipUdpServer)
            *pPort = mSipUdpServer->getServerPort();
         break;
      case SIP_TRANSPORT_TCP:
         if (mSipTcpServer)
            *pPort = mSipTcpServer->getServerPort();
         break;
#ifdef HAVE_SSL
      case SIP_TRANSPORT_TLS:
         if (mSipTlsServer)
            *pPort = mSipTlsServer->getServerPort();
         break;
#endif
      default:
         if (mSipUdpServer)
            *pPort = mSipUdpServer->getServerPort();
         break;
      }
   }

   return TRUE;
}


// Get the NAT mapped address and port
UtlBoolean SipUserAgent::getNatMappedAddress(UtlString* pIpAddress,
                                             int* pPort,
                                             SIP_TRANSPORT_TYPE protocol /*= SIP_TRANSPORT_UDP*/)
{
   UtlBoolean bRet(FALSE);

   if (mSipUdpServer)
   {
      bRet = mSipUdpServer->getStunAddress(pIpAddress, pPort);
   }
   else if (mSipTcpServer)
   {
      // TODO - a TCP server should also be able to return a stun address
      //bRet = mSipTcpServer->getStunAddress(pIpAddress, pPort);
   }
   return bRet;
}


void SipUserAgent::setIsUserAgent(UtlBoolean isUserAgent)
{
   mIsUaTransactionByDefault = isUserAgent;
}

/// Add either Server or User-Agent header, as appropriate
void SipUserAgent::setSelfHeader(SipMessage& message)
{
   if (mIsUaTransactionByDefault)
   {
      setUserAgentHeader(message);
   }
   else
   {
      setServerHeader(message);
   }
}


// setUserAgentHeaderProperty
//      provides a string to be appended to the standard User-Agent
//      header value between "<vendor>/<version>" and the platform (eg "(VxWorks)")
//      Value should be formated either as "token/token" or "(string)"
//      with no leading or trailing space.
void SipUserAgent::setUserAgentHeaderProperty( const char* property )
{
   if ( property )
   {
      mUserAgentHeaderProperties.append(" ");
      mUserAgentHeaderProperties.append( property );
   }
}


void SipUserAgent::setMaxForwards(int maxForwards)
{
   if(maxForwards > 0)
   {
      mMaxForwards = maxForwards;
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,"SipUserAgent::setMaxForwards maxForwards <= 0: %d",
         maxForwards);
   }
}

int SipUserAgent::getMaxForwards()
{
   int maxForwards;
   if(mMaxForwards <= 0)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,"SipUserAgent::getMaxForwards maxForwards <= 0: %d",
         mMaxForwards);

      maxForwards = SIP_DEFAULT_MAX_FORWARDS;
   }
   else
   {
      maxForwards = mMaxForwards;
   }

   return(maxForwards);
}

int SipUserAgent::getMaxSrvRecords() const
{
   return(mMaxSrvRecords);
}

void SipUserAgent::setMaxSrvRecords(int maxSrvRecords)
{
   mMaxSrvRecords = maxSrvRecords;
}

int SipUserAgent::getDnsSrvTimeout()
{
   return(mDnsSrvTimeout);
}

void SipUserAgent::setDnsSrvTimeout(int timeout)
{
   mDnsSrvTimeout = timeout;
}

void SipUserAgent::setForking(UtlBoolean enabled)
{
   mForkingEnabled = enabled;
}

void SipUserAgent::prepareContact(SipMessage& message,
                                  const UtlString& targetIpAddress,
                                  int targetPort)
{
   SIP_TRANSPORT_TYPE transport = SipTransport::getSipTransport(message.getSendProtocol());

   // Add a default contact if none is present
   // AND To all requests -- except REGISTERs (registering using default contact is nosence)
   //   OR all non-failure responses 
   int cseqNum = 0;
   UtlString cseqMethod;
   message.getCSeqField(&cseqNum , &cseqMethod);
   Url contactUri;
   if (!message.getContactUri(0, contactUri) &&
      ((!message.isResponse() && (cseqMethod.compareTo(SIP_REGISTER_METHOD, UtlString::ignoreCase) != 0))
      || (message.getResponseStatusCode() < SIP_MULTI_CHOICE_CODE)))
   {
      UtlString contactIp;
      int contactPort = PORT_NONE;
      UtlString userId;
      if (message.isResponse())
      {
         // reuse userId of to field
         Url toField;
         message.getToUrl(toField);
         toField.getUserId(userId);
      }
      else
      {
         // reuse userId of from field
         Url fromField;
         message.getFromUrl(fromField);
         fromField.getUserId(userId);
      }

      SipContactSelector contactSelector(*this);
      contactSelector.getBestContactUri(contactUri, userId,
         transport, message.getLocalIp(), targetIpAddress, targetPort);

      message.setContactField(contactUri.toString());
   }
   else
   {
      // if there is already a contact and update is allowed, try to find a better contact
      if (message.isContactOverrideAllowed() && message.getContactUri(0, contactUri))
      {
         UtlString contactIp;
         int contactPort = PORT_NONE;
         // reuse userId of from old contact
         UtlString userId;
         contactUri.getUserId(userId);

         SipContactSelector contactSelector(*this);
         contactSelector.getBestContactUri(contactUri, userId,
            transport, message.getLocalIp(), targetIpAddress, targetPort);

         message.setContactField(contactUri.toString());
      }
   }
}

void SipUserAgent::getAllowedMethods(UtlString* allowedMethods) const
{
   UtlDListIterator iterator(allowedSipMethods);
   allowedMethods->remove(0);
   UtlString* method;

   while ((method = (UtlString*) iterator()))
   {
      if(!method->isNull())
      {
         if(!allowedMethods->isNull())
         {
            allowedMethods->append(",");
         }
         allowedMethods->append(method->data());
      }
   }
}

void SipUserAgent::getDirectoryServer(int index, UtlString* address,
                                      int* port, UtlString* protocol)
{
   UtlString serverAddress;
   NameValueTokenizer::getSubField(directoryServers.data(), 0,
      SIP_MULTIFIELD_SEPARATOR, &serverAddress);

   address->remove(0);
   *port = PORT_NONE;
   protocol->remove(0);
   SipMessage::parseAddressFromUri(serverAddress.data(),
      address, port, protocol);
   serverAddress.remove(0);
}

UtlBoolean SipUserAgent::getProxyServer(const UtlString& proxyServers,
                                        int index,
                                        UtlString& address,
                                        int& port,
                                        UtlString& protocol)
{
   address.remove(0);
   port = PORT_NONE;
   protocol.remove(0);

   UtlString serverAddress;
   NameValueTokenizer::getSubField(proxyServers.data(), index, SIP_MULTIFIELD_SEPARATOR, &serverAddress);

   SipMessage::parseAddressFromUri(serverAddress.data(), &address, &port, &protocol);
   serverAddress.remove(0);

   return !address.isNull();
}

UtlBoolean SipUserAgent::getProxyServer(const SipMessage& sipMsg,
                                        int index,
                                        UtlString& address,
                                        int& port,
                                        UtlString& protocol)
{
   address.remove(0);
   port = PORT_NONE;
   protocol.remove(0);

   if (m_pLineProvider)
   {
      // try to get proxy servers for given message
      UtlString proxyServers;
      UtlBoolean found = m_pLineProvider->getProxyServersForMessage(sipMsg, proxyServers);
      if (found && !proxyServers.isNull())
      {
         // try to use found proxyServers
         return SipUserAgent::getProxyServer(proxyServers, index, address, port, protocol);
      }
      else
      {
         // try to use default proxy server
         return SipUserAgent::getProxyServer(m_defaultProxyServers, index, address, port, protocol);
      }
   }

   return FALSE;
}

void SipUserAgent::setDefaultProxyServers(const char* sipProxyServers)
{
   if (sipProxyServers)
   {
      m_defaultProxyServers = sipProxyServers ;
   }
   else
   {
      m_defaultProxyServers.remove(0) ;
   }
}

int SipUserAgent::getSipStateTransactionTimeout()
{
   return mTransactionStateTimeoutMs;
}

int SipUserAgent::getReliableTransportTimeout()
{
   return(mReliableTransportTimeoutMs);
}

int SipUserAgent::getFirstResendTimeout()
{
   return(mFirstResendTimeoutMs);
}

int SipUserAgent::getLastResendTimeout()
{
   return(mLastResendTimeoutMs);
}

int SipUserAgent::getDefaultExpiresSeconds() const
{
   return(mDefaultExpiresSeconds);
}

void SipUserAgent::setDefaultExpiresSeconds(int expiresSeconds)
{
   if(expiresSeconds > 0 &&
      expiresSeconds <= mMinInviteTransactionTimeout)
   {
      mDefaultExpiresSeconds = expiresSeconds;
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_ERR,
         "SipUserAgent::setDefaultExpiresSeconds "
         "illegal expiresSeconds value: %d IGNORED",
         expiresSeconds);
   }
}

int SipUserAgent::getDefaultSerialExpiresSeconds() const
{
   return(mDefaultSerialExpiresSeconds);
}

void SipUserAgent::setDefaultSerialExpiresSeconds(int expiresSeconds)
{
   if(expiresSeconds > 0 &&
      expiresSeconds <= mMinInviteTransactionTimeout)
   {
      mDefaultSerialExpiresSeconds = expiresSeconds;
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "SipUserAgent::setDefaultSerialExpiresSeconds "
         "illegal expiresSeconds value: %d IGNORED",
         expiresSeconds);
   }
}

void SipUserAgent::setMaxTcpSocketIdleTime(int idleTimeSeconds)
{
   if(mMinInviteTransactionTimeout < idleTimeSeconds)
   {
      mMaxTcpSocketIdleTime = idleTimeSeconds;
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "SipUserAgent::setMaxTcpSocketIdleTime "
         "idleTimeSeconds: %d less than mMinInviteTransactionTimeout: %d IGNORED",
         idleTimeSeconds, mMinInviteTransactionTimeout);
   }
}

void SipUserAgent::setHostAliases(UtlString& aliases)
{
   UtlString aliasString;
   int aliasIndex = 0;
   while(NameValueTokenizer::getSubField(aliases.data(), aliasIndex,
      ", \t", &aliasString))
   {
      Url aliasUrl(aliasString);
      UtlString hostAlias;
      aliasUrl.getHostAddress(hostAlias);
      int port = aliasUrl.getHostPort();

      if(!portIsValid(port))
      {
         hostAlias.append(":5060");
      }
      else
      {
         char portString[20];
         sprintf(portString, ":%d", port);
         hostAlias.append(portString);
      }

      UtlString* newAlias = new UtlString(hostAlias);
      mMyHostAliases.insert(newAlias);
      aliasIndex++;
   }
}

void SipUserAgent::printStatus()

{
   if(mSipUdpServer)
   {
      mSipUdpServer->printStatus();
   }
   if(mSipTcpServer)
   {
      mSipTcpServer->printStatus();
   }
#ifdef HAVE_SSL
   if(mSipTlsServer)
   {
      mSipTlsServer->printStatus();
   }
#endif

   UtlString txString;
   mSipTransactions.toString(txString);

   osPrintf("Transactions:\n%s\n", txString.data());
}

void SipUserAgent::startMessageLog(int newMaximumLogSize)
{
   if(newMaximumLogSize > 0) mMaxMessageLogSize = newMaximumLogSize;
   if(newMaximumLogSize == -1) mMaxMessageLogSize = -1;
   mMessageLogEnabled = TRUE;

   {
      OsWriteLock Writelock(mMessageLogWMutex);
      OsReadLock ReadLock(mMessageLogRMutex);
      if(mMaxMessageLogSize > 0)
         mMessageLog.capacity(mMaxMessageLogSize);
   }
}

void SipUserAgent::stopMessageLog()
{
   mMessageLogEnabled = FALSE;
}

void SipUserAgent::clearMessageLog()
{
   OsWriteLock Writelock(mMessageLogWMutex);
   OsReadLock Readlock(mMessageLogRMutex);
   mMessageLog.remove(0);
}

void SipUserAgent::logMessage(const char* message, int messageLength)
{
   if(mMessageLogEnabled)
   {
#ifdef TEST_PRINT
      osPrintf("SIP LOGGING ENABLED\n");
#endif
      {// lock scope
         OsWriteLock Writelock(mMessageLogWMutex);
         // Do not allow the log go grow beyond the maximum
         if(mMaxMessageLogSize > 0 &&
            ((((int)mMessageLog.length()) + messageLength) > mMaxMessageLogSize))
         {
            mMessageLog.remove(0,
               mMessageLog.length() + messageLength - mMaxMessageLogSize);
         }

         mMessageLog.append(message, messageLength);
      }//lock scope
   }
#ifdef TEST_PRINT
   else osPrintf("SIP LOGGING DISABLED\n");
#endif
}

void SipUserAgent::getMessageLog(UtlString& logData)
{
   OsReadLock Readlock(mMessageLogRMutex);
   logData = mMessageLog;
}

void SipUserAgent::allowExtension(const char* extension)
{
#ifdef TEST_PRINT
   osPrintf("Allowing extension: \"%s\"\n", extension);
#endif
   UtlString* extensionName = new UtlString(extension);
   allowedSipExtensions.append(extensionName);
}

void SipUserAgent::getSupportedExtensions(UtlString& extensionsString) const
{
   extensionsString.remove(0);
   UtlString* extensionName = NULL;
   UtlDListIterator iterator(allowedSipExtensions);
   while ((extensionName = (UtlString*) iterator()))
   {
      if(!extensionsString.isNull()) extensionsString.append(",");
      extensionsString.append(extensionName->data());
   }
}

void SipUserAgent::setLocationHeader(const char* szHeader)
{
   mLocationHeader = szHeader;
}

void SipUserAgent::setRecurseOnlyOne300Contact(UtlBoolean recurseOnlyOne)
{
   mRecurseOnlyOne300Contact = recurseOnlyOne;
}

SipMessage* SipUserAgent::getRequest(const SipMessage& response)
{
   // If the transaction exists and can be locked it
   // is returned.
   enum SipTransaction::messageRelationship relationship;
   SipTransaction* transaction =
      mSipTransactions.findTransactionFor(response,
      FALSE, // incoming
      relationship);
   SipMessage* request = NULL;

   if(transaction && transaction->getRequest())
   {
      // Make a copy to return
      request = new SipMessage(*(transaction->getRequest()));
   }

   // Need to unlock the transaction
   if(transaction)
      mSipTransactions.markAvailable(*transaction);

   return(request);
}

int SipUserAgent::getTcpPort() const
{
   int iPort = PORT_NONE ;

   if (mSipTcpServer)
   {
      iPort = mSipTcpServer->getServerPort() ;
   }

   return iPort ;
}

int SipUserAgent::getUdpPort() const
{
   int iPort = PORT_NONE ;

   if (mSipUdpServer)
   {
      iPort = mSipUdpServer->getServerPort() ;
   }

   return iPort ;
}

int SipUserAgent::getTlsPort() const
{
   int iPort = PORT_NONE ;

#ifdef HAVE_SSL
   if (mSipTlsServer)
   {
      iPort = mSipTlsServer->getServerPort() ;
   }
#endif

   return iPort ;
}


/* ============================ INQUIRY =================================== */

UtlBoolean SipUserAgent::isMethodAllowed(const char* method)
{
   UtlString methodName(method);
   UtlBoolean isAllowed = (allowedSipMethods.occurrencesOf(&methodName) > 0);

   if (!isAllowed)
   {
      /* The method was not explicitly requested, but check for whether the 
      * application has registered for the wildcard.  If so, the method is 
      * allowed, but we do not advertise that fact in the Allow header.*/
      UtlString wildcardMethod;

      OsReadLock lock(mObserverMutex);
      isAllowed = mMessageObservers.contains(&wildcardMethod);
   }

   return(isAllowed);
}

UtlBoolean SipUserAgent::isExtensionAllowed(const char* extension) const
{
#ifdef TEST_PRINT
   osPrintf("isExtensionAllowed extension: \"%s\"\n", extension);
#endif
   UtlString extensionString;
   if(extension) extensionString.append(extension);
   extensionString.toLower();
   UtlString extensionName(extensionString);
   extensionString.remove(0);
   return(allowedSipExtensions.occurrencesOf(&extensionName) > 0);
}

void SipUserAgent::whichExtensionsNotAllowed(const SipMessage* message,
                                             UtlString* disallowedExtensions) const
{
   int extensionIndex = 0;
   UtlString extension;

   disallowedExtensions->remove(0);
   while(message->getRequireExtension(extensionIndex, &extension))
   {
      if(!isExtensionAllowed(extension.data()))
      {
         if(!disallowedExtensions->isNull())
         {
            disallowedExtensions->append(SIP_MULTIFIELD_SEPARATOR);
            disallowedExtensions->append(SIP_SINGLE_SPACE);
         }
         disallowedExtensions->append(extension.data());
      }
      extensionIndex++;
   }
   extension.remove(0);
}

UtlBoolean SipUserAgent::isMessageLoggingEnabled()
{
   return(mMessageLogEnabled);
}

UtlBoolean SipUserAgent::isReady()
{
   return isStarted();
}

UtlBoolean SipUserAgent::waitUntilReady()
{
   // Lazy hack, should be a semaphore or event
   int count = 0;
   while(!isReady() && count < 5)
   {
      delay(500);
      count++;
   }

   return isReady() ;
}

UtlBoolean SipUserAgent::isForkingEnabled()
{
   return(mForkingEnabled);
}

UtlBoolean SipUserAgent::isMyHostAlias(Url& route) const
{
   UtlString hostAlias;
   route.getHostAddress(hostAlias);
   int port = route.getHostPort();

   if(port == PORT_NONE)
   {
      hostAlias.append(":5060");
   }
   else
   {
      char portString[20];
      SNPRINTF(portString, sizeof(portString), ":%d", port);
      hostAlias.append(portString);
   }

   UtlString aliasMatch(hostAlias);
   UtlContainable* found = mMyHostAliases.find(&aliasMatch);

   return(found != NULL);
}

UtlBoolean SipUserAgent::recurseOnlyOne300Contact()
{
   return(mRecurseOnlyOne300Contact);
}


UtlBoolean SipUserAgent::isOk(OsSocket::IpProtocolSocketType socketType)
{
   UtlBoolean retval = FALSE;
   switch(socketType)
   {
   case OsSocket::TCP :
      if (mSipTcpServer)
      {
         retval = mSipTcpServer->isOk();
      }
      break;
   case OsSocket::UDP :
      if (mSipUdpServer)
      {
         retval = mSipUdpServer->isOk();
      }
      break;
#ifdef HAVE_SSL
   case OsSocket::SSL_SOCKET :
      if (mSipTlsServer)
      {
         retval = mSipTlsServer->isOk();
      }
      break;
#endif
   default :
      OsSysLog::add(FAC_SIP, PRI_ERR, "SipUserAgent::isOK - invalid socket type %d",
         socketType);
      break;
   }

   return retval;
}

UtlBoolean SipUserAgent::isShutdownDone()
{
   return mbShutdownDone;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
UtlBoolean SipUserAgent::shouldAuthenticate(SipMessage* message) const
{
   UtlString method;
   message->getRequestMethod(&method);

   //SDUA - Do not authenticate if a CANCEL or an ACK req/res from other side
   UtlBoolean methodCompare = TRUE ;
   if (   strcmp(method.data(), SIP_ACK_METHOD) == 0
      || strcmp(method.data(), SIP_CANCEL_METHOD) == 0
      )
   {
      methodCompare = FALSE;
   }

   method.remove(0);
   return(   methodCompare
      && (   0 == mAuthenticationScheme.compareTo(HTTP_BASIC_AUTHENTICATION,
      UtlString::ignoreCase
      )
      || 0 == mAuthenticationScheme.compareTo(HTTP_DIGEST_AUTHENTICATION,
      UtlString::ignoreCase
      )
      )
      );
}

UtlBoolean SipUserAgent::authorized(SipMessage* request, const char* uri) const
{
   UtlBoolean allowed = FALSE;
   // Need to create a nonce database for nonce's created
   // for each message (or find the message for the previous
   // sequence number containing the authentication response
   // and nonce for this request)
   const char* nonce = "1234567890"; // :TBD: should be using nonce from the message

   if(mAuthenticationScheme.compareTo("") == 0)
   {
      allowed = TRUE;
   }

   else
   {
      UtlString user;
      UtlString password;

      // Get the user id
      request->getAuthorizationUser(&user);
      // Look up the password
      mpAuthenticationDb->get(user.data(), password);

#ifdef TEST_PRINT
      osPrintf("SipUserAgent::authorized user:%s password found:\"%s\"\n",
         user.data(), password.data());

#endif
      // If basic is set allow basic or digest
      if(mAuthenticationScheme.compareTo(HTTP_BASIC_AUTHENTICATION,
         UtlString::ignoreCase
         ) == 0
         )
      {
         allowed = request->verifyBasicAuthorization(user.data(),
            password.data());


         // Try Digest if basic failed
         if(! allowed)
         {
#ifdef TEST_PRINT
            osPrintf("SipUserAgent::authorized basic auth. failed\n");
#endif
            allowed = request->verifyMd5Authorization(user.data(),
               password.data(),
               nonce,
               mAuthenticationRealm.data(),
               uri);
         }
#ifdef TEST_PRINT
         else
         {
            osPrintf("SipUserAgent::authorized basic auth. passed\n");
         }
#endif
      }

      // If digest is set allow only digest
      else if(mAuthenticationScheme.compareTo(HTTP_DIGEST_AUTHENTICATION,
         UtlString::ignoreCase
         ) == 0
         )
      {
         allowed = request->verifyMd5Authorization(user.data(),
            password.data(),
            nonce,
            mAuthenticationRealm.data(),
            uri);
      }
      user.remove(0);
      password.remove(0);
   }

   return(allowed);
}

void SipUserAgent::addAuthentication(SipMessage* message) const
{
   message->setAuthenticationData(mAuthenticationScheme.data(),
      mAuthenticationRealm.data(),
      "1234567890",  // nonce
      "abcdefghij"); // opaque
}

UtlBoolean SipUserAgent::resendWithAuthorization(SipMessage* response,
                                                 SipMessage* request,
                                                 int* messageType,
                                                 int authorizationEntity)
{
   UtlBoolean requestResent =FALSE;
   int sequenceNum;
   UtlString method;
   response->getCSeqField(&sequenceNum, &method);

   // The transaction sends the ACK for error cases now
   //if(method.compareTo(SIP_INVITE_METHOD , UtlString::ignoreCase) == 0)
   //{
   // Need to send an ACK to finish transaction
   //      SipMessage ackMessage;
   //      ackMessage.setAckData(response, request);
   //      send(ackMessage);
   //}

   SipMessage* authorizedRequest = new SipMessage();

#ifdef TEST_PRINT
   osPrintf("**************************************\n");
   osPrintf("CREATING message in resendWithAuthorization @ address: %X\n",authorizedRequest);
   osPrintf("**************************************\n");
#endif

   if (m_pLineProvider && response && request && authorizedRequest &&
       buildAuthenticatedRequest(*response, *request, *authorizedRequest))
   {
#ifdef TEST_PRINT
      osPrintf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
      UtlString authBytes;
      int authBytesLen;
      authorizedRequest->getBytes(&authBytes, &authBytesLen);
      osPrintf("Auth. message:\n%s", authBytes.data());
      osPrintf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
#endif

      requestResent = send(*authorizedRequest);
      // Send the response back to the application
      // to notify it of the CSeq change for the response
      *messageType = SipMessageEvent::AUTHENTICATION_RETRY;
   }
#ifdef TEST
   else
   {
      osPrintf("Giving up on entity %d authorization, userId: \"%s\"\n",
         authorizationEntity, dbUserId.data());
      osPrintf("authorization failed previously sent: %d\n",
         request->getAuthorizationField(&authField, authorizationEntity));
   }
#endif

   delete authorizedRequest;

   return(requestResent);
}

void SipUserAgent::lookupSRVSipAddress(UtlString protocol, UtlString& sipAddress, int& port, UtlString& srcIp)
{
   OsSocket::IpProtocolSocketType transport = OsSocket::UNKNOWN;

   if (sipAddress.compareTo("127.0.0.1") != 0)
   {
      server_t *server_list;
      server_list = SipSrvLookup::servers(sipAddress.data(),
         "sip",
         transport,
         port,
         srcIp.data());

      // The returned value is a sorted array of server_t with last element having host=NULL.
      // The servers are arranged in order of decreasing preference.
      if ( !server_list )
      {
#ifdef TEST_PRINT
         osPrintf("The DNS server is not SRV capable; \nbind servers v8.0 and above are SRV capable\n");
#endif
      }
      else
      {
         // The result array contains the hostname,
         //   socket type, IP address and port (in network byte order)
         //   DNS preference and weight
         server_t toServerUdp;
         server_t toServerTcp;
         int i;

#ifdef TEST_PRINT
         osPrintf("\n   Pref   Wt   Type    Name(IP):Port\n");
         for (i=0; SipSrvLookup::isValidServerT(server_list[i]); i++)
         {
            UtlString name;
            UtlString ip;
            SipSrvLookup::getHostNameFromServerT(server_list[i],
               name);
            SipSrvLookup::getIpAddressFromServerT(server_list[i],
               ip);
            osPrintf( "%6d %5d %5d   %s(%s):%d\n",
               SipSrvLookup::getPreferenceFromServerT(server_list[i]),
               SipSrvLookup::getWeightFromServerT(server_list[i]),
               SipSrvLookup::getProtocolFromServerT(server_list[i]),
               name.data(),
               ip.data(),
               SipSrvLookup::getPortFromServerT(server_list[i]) );
         }
#endif

         for (i=0; server_list[i].isValidServerT(); i++)
         {
            if (server_list[i].getProtocolFromServerT() ==
               OsSocket::UDP)
            {
               if (! toServerUdp.isValidServerT())
               {
                  toServerUdp = server_list[i];
#ifdef TEST_PRINT
                  UtlString name;
                  SipSrvLookup::getHostNameFromServerT(toServerUdp,
                     name);
                  osPrintf("UDP server %s\n", name.data());
#endif
               }
            }
            else if (server_list[i].getProtocolFromServerT() ==
               OsSocket::TCP)
            {
               if (toServerTcp.isValidServerT())
               {
                  toServerTcp = server_list[i];
#ifdef TEST_PRINT
                  UtlString name;
                  SipSrvLookup::getHostNameFromServerT(toServerTcp,
                     name);
                  osPrintf("TCP server %s\n", name.data());
#endif
               }
            }
         }

         if (!protocol.compareTo("TCP") &&
            toServerTcp.isValidServerT())
         {
            int newPort = toServerTcp.getPortFromServerT();
            if (portIsValid(newPort))
            {
               toServerTcp.getIpAddressFromServerT(sipAddress);
               port = newPort;
            }
            OsSysLog::add(FAC_SIP, PRI_DEBUG,"SipUserAgent:: found TCP server %s port %d",
               sipAddress.data(), newPort
               );
         }
         else if (toServerUdp.isValidServerT())
         {
            int newPort = toServerUdp.getPortFromServerT();
            if (portIsValid(newPort))
            {
               toServerUdp.getIpAddressFromServerT(sipAddress);
               port = newPort;
            }
#ifdef TEST_PRINT
            osPrintf("found UDP server %s port %d/%d\n",
               sipAddress.data(), newPort,
               SipSrvLookup::getPortFromServerT(toServerUdp));
#endif
         }

         delete[] server_list;
      }
   }
}

void SipUserAgent::setServerHeader(SipMessage& message)
{
   UtlString existing;
   message.getServerField(&existing);

   if(existing.isNull())
   {
      UtlString headerValue;
      selfHeaderValue(headerValue);

      message.setServerField(headerValue.data());
   }
}

void SipUserAgent::setUserAgentHeader(SipMessage& message)
{
   UtlString uaName;
   message.getUserAgentField(&uaName);

   if(uaName.isNull())
   {
      selfHeaderValue(uaName);
      message.setUserAgentField(uaName.data());
   }
}

void SipUserAgent::selfHeaderValue(UtlString& self)
{
   self = defaultUserAgentName;

   if ( !mUserAgentHeaderProperties.isNull() )
   {
      self.append(mUserAgentHeaderProperties);
   }

   if (mbIncludePlatformInUserAgentName)
   {
      self.append(PLATFORM_UA_PARAM);
   }
}

void SipUserAgent::setIncludePlatformInUserAgentName( const UtlBoolean bInclude )
{
   mbIncludePlatformInUserAgentName = bInclude;
}

bool SipUserAgent::addContact(SipContact& sipContact)
{
   return mContactDb.addContact(sipContact);
}

void SipUserAgent::getContacts(UtlSList& contacts)
{
   mContactDb.getAll(contacts);
}

void SipUserAgent::setHeaderOptions(UtlBoolean bAllowHeader,
                                    UtlBoolean bDateHeader,
                                    UtlBoolean bShortNames,
                                    const UtlString& acceptLanguage,
                                    UtlBoolean bSupportedHeader)
{
   mbAllowHeader = bAllowHeader;
   mbDateHeader = bDateHeader;
   mbShortNames = bShortNames;
   mAcceptLanguage = acceptLanguage;
   mbSupportedHeader = bSupportedHeader;
}                          

void SipUserAgent::prepareVia(SipMessage& message,
                              UtlString& branchId, 
                              OsSocket::IpProtocolSocketType toProtocol,///< transport to use for sending SipMessage
                              const UtlString& targetAddress, 
                              int targetPort)
{
   UtlString viaAddress;
   int viaPort;
   UtlString viaProtocolString;
   SipMessage::convertProtocolEnumToString(toProtocol, viaProtocolString);

   SipContactSelector contactSelector(*this); // contact selector can also find out the best Via address
   contactSelector.getBestContactAddress(viaAddress, viaPort,
      SipTransport::getSipTransport(toProtocol),
      message.getLocalIp(), targetAddress, targetPort);

   // Add the via field data
   message.addVia(viaAddress.data(), viaPort, viaProtocolString,
      branchId.data(),
      (toProtocol == OsSocket::UDP) && getUseRport(),
      NULL);
   return;
}

UtlBoolean SipUserAgent::getCredentialForMessage(const SipMessage& sipResponse, // message with authentication request
                                                 const SipMessage& sipRequest, // original sip request
                                                 UtlString& userID,
                                                 UtlString& passMD5Token) const
{
   UtlBoolean credentialFound = FALSE;

   if (m_pLineProvider)
   {
      SipLineCredential sipLineCredential;
      credentialFound = m_pLineProvider->getCredentialForMessage(sipResponse, sipRequest, sipLineCredential);
      if (credentialFound)
      {
         UtlString scheme;
         UtlString realm;
         HttpMessage::HttpEndpointEnum authorizationEntity = 
            HttpMessage::getAuthorizationEntity(sipResponse.getResponseStatusCode());
         sipResponse.getAuthenticationData(&scheme, &realm, NULL, NULL, NULL, NULL, authorizationEntity, NULL);

         passMD5Token = sipLineCredential.getPasswordMD5Digest(realm);
         userID = sipLineCredential.getUserId();
      }
   }

   return credentialFound;
}

UtlBoolean SipUserAgent::buildAuthenticatedRequest(const SipMessage& response, // response with 401 or 407
                                                   const SipMessage& request, // original sip request
                                                   SipMessage& newAuthRequest)
{
   UtlBoolean createdResponse = FALSE;
   int sequenceNum; // cseq from response
   UtlString sipMethod; // method in cseq field
   UtlString callId; // sip call id from response
   HttpMessage::HttpEndpointEnum authorizationEntity = 
      HttpMessage::getAuthorizationEntity(response.getResponseStatusCode());
   Url fromUri;// fromUri from original request message, for logging

   // data from authentication request from sip response
   UtlString scheme;
   UtlString realm;
   UtlString nonce;
   UtlString opaque;
   UtlString algorithm;
   UtlString qop; 
   UtlString stale;

   response.getCSeqField(sequenceNum, sipMethod);
   response.getCallIdField(callId);
   response.getAuthenticationData(&scheme, &realm, &nonce, &opaque, &algorithm, &qop, authorizationEntity, &stale);
   request.getFromUrl(fromUri);

   UtlBoolean alreadyTriedOnce = FALSE; // if we already tried to send auth data once

   if(scheme.compareTo(HTTP_BASIC_AUTHENTICATION, UtlString::ignoreCase) == 0)
   {
      // Log error, basic authentication is not supported
      OsSysLog::add(FAC_AUTH, PRI_ERR, "SipUserAgent is unable to handle basic authentication:\nlineUri=%s\ncallid=%s\nmethod=%s\ncseq=%d\nrealm=%s",
         fromUri.toString().data(), callId.data(), sipMethod.data(), sequenceNum, realm.data());
      return FALSE;
   }
   else if(scheme.compareTo(HTTP_DIGEST_AUTHENTICATION, UtlString::ignoreCase) == 0)
   {
      OsSysLog::add(FAC_AUTH, PRI_DEBUG, "Received authentication request:\nlineId:%s\ncallid=%s\nscheme=%s\nmethod=%s\ncseq=%d\nrealm=%s",
         fromUri.toString().data(), callId.data(), scheme.data(), sipMethod.data(), sequenceNum, realm.data()) ;

      // if stale flag is TRUE, according to RFC2617 we may wish to retry so we do
      // According to RFC2617: "The server should only set stale to TRUE
      // if it receives a request for which the nonce is invalid but with a
      // valid digest for that nonce" - so there shouldn't be infinite auth loop
      if (stale.compareTo("TRUE", UtlString::ignoreCase) != 0)
      {
         // Check to see if we already tried to send the credentials
         // if stale=true is present, we always act as if we never tried to authenticate
         alreadyTriedOnce = request.hasDigestAuthorizationData(realm, authorizationEntity);
      }

      // find credential for message
      UtlString userID;
      UtlString passMD5Token;
      UtlBoolean credentialFound = getCredentialForMessage(response, request, userID, passMD5Token);

      if (credentialFound)
      {
         if (!alreadyTriedOnce)
         {
            OsSysLog::add(FAC_AUTH, PRI_DEBUG, "About to authenticate:\nlineId:%s\ncallid=%s\nscheme=%s\nmethod=%s\ncseq=%d\nrealm=%s",
               fromUri.toString().data(), callId.data(), scheme.data(), sipMethod.data(), sequenceNum, realm.data());

            SipUserAgent::buildAuthenticatedSipMessage(request, newAuthRequest, userID, passMD5Token, algorithm, realm,
               nonce, opaque, qop, authorizationEntity);

            return TRUE;
         }
         else
         {
            OsSysLog::add(FAC_AUTH, PRI_WARNING, "Repeated authentication request for:\nlineId:%s\ncallid=%s\nscheme=%s\nmethod=%s\ncseq=%d\nrealm=%s"
               "\nGiving up, probably wrong password was provided",
               fromUri.toString().data(), callId.data(), scheme.data(), sipMethod.data(), sequenceNum, realm.data());
            return FALSE;
         }
      }
      else
      {
         // credentials not found - either unknown realm, line or userid
         OsSysLog::add(FAC_AUTH, PRI_ERR, "No credentials found for:\nlineId:%s\ncallid=%s\nscheme=%s\nmethod=%s\ncseq=%d\nrealm=%s",
            fromUri.toString().data(), callId.data(), scheme.data(), sipMethod.data(), sequenceNum, realm.data());
         return FALSE;
      }
   }
   else
   {
      OsSysLog::add(FAC_AUTH, PRI_ERR, "Unknown authentication scheme:\nlineId:%s\ncallid=%s\nscheme=%s\nmethod=%s\ncseq=%d\nrealm=%s",
         fromUri.toString().data(), callId.data(), scheme.data(), sipMethod.data(), sequenceNum, realm.data());
      return FALSE;
   }

   return FALSE;
}

void SipUserAgent::getHttpBodyDigest(const SipMessage& sipMessage, UtlString& bodyDigest)
{
   // Get the digest of the body
   const HttpBody* body = sipMessage.getBody();
   const char* bodyString = "";

   if(body)
   {
      int len;
      body->getBytes(&bodyString, &len);
      if(bodyString == NULL)
         bodyString = "";
   }

   NetMd5Codec::encode(bodyString, bodyDigest);
}

void SipUserAgent::addSipMessageAuthentication(SipMessage& sipMessage,
                                               const UtlString& userID,
                                               const UtlString& passMD5Token,
                                               const UtlString& algorithm,
                                               const UtlString& realm,
                                               const UtlString& nonce,
                                               const UtlString& opaque,
                                               const UtlString& qop,
                                               int authorizationEntity)
{
   UtlString responseHash;
   UtlString requestUri;
   int nonceCount;
   UtlString sipMethod;

   sipMessage.getRequestUri(&requestUri);
   // :TBD: cheat and use the cseq instead of a real nonce-count
   sipMessage.getCSeqField(&nonceCount, &sipMethod);
   sipMessage.getRequestMethod(&sipMethod);
   nonceCount = (nonceCount + 1) / 2;

   // Use unique tokens which are constant for this
   // session to generate a cnonce
   UtlString cnonceSeed;
   Url fromUrl;
   UtlString fromTag;
   UtlString cnonce;
   sipMessage.getCallIdField(&cnonceSeed);
   sipMessage.getFromUrl(fromUrl);
   fromUrl.getFieldParameter("tag", fromTag);
   cnonceSeed.append(fromTag);
   cnonceSeed.append("blablacnonce"); // secret
   NetMd5Codec::encode(cnonceSeed, cnonce);

   UtlString bodyDigest;
   SipUserAgent::getHttpBodyDigest(sipMessage, bodyDigest);

   // Build the Digest hash response
   HttpMessage::buildMd5Digest(passMD5Token, algorithm, nonce, cnonce, nonceCount, qop,
      sipMethod, requestUri, bodyDigest, &responseHash);

   sipMessage.setDigestAuthorizationData(userID, realm, nonce, requestUri,
      responseHash, algorithm, cnonce, opaque, qop, nonceCount, authorizationEntity);
}

void SipUserAgent::buildAuthenticatedSipMessage(const SipMessage& sipMessageTemplate,
                                                SipMessage& sipMessage,
                                                const UtlString& userID,
                                                const UtlString& passMD5Token,
                                                const UtlString& algorithm,
                                                const UtlString& realm,
                                                const UtlString& nonce,
                                                const UtlString& opaque,
                                                const UtlString& qop,
                                                int authorizationEntity)
{
   // Construct a new request with authorization and send it
   // the Sticky DNS fields will be copied by the copy constructor
   sipMessage = sipMessageTemplate;
   // Reset the transport parameters
   sipMessage.resetTransport();
   // Get rid of the via as another will be added.
   sipMessage.removeLastVia();

   SipUserAgent::addSipMessageAuthentication(sipMessage, userID, passMD5Token, algorithm,
      realm, nonce, opaque, qop, authorizationEntity);

   // This is a new version of the message so increment the sequence number
   sipMessage.incrementCSeqNumber();

   // If the first hop of this message is strict routed,
   // we need to add the request URI host back in the route
   // field so that the send will work correctly
   //
   //  message is:
   //      METHOD something
   //      Route: xyz, abc
   //
   //  change to xyz:
   //      METHOD something
   //      Route: something, xyz, abc
   //
   //  which sends as:
   //      METHOD something
   //      Route: xyz, abc
   //
   // But if the first hop is loose routed:
   //
   //  message is:
   //      METHOD something
   //      Route: xyz;lr, abc
   //
   // leave URI and routes alone
   if (sipMessage.isClientMsgStrictRouted())
   {
      UtlString requestUri;
      sipMessage.getRequestUri(&requestUri);
      sipMessage.addRouteUri(requestUri);
   }
}

void SipUserAgent::addAgentCapabilities(SipMessage& sipMessage) const
{
   // add allow field to Refer and Invite request . It is
   // mandatory for refer method
   int seqNum;
   UtlString seqMethod;
   int responseStatusCode = 0;
   UtlBoolean bIsResponse = sipMessage.isResponse();
   
   if (bIsResponse)
   {
      responseStatusCode = sipMessage.getResponseStatusCode();
   }

   sipMessage.getCSeqField(seqNum, seqMethod);
   UtlString allowedMethodsSet;
   if (!sipMessage.getAllowField(allowedMethodsSet) && mbAllowHeader)
   {
      // only set Allow: for INVITE, SUBSCRIBE, OPTIONS
      if(seqMethod.compareTo(SIP_INVITE_METHOD) == 0 ||
         seqMethod.compareTo(SIP_SUBSCRIBE_METHOD) == 0 ||
         seqMethod.compareTo(SIP_OPTIONS_METHOD) == 0)
      {
         if (!bIsResponse ||
            (bIsResponse && responseStatusCode > SIP_1XX_CLASS_CODE && responseStatusCode < SIP_3XX_CLASS_CODE))
         {
            UtlString allowedMethods;
            getAllowedMethods(&allowedMethods);
            sipMessage.setAllowField(allowedMethods);
         }
      }
   }

   if(!sipMessage.getHeaderValue(0, SIP_SUPPORTED_FIELD) && mbSupportedHeader)
   {
      // only set Supported: for INVITE, SUBSCRIBE, OPTIONS
      if(seqMethod.compareTo(SIP_INVITE_METHOD) == 0 ||
         seqMethod.compareTo(SIP_SUBSCRIBE_METHOD) == 0 ||
         seqMethod.compareTo(SIP_OPTIONS_METHOD) == 0)
      {
         if (!bIsResponse ||
            (bIsResponse && responseStatusCode > SIP_1XX_CLASS_CODE && responseStatusCode < SIP_3XX_CLASS_CODE))
         {
            UtlString supportedExtensions;
            getSupportedExtensions(supportedExtensions);
            if (supportedExtensions.length() > 0)
            {
               sipMessage.setSupportedField(supportedExtensions);
            }
         }
      }
   }
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
