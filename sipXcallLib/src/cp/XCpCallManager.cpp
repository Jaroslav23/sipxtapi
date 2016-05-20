//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsLock.h>
#include <os/OsPtrLock.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/UtlSList.h>
#include <utl/UtlInt.h>
#include <net/SipUserAgent.h>
#include <mi/CpMediaInterfaceFactory.h>
#include <cp/XCpCallManager.h>
#include <cp/XCpAbstractCall.h>
#include <cp/XCpCall.h>
#include <cp/XCpConference.h>
#include <cp/XCpCallIdUtil.h>
#include <cp/CpMessageTypes.h>
#include <cp/msg/AcCommandMsg.h>
#include <cp/msg/ScCommandMsg.h>
#include <cp/msg/CmCommandMsg.h>
#include <cp/msg/CmGainFocusMsg.h>
#include <cp/msg/CmYieldFocusMsg.h>
#include <cp/msg/CmDestroyAbstractCallMsg.h>
#include <cp/msg/ScNotificationMsg.h>
#include <cp/msg/CpTimerMsg.h>
#include <cp/msg/ScTimerMsg.h>
#include <net/SipDialog.h>
#include <net/SipMessageEvent.h>
#include <net/SipLineProvider.h>

// DEFINES
#define MIN_SESSION_TIMER_EXPIRATION 90
#define DEFAULT_SESSION_TIMER_EXPIRATION 1800

//#define PRINT_SIP_MESSAGE
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const char* sipCallIdPrefix = "s"; // prefix of sip call-id sent in sip messages.

// STATIC VARIABLE INITIALIZATIONS
const int XCpCallManager::CALLMANAGER_MAX_REQUEST_MSGS = 2000;

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

XCpCallManager::XCpCallManager(CpCallStateEventListener* pCallEventListener,
                               SipInfoStatusEventListener* pInfoStatusEventListener,
                               SipInfoEventListener* pInfoEventListener,
                               SipSecurityEventListener* pSecurityEventListener,
                               CpMediaEventListener* pMediaEventListener,
                               CpRtpRedirectEventListener* pRtpRedirectEventListener,
							   CpConferenceEventListener* pConferenceEventListener,
                               SipUserAgent& rSipUserAgent,
                               const SdpCodecList& rSdpCodecList,
                               SipLineProvider* pSipLineProvider,
                               const UtlString& sBindIpAddress,
                               UtlBoolean bDoNotDisturb,
                               UtlBoolean bEnableICE,
                               UtlBoolean bIsRequiredLineMatch,
                               int rtpPortStart,
                               int rtpPortEnd,
                               int inviteExpiresSeconds,
                               int maxCalls,
                               CpMediaInterfaceFactory& rMediaInterfaceFactory)
: OsServerTask("XCallManager-%d", NULL, CALLMANAGER_MAX_REQUEST_MSGS)
, m_pCallEventListener(pCallEventListener)
, m_pInfoStatusEventListener(pInfoStatusEventListener)
, m_pInfoEventListener(pInfoEventListener)
, m_pSecurityEventListener(pSecurityEventListener)
, m_pMediaEventListener(pMediaEventListener)
, m_pRtpRedirectEventListener(pRtpRedirectEventListener)
, m_pConferenceEventListener(pConferenceEventListener)
, m_rSipUserAgent(rSipUserAgent)
, m_rDefaultSdpCodecList(rSdpCodecList)
, m_pSipLineProvider(pSipLineProvider)
, m_sipCallIdGenerator(sipCallIdPrefix)
, m_bDoNotDisturb(bDoNotDisturb)
, m_bIsRequiredLineMatch(bIsRequiredLineMatch)
, m_rtpPortStart(rtpPortStart)
, m_rtpPortEnd(rtpPortEnd)
, m_memberMutex(OsMutex::Q_FIFO)
, m_maxCalls(maxCalls)
, m_rMediaInterfaceFactory(rMediaInterfaceFactory)
, m_sBindIpAddress(sBindIpAddress)
, m_sessionTimerExpiration(DEFAULT_SESSION_TIMER_EXPIRATION)
, m_sessionTimerRefresh(CP_SESSION_REFRESH_AUTO)
, m_updateSetting(CP_SIP_UPDATE_ONLY_INBOUND)
, m_100relSetting(CP_100REL_PREFER_RELIABLE)
, m_sdpOfferingMode(CP_SDP_OFFERING_IMMEDIATE)
, m_inviteExpiresSeconds(inviteExpiresSeconds)
{
   startSipMessageObserving();

   m_natTraversalConfig.m_bEnableICE = bEnableICE;

   // Allow the "replaces" extension, because CallManager
   // implements the INVITE-with-Replaces logic.
   m_rSipUserAgent.allowExtension(SIP_REPLACES_EXTENSION);
   m_rSipUserAgent.allowExtension(SIP_SESSION_TIMER_EXTENSION);
   m_rSipUserAgent.allowExtension(SIP_PRACK_EXTENSION);
   m_rSipUserAgent.allowExtension(SIP_FROM_CHANGE_EXTENSION);
   m_rSipUserAgent.allowExtension(SIP_NO_REFER_SUB_EXTENSION);

   m_rMediaInterfaceFactory.setRtpPortRange(m_rtpPortStart, m_rtpPortEnd);
}

