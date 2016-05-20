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

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else /* _WIN32 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif /* _WIN32 */

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "os/OsLock.h"
#include "os/OsTimerTask.h"
#include "os/OsNatAgentTask.h"
#include <os/OsSharedServerTaskMgr.h>
#include "net/SipLineMgr.h"
#include "net/SipRefreshMgr.h"
#include "net/SipPimClient.h"
#include "net/SipSubscribeServer.h"
#include "net/SipDialogMgr.h"
#include "sdp/SdpCodecList.h"
#include "cp/XCpCallManager.h"
#include "mi/CpMediaInterfaceFactoryFactory.h"
#include "tapi/SipXCore.h"
#include "tapi/sipXtapi.h"
#include "tapi/SipXHandleMap.h"
#include "tapi/SipXConfig.h"
#include "tapi/SipXEvents.h"
#include "tapi/SipXAudio.h"
#include "tapi/SipXLine.h"
#include "tapi/SipXCall.h"
#include "tapi/SipXConference.h"
#include "tapi/SipXEventDispatcher.h"
#include "tapi/SipXMessageObserver.h"
#include "tapi/SipXPublish.h"
#include "tapi/SipXSubscribe.h"
#include "tapi/SipXKeepaliveEventListener.h"
#include "tapi/SipXRtpRedirectEventListener.h"
#include "tapi/SipXConferenceEventListener.h"
#include "tapi/SipXLineEventListener.h"
#include "tapi/SipXCallEventListener.h"
#include "tapi/SipXInfoStatusEventListener.h"
#include "tapi/SipXInfoEventListener.h"
#include "tapi/SipXSecurityEventListener.h"
#include "tapi/SipXMediaEventListener.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern SipXHandleMap gSubHandleMap;
extern SipXHandleMap gPubHandleMap;
extern SipXHandleMap gLineHandleMap;
extern SipXHandleMap gCallHandleMap;
extern SipXHandleMap gConfHandleMap;

// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
OsMutex gSessionLock(OsMutex::Q_FIFO);
static int gSessions = 0;

// GLOBAL FUNCTIONS

/* ============================ FUNCTIONS ================================= */


SIPXTAPI_API void sipxLogEntryAdd(SIPX_LOG_LEVEL priority, 
                                  const char *format,
                                  ...)
{
   va_list ap;
   va_start(ap, format);

   int threadId;
   OsTask::getCurrentTaskId(threadId);
   OsSysLog::vadd("sipXtapi", threadId, FAC_SIPXTAPI, (OsSysLogPriority)priority, format, ap);

   va_end(ap);
}


void sipxIncSessionCount()
{
   OsLock lock(gSessionLock);
   ++gSessions;
}


void sipxDecSessionCount()
{
   OsLock lock(gSessionLock);
   --gSessions;
}


int sipxGetSessionCount()
{
   return gSessions;
}


SIPX_RESULT sipxFlushHandles()
{
   gCallHandleMap.destroyAll();
   gLineHandleMap.destroyAll();
   gConfHandleMap.destroyAll();
   gPubHandleMap.destroyAll();
   gSubHandleMap.destroyAll();

   return SIPX_RESULT_SUCCESS ;
}


SIPX_RESULT sipxCheckForHandleLeaks() 
{
   SIPX_RESULT rc = SIPX_RESULT_SUCCESS;

   if (gCallHandleMap.entries() != 0)
   {
      printf("\ngpCallHandleMap Leaks (%d):\n", 
         (int) gCallHandleMap.entries());
      gCallHandleMap.dump();
      rc = SIPX_RESULT_FAILURE;
   }

   if (gLineHandleMap.entries() != 0)
   {
      printf("\ngpLineHandleMap Leaks (%d):\n",
         (int) gLineHandleMap.entries());
      gLineHandleMap.dump();
      rc = SIPX_RESULT_FAILURE;
   }

   if (gConfHandleMap.entries() != 0)
   {
      printf("\ngpConfHandleMap Leaks (%d):\n",
         (int) gConfHandleMap.entries());
      gConfHandleMap.dump();
      rc = SIPX_RESULT_FAILURE;
   }

   return rc;
}