XCpCallManager::~XCpCallManager()
{
   stopSipMessageObserving();
   waitUntilShutDown();
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean XCpCallManager::handleMessage(OsMsg& rRawMsg)
{
   UtlBoolean bResult = FALSE;
   int msgType = rRawMsg.getMsgType();
   int msgSubType = rRawMsg.getMsgSubType();

   switch (msgType)
   {
   case OsMsg::PHONE_APP:
      return handlePhoneAppMessage(rRawMsg);
   case CpMessageTypes::CM_COMMAND:
      return handleCallManagerCommandMessage((const CmCommandMsg&)rRawMsg);
   case CpMessageTypes::SC_COMMAND:
      return handleSipConnectionCommandMessage((const ScCommandMsg&)rRawMsg);
   case CpMessageTypes::SC_NOFITICATION:
      return handleSipConnectionNotificationMessage((const ScNotificationMsg&)rRawMsg);
   case OsMsg::OS_TIMER_MSG:
      return handleTimerMessage(rRawMsg);
   case OsMsg::OS_EVENT: // timer event
   default:
      {
         OsSysLog::add(FAC_CP, PRI_ERR, "Unknown TYPE %d of XCpCallManager message subtype: %d\n", msgType, msgSubType);
         bResult = TRUE;
         break;
      }
   }

   return bResult;
}

void XCpCallManager::requestShutdown(void)
{
   m_callStack.shutdownAllAbstractCallThreads();

   OsServerTask::requestShutdown();
}

OsStatus XCpCallManager::createCall(UtlString& sCallId)
{
   OsStatus result = OS_FAILED;

   if (sCallId.isNull())
   {
      sCallId = getNewCallId();
   }
   // always allow creation of new call, check for limit only when establishing

   XCpCall *pCall = new XCpCall(sCallId, m_rSipUserAgent, *this, m_pSipLineProvider, m_rMediaInterfaceFactory, m_rDefaultSdpCodecList, *getMessageQueue(),
      m_natTraversalConfig, m_sBindIpAddress, m_sessionTimerExpiration, m_sessionTimerRefresh, m_updateSetting, m_100relSetting, m_sdpOfferingMode,
      m_inviteExpiresSeconds, &m_callStack, m_pCallEventListener, m_pInfoStatusEventListener,
      m_pInfoEventListener, m_pSecurityEventListener, m_pMediaEventListener, m_pRtpRedirectEventListener);

   UtlBoolean resStart = pCall->start();
   if (resStart)
   {
      UtlBoolean resPush = m_callStack.push(*pCall);
      if (resPush)
      {
         result = OS_SUCCESS;
      }
      else
      {
         delete pCall; // also shuts down thread
         pCall = NULL;
      }
   }

   return result;
}

OsStatus XCpCallManager::createConference(UtlString& sConferenceId, const UtlString& conferenceUri)
{
   OsStatus result = OS_FAILED;

   // always allow creation of new conference, check for limit only when establishing
   if (sConferenceId.isNull())
   {
      sConferenceId = getNewConferenceId();
   }
   XCpConference *pConference = new XCpConference(sConferenceId, conferenceUri, m_rSipUserAgent, *this, m_callStack,
      m_pSipLineProvider, m_rMediaInterfaceFactory, m_rDefaultSdpCodecList,
      *getMessageQueue(), m_natTraversalConfig, m_sBindIpAddress, m_sessionTimerExpiration, m_sessionTimerRefresh,
      m_updateSetting, m_100relSetting, m_sdpOfferingMode, m_inviteExpiresSeconds, &m_callStack, m_pCallEventListener,
      m_pInfoStatusEventListener, m_pInfoEventListener, m_pSecurityEventListener, m_pMediaEventListener, m_pRtpRedirectEventListener,
	  m_pConferenceEventListener);

   UtlBoolean resStart = pConference->start();
   if (resStart)
   {
      UtlBoolean resPush = m_callStack.push(*pConference);
      if (resPush)
      {
         result = OS_SUCCESS;
      }
      else
      {
         delete pConference; // also shuts down thread
         pConference = NULL;
      }
   }

   return result;
}

OsStatus XCpCallManager::connectCall(const UtlString& sCallId,
                                     SipDialog& sSipDialog,
                                     const UtlString& toAddress,
                                     const UtlString& fullLineUrl,
                                     const UtlString& sSipCallId,
                                     const UtlString& locationHeader,
                                     CP_CONTACT_ID contactId,
                                     SIP_TRANSPORT_TYPE transport,
                                     CP_FOCUS_CONFIG focusConfig,
                                     const UtlString& replacesField,
                                     CP_CALLSTATE_CAUSE callstateCause,
                                     const SipDialog* pCallbackSipDialog)
{
   OsStatus result = OS_NOT_FOUND;

   if (!isAddressValid(toAddress))
   {
      return OS_INVALID_ARGUMENT;
   }

   sSipDialog = SipDialog();

   OsPtrLock<XCpCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findCall(sCallId, ptrLock);
   if (resFind)
   {
      UtlString sTmpSipCallId = sSipCallId;
      if (sTmpSipCallId.isNull())
      {
         sTmpSipCallId = getNewSipCallId();
      }
      // we found call and have a lock on it
      return ptrLock->connect(sTmpSipCallId, sSipDialog, toAddress, fullLineUrl, locationHeader,
         contactId, transport, focusConfig,
         replacesField, callstateCause, pCallbackSipDialog);
   }

   return result;
}

OsStatus XCpCallManager::connectConferenceCall(const UtlString& sConferenceId,
                                               SipDialog& sSipDialog,
                                               const UtlString& toAddress,
                                               const UtlString& fullLineUrl,
                                               const UtlString& sSipCallId,
                                               const UtlString& locationHeader,
                                               CP_CONTACT_ID contactId,
                                               SIP_TRANSPORT_TYPE transport,
                                               CP_FOCUS_CONFIG focusConfig)
{
   OsStatus result = OS_NOT_FOUND;

   if (!isAddressValid(toAddress))
   {
      return OS_INVALID_ARGUMENT;
   }

   sSipDialog = SipDialog();

   OsPtrLock<XCpConference> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findConference(sConferenceId, ptrLock);
   if (resFind)
   {
      UtlString sTmpSipCallId = sSipCallId;
      if (sTmpSipCallId.isNull())
      {
         sTmpSipCallId = getNewSipCallId();
      }
      // we found call and have a lock on it
      return ptrLock->connect(sTmpSipCallId, sSipDialog, toAddress, fullLineUrl, locationHeader,
         contactId, transport, focusConfig,
         NULL, CP_CALLSTATE_CAUSE_CONFERENCE);
   }

   return result;
}

OsStatus XCpCallManager::conferenceJoin(const UtlString& sConferenceId,
                                        const SipDialog& sipDialog)
{
   OsStatus result = OS_FAILED;

   // check that call exists
   {
      OsPtrLock<XCpCall> ptrLock; // auto pointer lock
      UtlBoolean resFind = m_callStack.findCall(sipDialog, ptrLock);
      if (!resFind)
      {
         result = OS_NOT_FOUND;
         return result;
      }
   }

   // find conference and execute join on it
   {
      OsPtrLock<XCpConference> ptrLock; // auto pointer lock
      UtlBoolean resFind = m_callStack.findConference(sConferenceId, ptrLock);
      if (resFind)
      {
         return ptrLock->join(sipDialog);
      }
   }

   return result;
}

OsStatus XCpCallManager::conferenceSplit(const UtlString& sConferenceId,
                                         const SipDialog& sipDialog,
                                         UtlString& sNewCallId)
{
   OsStatus result = OS_FAILED;
   sNewCallId.clear();

   if (sipDialog.isEstablishedDialog())
   {
      // create new call for sip connection
      OsStatus createResult = createCall(sNewCallId);
      if (createResult == OS_SUCCESS)
      {
         // find conference and execute split on it
         {
            OsPtrLock<XCpConference> ptrLock; // auto pointer lock
            UtlBoolean resFind = m_callStack.findConference(sConferenceId, ptrLock);
            if (resFind)
            {
               result = ptrLock->split(sipDialog, sNewCallId);
               if (result == OS_SUCCESS)
               {
                  return result;
               }
               else
               {
                  destroyCall(sNewCallId);
               }
            }
         }
      }
      else
      {
         result = createResult;
      }
   }

   return result;
}

OsStatus XCpCallManager::startCallRedirectRtp(const UtlString& sSrcCallId,
                                              const UtlString& sDstCallId,
                                              const SipDialog& dstSipDialog)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findCall(sSrcCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->startCallRedirectRtp(sDstCallId, dstSipDialog);
   }

   return result;
}

OsStatus XCpCallManager::stopCallRedirectRtp(const UtlString& sCallId)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findCall(sCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->stopCallRedirectRtp();
   }

   return result;
}

OsStatus XCpCallManager::acceptAbstractCallConnection(const UtlString& sAbstractCallId,
                                                      const SipDialog& sSipDialog,
                                                      UtlBoolean bSendSDP,
                                                      const UtlString& locationHeader,
                                                      CP_CONTACT_ID contactId,
                                                      SIP_TRANSPORT_TYPE transport)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->acceptConnection(sSipDialog, bSendSDP, locationHeader, contactId, transport);
   }

   return result;
}

OsStatus XCpCallManager::rejectAbstractCallConnection(const UtlString& sAbstractCallId,
                                                      const SipDialog& sSipDialog)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->rejectConnection(sSipDialog);
   }

   return result;
}

OsStatus XCpCallManager::redirectAbstractCallConnection(const UtlString& sAbstractCallId,
                                                        const SipDialog& sSipDialog,
                                                        const UtlString& sRedirectSipUrl)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->redirectConnection(sSipDialog, sRedirectSipUrl);
   }

   return result;
}

OsStatus XCpCallManager::answerAbstractCallConnection(const UtlString& sAbstractCallId,
                                                      const SipDialog& sSipDialog)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->answerConnection(sSipDialog);
   }

   return result;
}

OsStatus XCpCallManager::acceptAbstractCallTransfer(const UtlString& sAbstractCallId,
                                                    const SipDialog& sSipDialog)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->acceptConnectionTransfer(sSipDialog);
   }

   return result;
}

OsStatus XCpCallManager::rejectAbstractCallTransfer(const UtlString& sAbstractCallId,
                                                    const SipDialog& sSipDialog)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->rejectConnectionTransfer(sSipDialog);
   }

   return result;
}

OsStatus XCpCallManager::dropAbstractCallConnection(const UtlString& sAbstractCallId,
                                                    const SipDialog& sSipDialog)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->dropConnection(sSipDialog);
   }

   return result;
}

OsStatus XCpCallManager::dropAbstractCallConnection(const SipDialog& sSipDialog)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sSipDialog, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->dropConnection(sSipDialog);
   }

   return result;
}

OsStatus XCpCallManager::dropAllAbstractCallConnections(const UtlString& sAbstractCallId)
{
   if (XCpCallIdUtil::isCallId(sAbstractCallId))
   {
      // call has only 1 connection
      return dropCallConnection(sAbstractCallId);
   }
   else
   {
      // conference has many connections
      return dropAllConferenceConnections(sAbstractCallId);
   }
}

OsStatus XCpCallManager::dropCallConnection(const UtlString& sCallId)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findCall(sCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->dropConnection();
   }

   return result;
}

OsStatus XCpCallManager::dropConferenceConnection(const UtlString& sConferenceId, const SipDialog& sSipDialog)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpConference> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findConference(sConferenceId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->dropConnection(sSipDialog);
   }

   return result;
}

OsStatus XCpCallManager::dropAllConferenceConnections(const UtlString& sConferenceId)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpConference> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findConference(sConferenceId, ptrLock);
   if (resFind)
   {
      // we found conference and have a lock on it
      return ptrLock->dropAllConnections();
   }

   return result;
}

OsStatus XCpCallManager::dropAbstractCall(const UtlString& sAbstractCallId)
{
   if (XCpCallIdUtil::isCallId(sAbstractCallId))
   {
      return dropCall(sAbstractCallId);
   }
   else
   {
      return dropConference(sAbstractCallId);
   }
}

OsStatus XCpCallManager::dropCall(const UtlString& sCallId)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findCall(sCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->dropConnection();
   }

   return result;
}

OsStatus XCpCallManager::dropConference(const UtlString& sConferenceId)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpConference> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findConference(sConferenceId, ptrLock);
   if (resFind)
   {
      // we found conference and have a lock on it
      return ptrLock->dropAllConnections(TRUE);
   }

   return result;
}

OsStatus XCpCallManager::destroyAbstractCall(const UtlString& sAbstractCallId)
{
   OsStatus result = OS_NOT_FOUND;

   // this deletes it safely, shutting down thread and media resources
   UtlBoolean resDelete = m_callStack.deleteAbstractCall(sAbstractCallId);
   if (resDelete)
   {
      result = OS_SUCCESS;
   }

   return result;
}

OsStatus XCpCallManager::destroyCall(const UtlString& sCallId)
{
   OsStatus result = OS_NOT_FOUND;

   // this deletes it safely, shutting down thread and media resources
   UtlBoolean resDelete = m_callStack.deleteCall(sCallId);
   if (resDelete)
   {
      result = OS_SUCCESS;
   }

   return result;
}

OsStatus XCpCallManager::destroyConference(const UtlString& sConferenceId)
{
   OsStatus result = OS_NOT_FOUND;

   // this deletes it safely, shutting down thread and media resources
   UtlBoolean resDelete = m_callStack.deleteConference(sConferenceId);
   if (resDelete)
   {
      result = OS_SUCCESS;
   }

   return result;
}

OsStatus XCpCallManager::transferBlindAbstractCall(const UtlString& sAbstractCallId,
                                                   const SipDialog& sSipDialog,
                                                   const UtlString& sTransferSipUrl)
{
   OsStatus result = OS_NOT_FOUND;

   if (!isAddressValid(sTransferSipUrl))
   {
      return OS_INVALID_ARGUMENT;
   }

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->transferBlind(sSipDialog, sTransferSipUrl);
   }

   return result;
}

OsStatus XCpCallManager::transferConsultativeAbstractCall(const UtlString& sSourceAbstractCallId,
                                                          const SipDialog& sSourceSipDialog,
                                                          const UtlString& sTargetAbstractCallId,
                                                          const SipDialog& sTargetSipDialog)
{
   OsStatus result = OS_NOT_FOUND;
   SipDialog fullTargetSipDialog;

   {
      OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock, scoped
      UtlBoolean resFind = m_callStack.findAbstractCall(sTargetAbstractCallId, ptrLock);
      if (resFind)
      {
         // we found call and have a lock on it
         // get full sip dialog, since the one we got could have only tags & callid. We also need the remaining fields.
         result = ptrLock->getSipDialog(sTargetSipDialog, fullTargetSipDialog);
         if (result != OS_SUCCESS)
         {
            return result;
         }
      }
      else
      {
         return OS_NOT_FOUND;
      }
   }

   if (fullTargetSipDialog.getDialogState() != SipDialog::DIALOG_STATE_ESTABLISHED)
   {
      return OS_FAILED;
   }

   // we cannot lock 2 calls at the same time, it could result in dead lock
   {
      OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock, scoped
      UtlBoolean resFind = m_callStack.findAbstractCall(sSourceAbstractCallId, ptrLock);
      if (resFind)
      {
         // we found call and have a lock on it
         return ptrLock->transferConsultative(sSourceSipDialog, fullTargetSipDialog);
      }
   }

   return result;
}