const char* sipxContactTypeToString(SIPX_CONTACT_TYPE type) 
{
   const char* szResult = "UNKNOWN";

   switch (type)
   {
   case CONTACT_LOCAL:
      szResult = MAKESTR(CONTACT_LOCAL);
      break;
   case CONTACT_NAT_MAPPED:
      szResult = MAKESTR(CONTACT_NAT_MAPPED);
      break;
   case CONTACT_RELAY:
      szResult = MAKESTR(CONTACT_RELAY);
      break;
   default:
      break;
   }

   return szResult;
}

SIPX_RESULT validateNetwork()
{
   SIPX_RESULT res = SIPX_RESULT_NETWORK_FAILURE;

#ifdef _WIN32
   // Validate Network status (need to be implemented on linux/macos)
   char* szAddresses[1];
   char* szAdapters[1];
   int numAdapters = 1;
   if (sipxConfigGetAllLocalNetworkIps(szAddresses, szAdapters, &numAdapters) == SIPX_RESULT_SUCCESS)
   {
      if (numAdapters > 0)
      {
         free((void*) szAddresses[0]);
         free((void*) szAdapters[0]);
         res = SIPX_RESULT_SUCCESS;
      }
      else
      {
         OsSysLog::add(FAC_SIPXTAPI, PRI_ERR, "No network interfaces found");
      }
   }
   else
   {
      OsSysLog::add(FAC_SIPXTAPI, PRI_ERR, "Unable to query for network interfaces");
   }
#else
#warning "Network availability check not implemented on non-WIN32"
   res = SIPX_RESULT_SUCCESS;
#endif

   return res;
}


SIPX_RESULT checkEvalExpiration()
{
   SIPX_RESULT res = SIPX_RESULT_SUCCESS;
#ifdef SIPXTAPI_EVAL_EXPIRATION
   OsDateTime expireDate(EVAL_EXPIRE_YEAR, EVAL_EXPIRE_MONTH, EVAL_EXPIRE_DAY, 23, 59, 59, 0);
   OsDateTime nowDate;
   OsTime expireTime;
   OsTime nowTime;

   OsDateTime::getCurTime(nowDate);
   expireDate.cvtToTimeSinceEpoch(expireTime);
   nowDate.cvtToTimeSinceEpoch(nowTime);

   if (nowTime > expireTime)
   {
      OsSysLog::add(FAC_SIPXTAPI, PRI_ERR, "sipXtapi has expired");
      res = SIPX_RESULT_EVAL_TIMEOUT;
   }
#endif
   return res;
}

/****************************************************************************
* Public Initialization Functions
***************************************************************************/

SIPXTAPI_API SIPX_RESULT sipxInitialize(SIPX_INST* phInst,
                                        const int udpPort,
                                        const int tcpPort,
                                        const int tlsPort,
                                        const int rtpPortStart,
                                        const int maxConnections,
                                        const char* szIdentity,
                                        const char* szBindToAddr,
                                        int bUseSequentialPorts)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxInitialize");

#ifdef ENABLE_LOGGING
   // Start up logger thread if logging is on
   if (initLogger() == OS_SUCCESS)
   {
      OsSysLog::setLoggingPriority(PRI_DEBUG);
      OsSysLog::setOutputFile(0, "sipXtapi.log");
   }