OsStatus XCpCallManager::audioToneStart(const UtlString& sAbstractCallId,
                                        int iToneId,
                                        UtlBoolean bLocal,
                                        UtlBoolean bRemote,
                                        int duration)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->audioToneStart(iToneId, bLocal, bRemote, duration);
   }

   return result;
}

OsStatus XCpCallManager::audioToneStop(const UtlString& sAbstractCallId)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->audioToneStop();
   }

   return result;
}

OsStatus XCpCallManager::audioFilePlay(const UtlString& sAbstractCallId,
                                       const UtlString& audioFile,
                                       UtlBoolean bRepeat,
                                       UtlBoolean bLocal,
                                       UtlBoolean bRemote,
                                       UtlBoolean bMixWithMic /*= FALSE*/,
                                       int iDownScaling /*= 100*/,
                                       void* pCookie /*= NULL*/)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->audioFilePlay(audioFile, bRepeat, bLocal, bRemote, bMixWithMic, iDownScaling, pCookie);
   }

   return result;
}

OsStatus XCpCallManager::audioBufferPlay(const UtlString& sAbstractCallId,
                                         const void* pAudiobuf,
                                         size_t iBufSize,
                                         int iType,
                                         UtlBoolean bRepeat,
                                         UtlBoolean bLocal,
                                         UtlBoolean bRemote,
                                         UtlBoolean bMixWithMic /*= FALSE*/,
                                         int iDownScaling /*= 100*/,
                                         void* pCookie /*= NULL*/)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->audioBufferPlay(pAudiobuf, iBufSize, iType, bRepeat, bLocal, bRemote,
         bMixWithMic, iDownScaling, pCookie);
   }

   return result;
}

OsStatus XCpCallManager::audioStopPlayback(const UtlString& sAbstractCallId)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->audioStopPlayback();
   }

   return result;
}

OsStatus XCpCallManager::pauseAudioPlayback(const UtlString& sAbstractCallId)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->pauseAudioPlayback();
   }

   return result;
}

OsStatus XCpCallManager::resumeAudioPlayback(const UtlString& sAbstractCallId)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->resumeAudioPlayback();
   }

   return result;
}

OsStatus XCpCallManager::audioRecordStart(const UtlString& sAbstractCallId,
                                          const UtlString& sFile)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->audioRecordStart(sFile);
   }

   return result;
}

OsStatus XCpCallManager::audioRecordStop(const UtlString& sAbstractCallId)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->audioRecordStop();
   }

   return result;
}

OsStatus XCpCallManager::holdAbstractCallConnection(const UtlString& sAbstractCallId,
                                                    const SipDialog& sSipDialog)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->holdConnection(sSipDialog);
   }

   return result;
}

OsStatus XCpCallManager::holdAllConferenceConnections(const UtlString& sConferenceId)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpConference> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findConference(sConferenceId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->holdAllConnections();
   }

   return result;
}

OsStatus XCpCallManager::holdLocalAbstractCallConnection(const UtlString& sAbstractCallId)
{
   return m_callStack.yieldFocus(sAbstractCallId, TRUE);
}

OsStatus XCpCallManager::unholdLocalAbstractCallConnection(const UtlString& sAbstractCallId)
{
   return m_callStack.gainFocus(sAbstractCallId, FALSE);
}

OsStatus XCpCallManager::unholdAllConferenceConnections(const UtlString& sConferenceId)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpConference> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findConference(sConferenceId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->unholdAllConnections();
   }

   return result;
}

OsStatus XCpCallManager::unholdAbstractCallConnection(const UtlString& sAbstractCallId,
                                                      const SipDialog& sSipDialog)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->unholdConnection(sSipDialog);
   }

   return result;
}

OsStatus XCpCallManager::muteInputAbstractCallConnection(const UtlString& sAbstractCallId,
                                                         const SipDialog& sSipDialog)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->muteInputConnection(sSipDialog);
   }

   return result;
}

OsStatus XCpCallManager::unmuteInputAbstractCallConnection(const UtlString& sAbstractCallId,
                                                           const SipDialog& sSipDialog)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->unmuteInputConnection(sSipDialog);
   }

   return result;
}

OsStatus XCpCallManager::limitAbstractCallCodecPreferences(const UtlString& sAbstractCallId,
                                                           const UtlString& sAudioCodecs,
                                                           const UtlString& sVideoCodecs)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->limitCodecPreferences(sAudioCodecs, sVideoCodecs);
   }

   return result;
}

OsStatus XCpCallManager::renegotiateCodecsAbstractCallConnection(const UtlString& sAbstractCallId,
                                                                 const SipDialog& sSipDialog,
                                                                 const UtlString& sAudioCodecs,
                                                                 const UtlString& sVideoCodecs)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->renegotiateCodecsConnection(sSipDialog,
         sAudioCodecs, sVideoCodecs);
   }

   return result;
}

OsStatus XCpCallManager::renegotiateCodecsAllConferenceConnections(const UtlString& sConferenceId,
                                                                   const UtlString& sAudioCodecs,
                                                                   const UtlString& sVideoCodecs)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpConference> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findConference(sConferenceId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->renegotiateCodecsAllConnections(sAudioCodecs, sVideoCodecs);
   }

   return result;
}

void XCpCallManager::enableStun(const UtlString& sStunServer,
                                int iServerPort,
                                int iKeepAlivePeriodSecs /*= 0*/,
                                OsMsgQ* pNotificationQueue /*= NULL*/)
{
   OsLock lock(m_memberMutex); // use wide lock to make sure we enable stun for the correct server

   m_natTraversalConfig.m_sStunServer = sStunServer;
   m_natTraversalConfig.m_iStunPort = iServerPort;
   m_natTraversalConfig.m_iStunKeepAlivePeriodSecs = iKeepAlivePeriodSecs;

   m_rSipUserAgent.enableStun(sStunServer, iServerPort, iKeepAlivePeriodSecs, pNotificationQueue);
}

void XCpCallManager::enableTurn(const UtlString& sTurnServer,
                                int iTurnPort,
                                const UtlString& sTurnUsername,
                                const UtlString& sTurnPassword,
                                int iKeepAlivePeriodSecs /*= 0*/)
{
   OsLock lock(m_memberMutex);

   bool bEnabled = false;
   m_natTraversalConfig.m_sTurnServer = sTurnServer;
   m_natTraversalConfig.m_iTurnPort = iTurnPort;
   m_natTraversalConfig.m_sTurnUsername = sTurnUsername;
   m_natTraversalConfig.m_sTurnPassword = sTurnPassword;
   m_natTraversalConfig.m_iTurnKeepAlivePeriodSecs = iKeepAlivePeriodSecs;
   bEnabled = (m_natTraversalConfig.m_sTurnServer.length() > 0) && portIsValid(m_natTraversalConfig.m_iTurnPort);

   m_rSipUserAgent.getContactDb().enableTurn(bEnabled);
}

OsStatus XCpCallManager::sendInfo(const UtlString& sAbstractCallId,
                                  const SipDialog& sSipDialog,
                                  const UtlString& sContentType,
                                  const char* pContent,
                                  const size_t nContentLength,
                                  void* pCookie)
{
   OsStatus result = OS_FAILED;
   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->sendInfo(sSipDialog, sContentType, pContent, nContentLength, pCookie);
   }

   return result;
}

UtlString XCpCallManager::getNewSipCallId()
{
   return m_sipCallIdGenerator.getNewCallId();
}

UtlString XCpCallManager::getNewCallId()
{
   return XCpCallIdUtil::getNewCallId();
}

UtlString XCpCallManager::getNewConferenceId()
{
   return XCpCallIdUtil::getNewConferenceId();
}

void XCpCallManager::getSessionTimerConfig(int& sessionExpiration, CP_SESSION_TIMER_REFRESH& refresh) const
{
   sessionExpiration = m_sessionTimerExpiration;
   refresh = m_sessionTimerRefresh;
}

void XCpCallManager::setSessionTimerConfig(int sessionExpiration, CP_SESSION_TIMER_REFRESH refresh)
{
   if (sessionExpiration < MIN_SESSION_TIMER_EXPIRATION)
   {
      m_sessionTimerExpiration = MIN_SESSION_TIMER_EXPIRATION;
   }
   else
   {
      m_sessionTimerExpiration = sessionExpiration;
   }

   m_sessionTimerRefresh = refresh;
}

/* ============================ ACCESSORS ================================= */

CpMediaInterfaceFactory* XCpCallManager::getMediaInterfaceFactory() const
{
   return &m_rMediaInterfaceFactory;
}

/* ============================ INQUIRY =================================== */

int XCpCallManager::getCallCount() const
{
   return m_callStack.getCallCount();
}

OsStatus XCpCallManager::getAbstractCallIds(UtlSList& idList) const
{
   // first append call ids
   getCallIds(idList);
   // then append conference ids
   getConferenceIds(idList);

   return OS_SUCCESS;
}

OsStatus XCpCallManager::getCallIds(UtlSList& callIdList) const
{
   return m_callStack.getCallIds(callIdList);
}

OsStatus XCpCallManager::getConferenceIds(UtlSList& conferenceIdList) const
{
   return m_callStack.getConferenceIds(conferenceIdList);
}

OsStatus XCpCallManager::getCallSipCallId(const UtlString& sCallId,
                                          UtlString& sSipCallId) const
{
   OsStatus result = OS_NOT_FOUND;
   sSipCallId.remove(0);

   OsPtrLock<XCpCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findCall(sCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->getCallSipCallId(sSipCallId);
   }

   return result;
}

OsStatus XCpCallManager::getConferenceSipCallIds(const UtlString& sConferenceId,
                                                 UtlSList& sipCallIdList) const
{
   OsStatus result = OS_NOT_FOUND;
   sipCallIdList.destroyAll();

   OsPtrLock<XCpConference> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findConference(sConferenceId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->getConferenceSipCallIds(sipCallIdList);
   }

   return result;
}

OsStatus XCpCallManager::getRemoteUserAgent(const UtlString& sAbstractCallId,
                                            const SipDialog& sSipDialog,
                                            UtlString& userAgent) const
{
   OsStatus result = OS_NOT_FOUND;
   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->getRemoteUserAgent(sSipDialog, userAgent);
   }

   return result;
}

OsStatus XCpCallManager::getMediaConnectionId(const UtlString& sAbstractCallId,
                                              const SipDialog& sSipDialog,
                                              int& mediaConnID) const
{
   OsStatus result = OS_NOT_FOUND;
   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->getMediaConnectionId(sSipDialog, mediaConnID);
   }

   return result;
}

OsStatus XCpCallManager::getSipDialog(const UtlString& sAbstractCallId,
                                      const SipDialog& sSipDialog,
                                      SipDialog& sOutputSipDialog) const
{
   OsStatus result = OS_NOT_FOUND;
   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sAbstractCallId, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->getSipDialog(sSipDialog, sOutputSipDialog);
   }

   return result;
}

UtlBoolean XCpCallManager::isCallEstablished(const SipDialog& sipDialog) const
{
   OsStatus result = OS_NOT_FOUND;
   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sipDialog, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->isConnectionEstablished(sipDialog);
   }

   return result;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean XCpCallManager::checkCallLimit()
{
   if (m_maxCalls == -1)
   {
      return TRUE;
   }

   {
      OsLock lock(m_memberMutex);
      int callCount = getCallCount();
      if (callCount >= m_maxCalls)
      {
         return FALSE;
      }
   }

   return TRUE;
}

// handles OsMsg::PHONE_APP messages
UtlBoolean XCpCallManager::handlePhoneAppMessage(const OsMsg& rRawMsg)
{
   UtlBoolean bResult = FALSE;
   int msgSubType = rRawMsg.getMsgSubType();

   switch (msgSubType)
   {
   case SipMessage::NET_SIP_MESSAGE:
      return handleSipMessageEvent((const SipMessageEvent&)rRawMsg);
   default:
      {
         OsSysLog::add(FAC_CP, PRI_ERR, "Unknown PHONE_APP CallManager message subtype: %d\n", msgSubType);
         break;
      }
   }

   return bResult;
}

UtlBoolean XCpCallManager::handleCallManagerCommandMessage(const CmCommandMsg& rMsg)
{
   CmCommandMsg::SubTypesEnum subType = (CmCommandMsg::SubTypesEnum)rMsg.getMsgSubType();
   switch (subType)
   {
   case CmCommandMsg::CM_GAIN_FOCUS:
      return handleGainFocusCommandMessage((const CmGainFocusMsg&)rMsg);
   case CmCommandMsg::CM_YIELD_FOCUS:
      return handleYieldFocusCommandMessage((const CmYieldFocusMsg&)rMsg);
   case CmCommandMsg::CM_DESTROY_ABSTRACT_CALL:
      return handleDestroyAbstractCallCommandMessage((const CmDestroyAbstractCallMsg&)rMsg);
   default:
      ;
   }

   return FALSE;
}

UtlBoolean XCpCallManager::handleGainFocusCommandMessage(const CmGainFocusMsg& rMsg)
{
   m_callStack.gainFocus(rMsg.getAbstractCallId(), rMsg.getGainOnlyIfNoFocusedCall());
   return TRUE;
}

UtlBoolean XCpCallManager::handleYieldFocusCommandMessage(const CmYieldFocusMsg& rMsg)
{
   m_callStack.yieldFocus(rMsg.getAbstractCallId(), TRUE);
   return TRUE;
}