#endif

   int iActualTLSPort = tlsPort;
   // log sipXtapi version
   char cVersion[80];
   SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
   SIPX_RESULT res;
   sipxConfigGetVersion(cVersion, sizeof(cVersion));
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO, cVersion);

   // log settings
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxInitialize tcpPort=%d udpPort=%d tlsPort=%d rtpPortStart=%d"
      " maxConnections=%d identity=%s bindTo=%s sequentialPorts=%d",
      tcpPort, udpPort, iActualTLSPort, rtpPortStart, maxConnections,
      ((szIdentity != NULL) ? szIdentity : ""),
      ((szBindToAddr != NULL) ? szBindToAddr : ""),
      bUseSequentialPorts);

   // check whether we have at least 1 NIC
   if ((res = validateNetwork()) != SIPX_RESULT_SUCCESS)
   {
      return res;
   }

   // check for evaluation expiration
   if ((res = checkEvalExpiration()) != SIPX_RESULT_SUCCESS)
   {
      return res;
   }

   // log whether this is NSS version of sipxtapi
#ifdef HAVE_NSS
   OsSysLog::add(FAC_SIPXTAPI, PRI_ERR, "sipXtapi built with NSS support");
#else
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO, "sipXtapi built without NSS support");
#endif

   // create sipxtapi instance structure
   SIPX_INSTANCE_DATA* pInst = new SIPX_INSTANCE_DATA();

   // create event listeners
   pInst->pSharedTaskMgr = new OsSharedServerTaskMgr(1); // use 1 thread
   pInst->pSharedTaskMgr->start();
   pInst->pLineEventListener = new SipXLineEventListener(pInst);
   pInst->pSharedTaskMgr->manage(*pInst->pLineEventListener);
   pInst->pCallEventListener = new SipXCallEventListener(pInst);
   pInst->pSharedTaskMgr->manage(*pInst->pCallEventListener);
   pInst->pInfoStatusEventListener = new SipXInfoStatusEventListener(pInst);
   pInst->pSharedTaskMgr->manage(*pInst->pInfoStatusEventListener);
   pInst->pInfoEventListener = new SipXInfoEventListener(pInst);
   pInst->pSharedTaskMgr->manage(*pInst->pInfoEventListener);
   pInst->pSecurityEventListener = new SipXSecurityEventListener(pInst);
   pInst->pSharedTaskMgr->manage(*pInst->pSecurityEventListener);
   pInst->pMediaEventListener = new SipXMediaEventListener(pInst);
   pInst->pSharedTaskMgr->manage(*pInst->pMediaEventListener);
   pInst->pRtpRedirectEventListener = new SipXRtpRedirectEventListener(pInst);
   pInst->pSharedTaskMgr->manage(*pInst->pRtpRedirectEventListener);
   pInst->pConferenceEventListener = new SipXConferenceEventListener(pInst);
   pInst->pSharedTaskMgr->manage(*pInst->pConferenceEventListener);
   // create refresh manager
   pInst->pRefreshManager = new SipRefreshMgr(pInst->pLineEventListener);
   // create Line manager
   pInst->pLineManager = new SipLineMgr(pInst->pRefreshManager);
   pInst->pRefreshManager->setLineMgr(pInst->pLineManager);

   pInst->nCalls = 0;
   pInst->nLines = 0;
   pInst->nConferences = 0;

   Url defaultIdentity(szIdentity); // used to build default contact for MESSAGE, keepalive OPTIONS etc

   // set default bind address, 0.0.0.0 = all IP addresses
   if (szBindToAddr == NULL)
   {
      szBindToAddr = "0.0.0.0";
   }
   
   // Bind the SIP user agent to a port and start it up
   pInst->pSipUserAgent = new SipUserAgent(
      tcpPort,                    // sipTcpPort
      udpPort,                    // sipUdpPort
      iActualTLSPort,             // sipTlsPort
      szBindToAddr,               // bind IP Address
      defaultIdentity.getUserId().data(), // default user
      NULL,                       // sipProxyServers
      NULL,                       // sipDirectoryServers
      NULL,                       // authenticationScheme
      NULL,                       // authenicateRealm
      NULL,                       // authenticateDb
      NULL,                       // authorizeUserIds
      NULL,                       // authorizePasswords
      pInst->pLineManager,        // lineMgr
      SIP_DEFAULT_RTT,            // sipFirstResendTimeout
      TRUE,                       // defaultToUaTransactions
      -1,                         // readBufferSize
      OsServerTask::DEF_MAX_MSGS, // queueSize
      bUseSequentialPorts);       // bUseNextAvailablePort
   pInst->pSipUserAgent->allowMethod(SIP_INFO_METHOD);
   pInst->pSipUserAgent->allowMethod(SIP_PRACK_METHOD);
   pInst->pSipUserAgent->allowMethod(SIP_UPDATE_METHOD);
   pInst->pSipUserAgent->allowMethod(SIP_SUBSCRIBE_METHOD);
   pInst->pSipUserAgent->allowMethod(SIP_NOTIFY_METHOD);

   // set bind address on OsSocket
   UtlString defaultBindAddressString;
   int unused;
   pInst->pSipUserAgent->getLocalAddress(&defaultBindAddressString, &unused, SIP_TRANSPORT_UDP);
   unsigned long defaultBindAddress = inet_addr(defaultBindAddressString.data());
   OsSocket::setDefaultBindAddress(defaultBindAddress);
   // start SipUserAgent server task
   pInst->pSipUserAgent->start();

   // log bind address, ports
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO, "Default bind address %s, udpPort=%d, tcpPort=%d, tlsPort=%d",
      defaultBindAddressString.data(),
      pInst->pSipUserAgent->getUdpPort(),
      pInst->pSipUserAgent->getTcpPort(),
      pInst->pSipUserAgent->getTlsPort());

   // create and start SipPimClient
   pInst->pSipPimClient = new SipPimClient(*pInst->pSipUserAgent, defaultIdentity);
   pInst->pSipPimClient->start();
   pInst->pSipPimClient->setIncomingImTextHandler(&sipxFirePIMEvent, pInst);

   // Startup Line Manager  Refresh Manager
   pInst->pLineManager->start();
   pInst->pRefreshManager->setSipUserAgent(pInst->pSipUserAgent);
   pInst->pRefreshManager->startRefreshMgr();

   // Create and start up a SIP SUBSCRIBE server
   pInst->pSubscribeServer = 
      SipSubscribeServer::buildBasicServer(*pInst->pSipUserAgent);
   pInst->pSubscribeServer->start();

   // Create and start up a SIP SUBSCRIBE client
   pInst->pDialogManager = new SipDialogMgr();
   pInst->pSipRefreshManager = 
      new SipRefreshManager(*pInst->pSipUserAgent, *pInst->pDialogManager);
   pInst->pSipRefreshManager->start();
   pInst->pSubscribeClient = new SipSubscribeClient(*pInst->pSipUserAgent,
                                                    *pInst->pDialogManager, 
                                                    *pInst->pSipRefreshManager);
   pInst->pSubscribeClient->start();

   // Enable PCMU, PCMA, Tones/RFC2833 codecs
   pInst->pSelectedCodecList = new SdpCodecList();

   // Instantiate the call processing subsystem
   // create call manager
   pInst->pCallManager = new XCpCallManager(
      pInst->pCallEventListener,
      pInst->pInfoStatusEventListener,
      pInst->pInfoEventListener,
      pInst->pSecurityEventListener,
      pInst->pMediaEventListener,
      pInst->pRtpRedirectEventListener,
      pInst->pConferenceEventListener,
      *pInst->pSipUserAgent,
      *pInst->pSelectedCodecList,
      pInst->pLineManager,
      szBindToAddr,
      FALSE, // doNotDisturb
      FALSE, // bEnableICE
      FALSE, // bIsRequiredLineMatch
      rtpPortStart, // rtpPortStart
      rtpPortStart + (2 * maxConnections), // rtpPortEnd
      CP_MAXIMUM_RINGING_EXPIRE_SECONDS,
      maxConnections, // maxCalls - max calls before sending busy. -1 means unlimited
      *sipXmediaFactoryFactory(NULL));

   // Start up the call processing system
   pInst->pCallManager->start();

   pInst->pAvailableCodecList = new SdpCodecList();

   CpMediaInterfaceFactory* pInterfaceFactory = pInst->pCallManager->getMediaInterfaceFactory();
   if (!pInterfaceFactory)
   {
      OsSysLog::add(FAC_SIPXTAPI, PRI_ERR, "Unable to create global media interface");
   }
   else
   {
      pInterfaceFactory->buildAllCodecList(*pInst->pAvailableCodecList);
      if (szIdentity)
      {
         pInterfaceFactory->setRTCPName(szIdentity);
      }
   }

   // init codecs
   pInst->videoFormat = VIDEO_FORMAT_ANY;
   sipxConfigSelectAudioCodecByName(pInst, NULL); // select all audio codecs
   sipxConfigSelectVideoCodecByName(pInst, NULL); // select all video codecs

   initAudioDevices(*pInst);

   // Setup listener delegation
   SipXEventDispatcher::initDispatcher();

   *phInst = pInst;
   sipxIncSessionCount();

   // create the message observer
   pInst->pMessageObserver = new SipXMessageObserver(pInst);
   pInst->pMessageObserver->start();

   // Enable ICE by default (only makes sense with STUN, TURN or with mulitple nics 
   // (multiple nic support still needs work).
   //sipxConfigEnableIce(pInst);

   pInst->pSipUserAgent->setHeaderOptions(pInst->bAllowHeader, pInst->bDateHeader,
      pInst->bShortNames, pInst->szAcceptLanguage, pInst->bSupportedHeader);

   rc = SIPX_RESULT_SUCCESS;
   //  check for TLS initialization