UtlBoolean XCpCallManager::handleDestroyAbstractCallCommandMessage(const CmDestroyAbstractCallMsg& rMsg)
{
   UtlString sAbstractCallId = rMsg.getAbstractCallId();
   m_callStack.yieldFocus(sAbstractCallId, TRUE); // give focus to next call
   m_callStack.deleteAbstractCall(sAbstractCallId);
   return TRUE;
}

UtlBoolean XCpCallManager::handleSipConnectionCommandMessage(const ScCommandMsg& rMsg)
{
   SipDialog sipDialog;
   rMsg.getSipDialog(sipDialog);

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sipDialog, ptrLock);
   if (resFind)
   {
      // post message to call
      return ptrLock->postMessage(rMsg) == OS_SUCCESS;
   }
   else
   {
      return FALSE;
   }
}

UtlBoolean XCpCallManager::handleSipConnectionNotificationMessage(const ScNotificationMsg& rMsg)
{
   SipDialog sipDialog;
   rMsg.getSipDialog(sipDialog);

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sipDialog, ptrLock);
   if (resFind)
   {
      // post message to call
      return ptrLock->postMessage(rMsg) == OS_SUCCESS;
   }
   else
   {
      return FALSE;
   }
}

UtlBoolean XCpCallManager::handleTimerMessage(const OsMsg& rRawMsg)
{
   const CpTimerMsg *pTimerMsg = dynamic_cast<const CpTimerMsg*>(&rRawMsg);
   if (pTimerMsg)
   {
      // dynamic cast succeeded
      switch ((CpTimerMsg::SubTypeEnum)rRawMsg.getMsgSubType())
      {
      case CpTimerMsg::CP_SIP_CONNECTION_TIMER:
         return handleSipConnectionTimer((const ScTimerMsg&)rRawMsg);
      default:
         break;
      }
   }

   return FALSE;
}

UtlBoolean XCpCallManager::handleSipConnectionTimer(const ScTimerMsg& rMsg)
{
   SipDialog sipDialog;
   rMsg.getSipDialog(sipDialog);

   // find correct call/conference for this timer message
   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sipDialog, ptrLock);
   if (resFind)
   {
      ptrLock->postMessage(rMsg);
      return TRUE;
   }
   else
   {
      OsSysLog::add(FAC_CP, PRI_DEBUG, "No call or conference was found for sip connection timer message subtype %d\n",
         rMsg.getMsgSubType());
   }

   return FALSE;
}

UtlBoolean XCpCallManager::handleSipMessageEvent(const SipMessageEvent& rSipMsgEvent)
{
   const SipMessage* pSipMessage = rSipMsgEvent.getMessage();
   if (pSipMessage)
   {
#ifdef PRINT_SIP_MESSAGE
      enableConsoleOutput(TRUE);
      osPrintf("\nXCpCallManager::handleSipMessageEvent\n%s\n-----------------------------------\n", pSipMessage->toString().data());
#endif
      if (!skipMessage(*pSipMessage))
      {
         OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
         UtlBoolean resFind = m_callStack.findHandlingAbstractCall(*pSipMessage, ptrLock);
         if (resFind)
         {
            // post message to call
            return ptrLock->postMessage(rSipMsgEvent);
         }
         else
         {
            return handleUnknownSipMessageEvent(rSipMsgEvent);
         }
      }      
   }

   return TRUE;
}

UtlBoolean XCpCallManager::handleUnknownSipMessageEvent(const SipMessageEvent& rSipMsgEvent)
{
   int messageType = rSipMsgEvent.getMessageStatus();

   switch(messageType)
   {
   case SipMessageEvent::APPLICATION:
      {
         const SipMessage* pSipMessage = rSipMsgEvent.getMessage();
         if (pSipMessage)
         {
            // Its a SIP Request
            if(pSipMessage->isRequest())
            {
               return handleUnknownSipRequest(*pSipMessage);
            }
         }
      }
   default:
      ;
   }

   return FALSE;
}

// Handler for inbound INVITE SipMessage, for which there is no existing call or conference.
UtlBoolean XCpCallManager::handleUnknownInviteRequest(const SipMessage& rSipMessage)
{
   UtlString toTag;
   rSipMessage.getToFieldTag(toTag);

   if (toTag.isNull())
   {
      // handle INVITE without to tag
      // maybe check if line exists
      if(m_pSipLineProvider && m_bIsRequiredLineMatch)
      {
         UtlBoolean isLineValid = m_pSipLineProvider->lineExists(rSipMessage);
         if (!isLineValid)
         {
            // no such user - return 404
            SipMessage noSuchUserResponse;
            noSuchUserResponse.setResponseData(&rSipMessage,
               SIP_NOT_FOUND_CODE,
               SIP_NOT_FOUND_TEXT);
            m_rSipUserAgent.send(noSuchUserResponse);
            return TRUE;
         }
      }
      // check if we didn't reach call limit
      if (checkCallLimit())
      {
         if (!m_bDoNotDisturb)
         {
            // maybe we have public conference capable of handling the message
            Url requestUri;
            rSipMessage.getRequestUri(requestUri);
            OsPtrLock<XCpConference> ptrConferenceLock; // auto pointer lock
            UtlBoolean resFind = m_callStack.findConferenceByUri(requestUri, ptrConferenceLock);
            if (resFind)
            {
               // post message to conference
               SipMessageEvent sipMessageEvent(rSipMessage);
               ptrConferenceLock->postMessage(sipMessageEvent);
            }
            else
            {
               // all checks passed, create new call
               createNewInboundCall(rSipMessage);
            }
         }
         else
         {
            UtlString requestUri;
            rSipMessage.getRequestUri(&requestUri);
            OsSysLog::add(FAC_CP, PRI_DEBUG, "XCpCallManager::handleSipMessage - Rejecting inbound call to %s due to DND mode", requestUri.data());
            // send 486 Busy here
            SipMessage busyHereResponse;
            busyHereResponse.setInviteBusyData(&rSipMessage);
            m_rSipUserAgent.send(busyHereResponse);
         }
      }
      else
      {
         OsSysLog::add(FAC_CP, PRI_WARNING, "XCpCallManager::handleSipMessage - The call stack size as reached it's limit of %d", m_maxCalls);
         // send 486 Busy here
         SipMessage busyHereResponse;
         busyHereResponse.setInviteBusyData(&rSipMessage);
         m_rSipUserAgent.send(busyHereResponse);
      }

      return TRUE;
   }
   else
   {
      // to tag present but dialog not found -> doesn't exist
      sendBadTransactionError(rSipMessage);
      return TRUE;
   }
}

UtlBoolean XCpCallManager::handleUnknownUpdateRequest(const SipMessage& rSipMessage)
{
   // always send 481 Call/Transaction Does Not Exist for unknown UPDATE requests
   sendBadTransactionError(rSipMessage);
   return TRUE;
}

// Handler for inbound OPTIONS SipMessage, for which there is no existing call or conference. 
UtlBoolean XCpCallManager::handleUnknownOptionsRequest(const SipMessage& rSipMessage)
{
   UtlString fromTag;
   rSipMessage.getFromFieldTag(fromTag);
   UtlString toTag;
   rSipMessage.getToFieldTag(toTag);

   if (!toTag.isNull())
   {
      // to tag present but dialog not found -> doesn't exist
      sendBadTransactionError(rSipMessage);
      return TRUE;
   }
   else if (fromTag.isNull())
   {
      // both tags are NULL
      // out of dialog OPTIONS is handled by SipUserAgent
   }
   return FALSE;
}

UtlBoolean XCpCallManager::handleUnknownReferRequest(const SipMessage& rSipMessage)
{
   UtlString toTag;
   rSipMessage.getToFieldTag(toTag);

   if (!toTag.isNull())
   {
      // to tag present but dialog not found -> doesn't exist
      sendBadTransactionError(rSipMessage);
      return TRUE;
   }
   else
   {
      // handle out of dialog REFER
      // We do not support out of dialog REFER. We won't start dialing just because somebody from outside
      // tells us to.
      SipMessage sipResponse;
      sipResponse.setResponseData(&rSipMessage, SIP_SERVICE_UNAVAILABLE_CODE, SIP_SERVICE_UNAVAILABLE_TEXT);
      m_rSipUserAgent.send(sipResponse);
      return TRUE;
   }
}

UtlBoolean XCpCallManager::handleUnknownCancelRequest(const SipMessage& rSipMessage)
{
   // always send 481 Call/Transaction Does Not Exist for unknown CANCEL requests
   sendBadTransactionError(rSipMessage);
   return TRUE;
}

UtlBoolean XCpCallManager::handleUnknownPrackRequest(const SipMessage& rSipMessage)
{
   // always send 481 Call/Transaction Does Not Exist for unknown PRACK requests
   sendBadTransactionError(rSipMessage);
   return TRUE;
}

UtlBoolean XCpCallManager::handleUnknownInfoRequest(const SipMessage& rSipMessage)
{
   // always send 481 Call/Transaction Does Not Exist for unknown INFO requests
   sendBadTransactionError(rSipMessage);
   return TRUE;
}

// called for inbound request SipMessages, for which calls weren't found
UtlBoolean XCpCallManager::handleUnknownSipRequest(const SipMessage& rSipMessage)
{
   UtlString requestMethod;
   rSipMessage.getRequestMethod(&requestMethod);

   // Dangling or delayed ACK
   if(requestMethod.compareTo(SIP_ACK_METHOD) == 0)
   {
      // ACK has no response
      return TRUE;
   }
   else if(requestMethod.compareTo(SIP_INVITE_METHOD) == 0)
   {
      return handleUnknownInviteRequest(rSipMessage);
   }
   else if(requestMethod.compareTo(SIP_OPTIONS_METHOD) == 0)
   {
      return handleUnknownOptionsRequest(rSipMessage);
   }
   else if(requestMethod.compareTo(SIP_UPDATE_METHOD) == 0)
   {
      return handleUnknownUpdateRequest(rSipMessage);
   }
   else if(requestMethod.compareTo(SIP_REFER_METHOD) == 0)
   {
      return handleUnknownReferRequest(rSipMessage);
   }
   else if(requestMethod.compareTo(SIP_CANCEL_METHOD) == 0)
   {
      return handleUnknownCancelRequest(rSipMessage);
   }
   else if(requestMethod.compareTo(SIP_PRACK_METHOD) == 0)
   {
      return handleUnknownPrackRequest(rSipMessage);
   }
   else if(requestMethod.compareTo(SIP_INFO_METHOD) == 0)
   {
      return handleUnknownInfoRequest(rSipMessage);
   }

   // 481 Call/Transaction Does Not Exist must be sent automatically by somebody else for SUBSCRIBE/NOTIFY messages
   // multiple observers may receive the same SipMessage
   // currently nobody sends these 481s, not even SipSubscribeServer

   return FALSE;
}

void XCpCallManager::createNewInboundCall(const SipMessage& rSipMessage)
{
   UtlString sSipCallId = getNewSipCallId();

   XCpCall* pCall = new XCpCall(sSipCallId, m_rSipUserAgent, *this, m_pSipLineProvider, m_rMediaInterfaceFactory, m_rDefaultSdpCodecList, 
      *getMessageQueue(), m_natTraversalConfig, m_sBindIpAddress, m_sessionTimerExpiration, m_sessionTimerRefresh,
      m_updateSetting, m_100relSetting, m_sdpOfferingMode, m_inviteExpiresSeconds, &m_callStack, m_pCallEventListener,
      m_pInfoStatusEventListener, m_pInfoEventListener, m_pSecurityEventListener, m_pMediaEventListener, m_pRtpRedirectEventListener);

   UtlBoolean resStart = pCall->start(); // start thread
   if (resStart)
   {
      SipMessageEvent sipMessageEvent(rSipMessage);
      pCall->postMessage(sipMessageEvent); // repost message into thread
      UtlBoolean resPush = m_callStack.push(*pCall); // inserts call into list of calls
      if (resPush)
      {
         if (OsSysLog::willLog(FAC_CP, PRI_DEBUG))
         {
            UtlString requestUri;
            rSipMessage.getRequestUri(&requestUri);
            UtlString fromField;
            rSipMessage.getFromField(&fromField);
            OsSysLog::add(FAC_CP, PRI_DEBUG, "XCpCallManager::createNewCall - Creating new call destined to %s from %s", requestUri.data(), fromField.data());
         }
      }
      else
      {
         OsSysLog::add(FAC_CP, PRI_ERR, "XCpCallManager::createNewCall - Couldn't push call on stack");
         pCall->requestShutdown();
         delete pCall;
      }
   }
   else
   {
      OsSysLog::add(FAC_CP, PRI_ERR, "XCpCallManager::createNewCall - Call thread could not be started");
   }
}

UtlBoolean XCpCallManager::sendBadTransactionError(const SipMessage& rSipMessage)
{
   SipMessage badTransactionMessage;
   badTransactionMessage.setBadTransactionData(&rSipMessage);
   return m_rSipUserAgent.send(badTransactionMessage);
}