#ifdef HAVE_SSL
   if (pInst->pSipUserAgent->getTlsServer() && iActualTLSPort > 0)
   {
      OsStatus initStatus = pInst->pSipUserAgent->getTlsServer()->getTlsInitCode();
      if (initStatus != OS_SUCCESS)
      {
         rc = SIPX_RESULT_FAILURE;
      }
   }
#endif

   pInst->pKeepaliveEventListener = new SipXKeepaliveEventListener(pInst);
   pInst->pSharedTaskMgr->manage(*pInst->pKeepaliveEventListener);

   return rc;
}


SIPXTAPI_API SIPX_RESULT sipxReInitialize(SIPX_INST* phInst,
                                          const int udpPort,
                                          const int tcpPort,
                                          const int tlsPort,
                                          const int rtpPortStart,
                                          const int maxConnections,
                                          const char* szIdentity,
                                          const char* szBindToAddr,
                                          int bUseSequentialPorts)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxReInitialize");

   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxReInitialize hInst=%p",
      *phInst);

   SIPX_RESULT rc = SIPX_RESULT_FAILURE;

   if (phInst)
   {
      SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, *phInst);

      // remember original filename, as it could be lost when uninitializing sipxtapi
      UtlString logfile;
      OsStatus logFileResult = OS_FAILED;
      SIPX_LOG_LEVEL logPriority;
      logFileResult = OsSysLog::getOutputFile(logfile);
      logPriority = (SIPX_LOG_LEVEL)OsSysLog::getLoggingPriority();

      if (pInst)
      {
         sipxSubscribeDestroyAll(*phInst);
         sipxPublisherDestroyAll(*phInst);
         sipxCallDestroyAll(*phInst);
         sipxConferenceDestroyAll(*phInst);

         // wait until all calls have been destroyed
         int nCalls = 0;
         pInst->lock.acquire();
         nCalls = pInst->nCalls;
         pInst->lock.release();

         while(nCalls > 0)
         {
            OsTask::delay(10);

            pInst->lock.acquire();
            nCalls = pInst->nCalls;
            pInst->lock.release();
         }

         sipxLineRemoveAll(*phInst);
         sipxUnInitialize(*phInst, true);

         if (logFileResult == OS_SUCCESS && logfile.length() > 0)
         {
            // if we can set previous logging configuration
            sipxConfigInitLogging(logfile.data(), logPriority);
         }
         else
         {
            // log file is not used, but at least set priority to original level
            sipxConfigSetLogLevel(logPriority);
         }
      }

      rc = sipxInitialize(phInst,
         udpPort,
         tcpPort,
         tlsPort, 
         rtpPortStart,
         maxConnections,
         szIdentity,
         szBindToAddr,
         bUseSequentialPorts);
   }

   return rc;
}