void XCpCallManager::startSipMessageObserving()
{
   m_rSipUserAgent.addMessageObserver(*(this->getMessageQueue()),
      SIP_INVITE_METHOD,
      TRUE, // want to get requests
      TRUE, // and responses
      TRUE, // Incoming messages
      FALSE); // Don't want to see out going messages
   m_rSipUserAgent.addMessageObserver(*(this->getMessageQueue()),
      SIP_UPDATE_METHOD,
      TRUE, // want to get requests
      TRUE, // and responses
      TRUE, // Incoming messages
      FALSE); // Don't want to see out going messages
   m_rSipUserAgent.addMessageObserver(*(this->getMessageQueue()),
      SIP_BYE_METHOD,
      TRUE, // want to get requests
      TRUE, // and responses
      TRUE, // Incoming messages
      FALSE); // Don't want to see out going messages
   m_rSipUserAgent.addMessageObserver(*(this->getMessageQueue()),
      SIP_CANCEL_METHOD,
      TRUE, // want to get requests
      TRUE, // and responses
      TRUE, // Incoming messages
      FALSE); // Don't want to see out going messages
   m_rSipUserAgent.addMessageObserver(*(this->getMessageQueue()),
      SIP_ACK_METHOD,
      TRUE, // want to get requests
      FALSE, // no such thing as a ACK response
      TRUE, // Incoming messages
      FALSE); // Don't want to see out going messages
   m_rSipUserAgent.addMessageObserver(*(this->getMessageQueue()),
      SIP_REFER_METHOD,
      TRUE, // want to get requests
      TRUE, // and responses
      TRUE, // Incoming messages
      FALSE); // Don't want to see out going messages
   m_rSipUserAgent.addMessageObserver(*(this->getMessageQueue()),
      SIP_OPTIONS_METHOD,
      TRUE, // want to get requests
      TRUE, // do want responses
      TRUE, // Incoming messages
      FALSE); // Don't want to see out going messages
   m_rSipUserAgent.addMessageObserver(*(this->getMessageQueue()),
      SIP_NOTIFY_METHOD,
      TRUE, // do want to get requests
      TRUE, // do want responses
      TRUE, // Incoming messages
      FALSE); // Don't want to see out going messages
   m_rSipUserAgent.addMessageObserver(*(this->getMessageQueue()),
      SIP_INFO_METHOD,
      TRUE, // do want to get requests
      TRUE, // do want responses
      TRUE, // Incoming messages
      FALSE); // Don't want to see out going messages
   m_rSipUserAgent.addMessageObserver(*(this->getMessageQueue()),
      SIP_PRACK_METHOD,
      TRUE, // do want to get requests
      TRUE, // do want responses
      TRUE, // Incoming messages
      FALSE); // Don't want to see out going messages
   m_rSipUserAgent.addMessageObserver(*(this->getMessageQueue()),
      SIP_SUBSCRIBE_METHOD,
      TRUE, // do want to get requests
      TRUE, // do want responses
      TRUE, // Incoming messages
      FALSE); // Don't want to see out going messages
}

void XCpCallManager::stopSipMessageObserving()
{
   m_rSipUserAgent.removeMessageObserver(*(this->getMessageQueue()));
}

UtlBoolean XCpCallManager::skipMessage(const SipMessage& sipMessage) const
{
   if (sipMessage.isOptionsRequest())
   {
      UtlString toTag;
      sipMessage.getToFieldTag(toTag);
      if (toTag.isNull())
      {
         return TRUE; // skip OPTIONS request with NULL to tag, those are handled by SipUserAgent
      }
   }

   return FALSE;
}

UtlBoolean XCpCallManager::isAddressValid(const UtlString& address) const
{
   UtlBoolean returnCode = TRUE;

   // Check that we are adhering to one of the address schemes
   // Currently we only support SIP URLs so everything must map
   // to a SIP URL
   RegEx ip4Address("^[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+$");

   // If it is all digits
   RegEx allDigits("^[0-9*]+$");

   if (allDigits.Search(address.data()))
   {
      // There must be a valid default SIP host address (SIP_DIRECTORY_SERVER)
      UtlString directoryServerAddress;
      int port;
      UtlString protocol;
      m_rSipUserAgent.getDirectoryServer(0, &directoryServerAddress, &port, &protocol);

      // If there is no host or there is an invalid IP4 address
      // We do not validate DNS host names here so that we do not block
      if(directoryServerAddress.isNull() // no host
         || (ip4Address.Search(directoryServerAddress.data())
         && !OsSocket::isIp4Address(directoryServerAddress)
         ))
      {
         returnCode = FALSE;
      }
   }
   else
   {
      // If it is not all digits it must be a SIP URL
      Url addressUrl(address.data());
      UtlString urlHost;
      addressUrl.getHostAddress(urlHost);

      if(urlHost.isNull())
      {
         returnCode = FALSE;
      }
      else
      {
         // If the host name is an IP4 address check that it is valid
         if (ip4Address.Search(urlHost.data()) && !OsSocket::isIp4Address(urlHost))
         {
            returnCode = FALSE;
         }
         else
         {
            UtlString tagValue;
            addressUrl.getFieldParameter("tag", tagValue, 0);
            if (!tagValue.isNull())
            {
               returnCode = FALSE;
            }
         }
      }
   }

   return returnCode;
}

OsStatus XCpCallManager::sendMessage(const OsMsg& msg, const SipDialog& sSipDialog)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(sSipDialog, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->postMessage(msg);
   }

   return result;
}

OsStatus XCpCallManager::subscribe(CP_NOTIFICATION_TYPE notificationType,
                                   const SipDialog& targetSipDialog,
                                   const SipDialog& callbackSipDialog)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(targetSipDialog, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->subscribe(notificationType, targetSipDialog, callbackSipDialog);
   }

   return result;
}

OsStatus XCpCallManager::unsubscribe(CP_NOTIFICATION_TYPE notificationType,
                                     const SipDialog& targetSipDialog,
                                     const SipDialog& callbackSipDialog)
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XCpAbstractCall> ptrLock; // auto pointer lock
   UtlBoolean resFind = m_callStack.findAbstractCall(targetSipDialog, ptrLock);
   if (resFind)
   {
      // we found call and have a lock on it
      return ptrLock->unsubscribe(notificationType, targetSipDialog, callbackSipDialog);
   }

   return result;
}

OsStatus XCpCallManager::createConnectedCall(SipDialog& sipDialog,
                                             const UtlString& toAddress,
                                             const UtlString& fullLineUrl,// includes display name, SIP URI
                                             const UtlString& sSipCallId, // can be used to suggest sip call-id
                                             const UtlString& locationHeader,
                                             CP_CONTACT_ID contactId,
                                             SIP_TRANSPORT_TYPE transport,
                                             CP_FOCUS_CONFIG focusConfig,
                                             const UtlString& replacesField,
                                             CP_CALLSTATE_CAUSE callstateCause,
                                             const SipDialog* pCallbackSipDialog)
{
   UtlString sCallId;
   OsStatus createResult = createCall(sCallId);

   if (createResult == OS_SUCCESS)
   {
      // call was created, try to connect it
      OsStatus connectResult = connectCall(sCallId, sipDialog, toAddress, fullLineUrl, sSipCallId,
         locationHeader, contactId, transport, focusConfig, replacesField, callstateCause, pCallbackSipDialog);

      if (connectResult != OS_SUCCESS)
      {
         destroyCall(sCallId);
      }

      return connectResult;
   }
   else
   {
      return createResult;
   }
}

/* ============================ FUNCTIONS ================================= */