SIPXTAPI_API SIPX_RESULT sipxUnInitialize(SIPX_INST hInst,
                                          int bForceShutdown)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxUnInitialize");

   SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;

   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxUnInitialize hInst=%p",
      hInst);

   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst)
   {
      // Verify that all calls are torn down and that no lines
      // are present.

      int iAttempts = 0;
      int nCalls;
      int nConferences;
      int nLines;
      size_t nCallManagerCalls;
      UtlSList sessionCallIdList;

      // wait for 500ms until all calls, lines etc are dropped
      do
      {
         pInst->lock.acquire();
         nCalls = pInst->nCalls;
         nConferences = pInst->nConferences;
         nLines = pInst->nLines;
         pInst->lock.release();

         sessionCallIdList.destroyAll(); // empty the list
         sipxGetAllAbstractCallIds(hInst, sessionCallIdList);
         nCallManagerCalls = sessionCallIdList.entries();

         if ((nCalls != 0) || (nConferences != 0) || (nLines != 0) || (nCallManagerCalls != 0))
         {
            OsTask::delay(50);
            iAttempts++ ;

            OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING,
               "Busy SIPX_INST (Waiting) (%p) nCalls=%d, nLines=%d, nConferences=%d nCallManagerCalls=%d",
               hInst, nCalls, nLines, nConferences, nCallManagerCalls);
         }
         else
         {
            break;
         }
      }while (iAttempts < 10);
      sessionCallIdList.destroyAll();      

      if (bForceShutdown || (nCalls == 0) && (nConferences == 0) && 
         (nLines == 0) && (nCallManagerCalls == 0))
      {
         // First: Shutdown user agent to avoid processing during teardown
         pInst->pSipUserAgent->shutdown(TRUE);
         pInst->pSipUserAgent->requestShutdown();
         pInst->pSipPimClient->requestShutdown();

         // get rid of pointer to the line manager in the refresh manager
         pInst->pRefreshManager->setLineMgr(NULL);
         pInst->pLineManager->requestShutdown();
         pInst->pCallManager->requestShutdown();
         pInst->pRefreshManager->requestShutdown();
         pInst->pSubscribeClient->requestShutdown();
         pInst->pSubscribeServer->requestShutdown();
         pInst->pSipRefreshManager->requestShutdown();
         pInst->pMessageObserver->requestShutdown();
         pInst->pSelectedCodecList->clearCodecs();

         delete pInst->pCallManager;
         pInst->pCallManager = NULL;
         delete pInst->pSipPimClient;
         pInst->pSipPimClient = NULL;
         delete pInst->pSubscribeClient;
         pInst->pSubscribeClient = NULL;
         delete pInst->pSubscribeServer;
         pInst->pSubscribeServer = NULL;
         delete pInst->pRefreshManager;
         pInst->pRefreshManager = NULL;
         delete pInst->pSipRefreshManager;
         pInst->pSipRefreshManager = NULL;
         delete pInst->pDialogManager;
         pInst->pDialogManager = NULL;
         delete pInst->pSipUserAgent;
         pInst->pSipUserAgent = NULL;
         delete pInst->pLineManager;
         pInst->pLineManager = NULL;
         delete pInst->pSelectedCodecList;
         pInst->pSelectedCodecList = NULL;
         delete pInst->pAvailableCodecList;
         pInst->pAvailableCodecList = NULL;
         // release all shared server tasks
         pInst->pSharedTaskMgr->release(*pInst->pLineEventListener);
         pInst->pSharedTaskMgr->release(*pInst->pCallEventListener);
         pInst->pSharedTaskMgr->release(*pInst->pInfoStatusEventListener);
         pInst->pSharedTaskMgr->release(*pInst->pInfoEventListener);
         pInst->pSharedTaskMgr->release(*pInst->pSecurityEventListener);
         pInst->pSharedTaskMgr->release(*pInst->pMediaEventListener);
         pInst->pSharedTaskMgr->release(*pInst->pKeepaliveEventListener);
         pInst->pSharedTaskMgr->release(*pInst->pRtpRedirectEventListener);
         pInst->pSharedTaskMgr->release(*pInst->pConferenceEventListener);
         pInst->pSharedTaskMgr->shutdown();
         pInst->pSharedTaskMgr->flushMessages();
         // get rid of listeners
         delete pInst->pLineEventListener;
         pInst->pLineEventListener = NULL;
         delete pInst->pCallEventListener;
         pInst->pCallEventListener = NULL;
         delete pInst->pInfoStatusEventListener;
         pInst->pInfoStatusEventListener = NULL;
         delete pInst->pInfoEventListener;
         pInst->pInfoEventListener = NULL;
         delete pInst->pSecurityEventListener;
         pInst->pSecurityEventListener = NULL;
         delete pInst->pMediaEventListener;
         pInst->pMediaEventListener = NULL;
         delete pInst->pRtpRedirectEventListener;
         pInst->pRtpRedirectEventListener = NULL;
         delete pInst->pConferenceEventListener;
         pInst->pConferenceEventListener = NULL;
         delete pInst->pMessageObserver;
         pInst->pMessageObserver = NULL;
         delete pInst->pKeepaliveEventListener;
         pInst->pKeepaliveEventListener = NULL;
#ifdef _WIN32
         Sleep(50); // give it time to dispatch all events
         // this will be removed
#endif
         // remove listeners for this instance
         SipXEventDispatcher::removeAllListeners(hInst);
         // shutdown event dispatcher, it has it's own counter
         SipXEventDispatcher::shutdownDispatcher();

         sipxDestroyMediaFactoryFactory();

         sipxDecSessionCount();
         if (sipxGetSessionCount() == 0)
         {
            // Destroy the timer task to flush timers
            OsTimerTask::destroyTimerTask();
            OsNatAgentTask::releaseInstance();
            OsSysLog::shutdown();
         }

         // free audio devices strings
         freeAudioDevices(*pInst);

         // delete sipxtapi instance
         delete pInst;
         pInst = NULL;

         rc = SIPX_RESULT_SUCCESS;
      }
      else
      {
         OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
            "Unable to shutdown busy SIPX_INST (%p) nCalls=%d, nLines=%d, nConferences=%d",
            hInst, nCalls, nLines, nConferences);
         rc = SIPX_RESULT_BUSY;
      }
   }

   return rc;
}

SIPX_CONTACT_ADDRESS getSipxContact(const SipContact& sipContact)
{
   SIPX_CONTACT_ADDRESS sipxContact;
   sipxContact.id = sipContact.getContactId();
   sipxContact.eContactType = (SIPX_CONTACT_TYPE)sipContact.getContactType();
   sipxContact.eTransportType = (SIPX_TRANSPORT_TYPE)sipContact.getTransportType();
   SAFE_STRNCPY(sipxContact.cInterface, sipContact.getAdapterName().data(), MAX_ADAPTER_NAME_LENGTH + 4);
   SAFE_STRNCPY(sipxContact.cInterfaceIp, sipContact.getAdapterIp().data(), 28);
   SAFE_STRNCPY(sipxContact.cIpAddress, sipContact.getIpAddress().data(), 28);
   sipxContact.iPort = sipContact.getPort();
   return sipxContact;
}
