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
#include <os/OsLock.h>
#include <os/OsReadLock.h>
#include <os/OsWriteLock.h>
#include <os/OsMsgQ.h>
#include <os/OsPtrLock.h>
#include <os/OsIntPtrMsg.h>
#include <net/SipMessage.h>
#include <net/SipMessageEvent.h>
#include <net/QoS.h>
#include <net/SipLineProvider.h>
#include <net/SipLine.h>
#include <sdp/SdpCodecFactory.h>
#include <mi/CpMediaInterfaceFactory.h>
#include <mi/CpMediaInterface.h>
#include <cp/XCpAbstractCall.h>
#include <cp/XSipConnection.h>
#include <cp/CpCodecInfo.h>
#include <cp/XCpCallConnectionListener.h>
#include <cp/CpNotificationMsgDef.h>
#include <cp/CpMessageTypes.h>
#include <cp/msg/AcCommandMsg.h>
#include <cp/msg/AcNotificationMsg.h>
#include <cp/msg/AcAudioBufferPlayMsg.h>
#include <cp/msg/AcAudioFilePlayMsg.h>
#include <cp/msg/AcAudioPausePlaybackMsg.h>
#include <cp/msg/AcAudioResumePlaybackMsg.h>
#include <cp/msg/AcAudioStopPlaybackMsg.h>
#include <cp/msg/AcAudioRecordStartMsg.h>
#include <cp/msg/AcAudioRecordStopMsg.h>
#include <cp/msg/AcAudioToneStartMsg.h>
#include <cp/msg/AcAudioToneStopMsg.h>
#include <cp/msg/AcMuteInputConnectionMsg.h>
#include <cp/msg/AcUnmuteInputConnectionMsg.h>
#include <cp/msg/AcLimitCodecPreferencesMsg.h>
#include <cp/msg/AcAcceptConnectionMsg.h>
#include <cp/msg/AcRejectConnectionMsg.h>
#include <cp/msg/AcRedirectConnectionMsg.h>
#include <cp/msg/AcAnswerConnectionMsg.h>
#include <cp/msg/AcHoldConnectionMsg.h>
#include <cp/msg/AcUnholdConnectionMsg.h>
#include <cp/msg/AcTransferBlindMsg.h>
#include <cp/msg/AcTransferConsultativeMsg.h>
#include <cp/msg/AcRenegotiateCodecsMsg.h>
#include <cp/msg/AcSendInfoMsg.h>
#include <cp/msg/AcSubscribeMsg.h>
#include <cp/msg/AcUnsubscribeMsg.h>
#include <cp/msg/AcAcceptTransferMsg.h>
#include <cp/msg/AcRejectTransferMsg.h>
#include <cp/msg/AcStartedMsg.h>
#include <cp/msg/CmGainFocusMsg.h>
#include <cp/msg/CmYieldFocusMsg.h>
#include <cp/msg/CpTimerMsg.h>
#include <cp/msg/AcTimerMsg.h>
#include <cp/msg/ScTimerMsg.h>
#include <cp/msg/ScCommandMsg.h>
#include <cp/msg/ScNotificationMsg.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const int XCpAbstractCall::CALL_MAX_REQUEST_MSGS = 200;
const UtlContainableType XCpAbstractCall::TYPE = "XCpAbstractCall";

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

XCpAbstractCall::XCpAbstractCall(const UtlString& sId,
                                 SipUserAgent& rSipUserAgent,
                                 XCpCallControl& rCallControl,
                                 SipLineProvider* pSipLineProvider,
                                 CpMediaInterfaceFactory& rMediaInterfaceFactory,
                                 const SdpCodecList& rDefaultSdpCodecList,
                                 OsMsgQ& rCallManagerQueue,
                                 const CpNatTraversalConfig& rNatTraversalConfig,
                                 const UtlString& sBindIpAddress,
                                 int sessionTimerExpiration,
                                 CP_SESSION_TIMER_REFRESH sessionTimerRefresh,
                                 CP_SIP_UPDATE_CONFIG updateSetting,
                                 CP_100REL_CONFIG c100relSetting,
                                 CP_SDP_OFFERING_MODE sdpOfferingMode,
                                 int inviteExpiresSeconds,
                                 XCpCallConnectionListener* pCallConnectionListener,
                                 CpCallStateEventListener* pCallEventListener,
                                 SipInfoStatusEventListener* pInfoStatusEventListener,
                                 SipInfoEventListener* pInfoEventListener,
                                 SipSecurityEventListener* pSecurityEventListener,
                                 CpMediaEventListener* pMediaEventListener,
                                 CpRtpRedirectEventListener* pRtpRedirectEventListener)
: OsServerTask("XCpAbstractCall-%d", NULL, CALL_MAX_REQUEST_MSGS)
, m_memberMutex(OsMutex::Q_FIFO)
, m_sId(sId)
, m_rSipUserAgent(rSipUserAgent)
, m_rCallControl(rCallControl)
, m_pSipLineProvider(pSipLineProvider)
, m_rMediaInterfaceFactory(rMediaInterfaceFactory)
, m_rDefaultSdpCodecList(rDefaultSdpCodecList)
, m_rCallManagerQueue(rCallManagerQueue)
, m_pMediaInterface(NULL)
, m_instanceRWMutex(OsRWMutex::Q_FIFO)
, m_pCallConnectionListener(pCallConnectionListener)
, m_pCallEventListener(pCallEventListener)
, m_pInfoStatusEventListener(pInfoStatusEventListener)
, m_pInfoEventListener(pInfoEventListener)
, m_pSecurityEventListener(pSecurityEventListener)
, m_pMediaEventListener(pMediaEventListener)
, m_pRtpRedirectEventListener(pRtpRedirectEventListener)
, m_natTraversalConfig(rNatTraversalConfig)
, m_sBindIpAddress(sBindIpAddress)
, m_sessionTimerExpiration(sessionTimerExpiration)
, m_sessionTimerRefresh(sessionTimerRefresh)
, m_updateSetting(updateSetting)
, m_100relSetting(c100relSetting)
, m_sdpOfferingMode(sdpOfferingMode)
, m_focusConfig(CP_FOCUS_IF_AVAILABLE)
, m_inviteExpiresSeconds(inviteExpiresSeconds)
{

}

XCpAbstractCall::~XCpAbstractCall()
{
   waitUntilShutDown();
   // release media interface if its still present. This should never happen. Only here as the last resort.
   releaseMediaInterface();
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean XCpAbstractCall::start()
{
   UtlBoolean res = OsServerTask::start();

   if (res)
   {
      // post notification that abstract call has been started
      AcStartedMsg msg;
      postMessage(msg);
   }

   return res;
}

UtlBoolean XCpAbstractCall::handleMessage(OsMsg& rRawMsg)
{
   UtlBoolean bResult = FALSE;

   switch (rRawMsg.getMsgType())
   {
   case CpMessageTypes::AC_COMMAND:
      return handleCommandMessage((const AcCommandMsg&)rRawMsg);
   case CpMessageTypes::AC_NOTIFICATION:
      return handleNotificationMessage((const AcNotificationMsg&)rRawMsg);
   case CpMessageTypes::SC_COMMAND:
      return handleSipConnectionCommandMessage((const ScCommandMsg&)rRawMsg);
   case CpMessageTypes::SC_NOFITICATION:
      return handleSipConnectionNotificationMessage((const ScNotificationMsg&)rRawMsg);
   case OsMsg::PHONE_APP:
      return handlePhoneAppMessage(rRawMsg);
   case CpTimerMsg::OS_TIMER_MSG:
      return handleTimerMessage((const CpTimerMsg&)rRawMsg);
   case OsMsg::MP_CONNECTION_NOTF_MSG:
      return handleConnectionNotfMessage((const OsIntPtrMsg&)rRawMsg);
   case OsMsg::MP_INTERFACE_NOTF_MSG:
      return handleInterfaceNotfMessage((const OsIntPtrMsg&)rRawMsg);
   case OsMsg::OS_EVENT: // timer event
   default:
      break;
   }

   return bResult;
}

OsStatus XCpAbstractCall::acceptConnection(const SipDialog& sSipDialog,
                                           UtlBoolean bSendSDP,
                                           const UtlString& locationHeader,
                                           CP_CONTACT_ID contactId,
                                           SIP_TRANSPORT_TYPE transport)
{
   AcAcceptConnectionMsg acceptConnectionMsg(sSipDialog, bSendSDP, locationHeader, contactId, transport);
   return postMessage(acceptConnectionMsg);
}

OsStatus XCpAbstractCall::rejectConnection(const SipDialog& sSipDialog)
{
   AcRejectConnectionMsg rejectConnectionMsg(sSipDialog);
   return postMessage(rejectConnectionMsg);
}

OsStatus XCpAbstractCall::redirectConnection(const SipDialog& sSipDialog,
                                             const UtlString& sRedirectSipUrl)
{
   AcRedirectConnectionMsg redirectConnectionMsg(sSipDialog, sRedirectSipUrl);
   return postMessage(redirectConnectionMsg);
}

OsStatus XCpAbstractCall::answerConnection(const SipDialog& sSipDialog)
{
   AcAnswerConnectionMsg answerConnectionMsg(sSipDialog);
   return postMessage(answerConnectionMsg);
}

OsStatus XCpAbstractCall::acceptConnectionTransfer(const SipDialog& sipDialog)
{
   AcAcceptTransferMsg acceptTransferMsg(sipDialog);
   return postMessage(acceptTransferMsg);
}

OsStatus XCpAbstractCall::rejectConnectionTransfer(const SipDialog& sipDialog)
{
   AcRejectTransferMsg rejectTransferMsg(sipDialog);
   return postMessage(rejectTransferMsg);
}

OsStatus XCpAbstractCall::transferBlind(const SipDialog& sipDialog, const UtlString& sTransferSipUrl)
{
   AcTransferBlindMsg transferBlindMsg(sipDialog, sTransferSipUrl);
   return postMessage(transferBlindMsg);
}

OsStatus XCpAbstractCall::transferConsultative(const SipDialog& sourceSipDialog, const SipDialog& targetSipDialog)
{
   AcTransferConsultativeMsg transferConsultativeMsg(sourceSipDialog, targetSipDialog);
   return postMessage(transferConsultativeMsg);
}

OsStatus XCpAbstractCall::holdConnection(const SipDialog& sipDialog)
{
   AcHoldConnectionMsg holdConnectionMsg(sipDialog);
   return postMessage(holdConnectionMsg);
}

OsStatus XCpAbstractCall::unholdConnection(const SipDialog& sipDialog)
{
   AcUnholdConnectionMsg unholdConnectionMsg(sipDialog);
   return postMessage(unholdConnectionMsg);
}

OsStatus XCpAbstractCall::audioToneStart(int iToneId,
                                       UtlBoolean bLocal,
                                       UtlBoolean bRemote,
                                       int duration)
{
   AcAudioToneStartMsg audioToneStartMsg(iToneId, bLocal, bRemote, duration);
   return postMessage(audioToneStartMsg);
}

OsStatus XCpAbstractCall::audioToneStop()
{
   AcAudioToneStopMsg audioToneStopMsg;
   return postMessage(audioToneStopMsg);
}

OsStatus XCpAbstractCall::audioFilePlay(const UtlString& audioFile,
                                        UtlBoolean bRepeat,
                                        UtlBoolean bLocal,
                                        UtlBoolean bRemote,
                                        UtlBoolean bMixWithMic /*= FALSE*/,
                                        int iDownScaling /*= 100*/,
                                        void* pCookie /*= NULL*/)
{
   AcAudioFilePlayMsg audioFilePlayMsg(audioFile, bRepeat, bLocal, bRemote, bMixWithMic, iDownScaling, pCookie);
   return postMessage(audioFilePlayMsg);
}

OsStatus XCpAbstractCall::audioBufferPlay(const void* pAudiobuf,
                                          size_t iBufSize,
                                          int iType,
                                          UtlBoolean bRepeat,
                                          UtlBoolean bLocal,
                                          UtlBoolean bRemote,
                                          UtlBoolean bMixWithMic /*= FALSE*/,
                                          int iDownScaling /*= 100*/,
                                          void* pCookie /*= NULL*/)
{
   AcAudioBufferPlayMsg audioBufferPlayMsg(pAudiobuf, iBufSize, iType, bRepeat, bLocal,
      bRemote, bMixWithMic, iDownScaling, pCookie);
   return postMessage(audioBufferPlayMsg);
}

OsStatus XCpAbstractCall::audioStopPlayback()
{
   AcAudioStopPlaybackMsg audioStopPlaybackMsg;
   return postMessage(audioStopPlaybackMsg);
}

OsStatus XCpAbstractCall::pauseAudioPlayback()
{
   AcAudioPausePlaybackMsg audioPausePlaybackMsg;
   return postMessage(audioPausePlaybackMsg);
}

OsStatus XCpAbstractCall::resumeAudioPlayback()
{
   AcAudioResumePlaybackMsg audioResumePlaybackMsg;
   return postMessage(audioResumePlaybackMsg);
}

OsStatus XCpAbstractCall::audioRecordStart(const UtlString& sFile)
{
   AcAudioRecordStartMsg audioRecordStartMsg(sFile);
   return postMessage(audioRecordStartMsg);
}

OsStatus XCpAbstractCall::audioRecordStop()
{
   AcAudioRecordStopMsg audioRecordStopMsg;
   return postMessage(audioRecordStopMsg);
}

OsStatus XCpAbstractCall::muteInputConnection(const SipDialog& sipDialog)
{
   AcMuteInputConnectionMsg muteInputConnectionMsg(sipDialog);
   return postMessage(muteInputConnectionMsg);
}

OsStatus XCpAbstractCall::unmuteInputConnection(const SipDialog& sipDialog)
{
   AcUnmuteInputConnectionMsg unmuteInputConnectionMsg(sipDialog);
   return postMessage(unmuteInputConnectionMsg);
}

OsStatus XCpAbstractCall::limitCodecPreferences(const UtlString& sAudioCodecs,
                                                const UtlString& sVideoCodecs)
{
   AcLimitCodecPreferencesMsg limitCodecPreferencesMsg(sAudioCodecs, sVideoCodecs);
   return postMessage(limitCodecPreferencesMsg);
}

OsStatus XCpAbstractCall::renegotiateCodecsConnection(const SipDialog& sipDialog,
                                                      const UtlString& sAudioCodecs,
                                                      const UtlString& sVideoCodecs)
{
   AcRenegotiateCodecsMsg renegotiateCodecsMsg(sipDialog, sAudioCodecs,
      sVideoCodecs);
   return postMessage(renegotiateCodecsMsg);
}

OsStatus XCpAbstractCall::sendInfo(const SipDialog& sipDialog,
                                   const UtlString& sContentType,
                                   const char* pContent,
                                   const size_t nContentLength,
                                   void* pCookie)
{
   AcSendInfoMsg sendInfoMsg(sipDialog, sContentType, pContent, nContentLength, pCookie);
   return postMessage(sendInfoMsg);
}

OsStatus XCpAbstractCall::subscribe(CP_NOTIFICATION_TYPE notificationType,
                                    const SipDialog& targetSipDialog,
                                    const SipDialog& callbackSipDialog)
{
   AcSubscribeMsg subscribeMsg(notificationType, targetSipDialog, callbackSipDialog);
   return postMessage(subscribeMsg);
}

OsStatus XCpAbstractCall::unsubscribe(CP_NOTIFICATION_TYPE notificationType,
                                      const SipDialog& targetSipDialog,
                                      const SipDialog& callbackSipDialog)
{
   AcUnsubscribeMsg unsubscribeMsg(notificationType, targetSipDialog, callbackSipDialog);
   return postMessage(unsubscribeMsg);
}

OsStatus XCpAbstractCall::acquireExclusive()
{
   return m_instanceRWMutex.acquireWrite();
}

/* ============================ ACCESSORS ================================= */

unsigned XCpAbstractCall::hash() const
{
   return (unsigned)this;
}

UtlContainableType XCpAbstractCall::getContainableType() const
{
   return XCpAbstractCall::TYPE;
}

UtlString XCpAbstractCall::getId() const
{
   return m_sId;
}

/* ============================ INQUIRY =================================== */

int XCpAbstractCall::compareTo(UtlContainable const* inVal) const
{
   int result;

   if (inVal->isInstanceOf(XCpAbstractCall::TYPE))
   {
      result = hash() - inVal->hash();
   }
   else
   {
      result = -1; 
   }

   return result;
}

OsStatus XCpAbstractCall::getRemoteUserAgent(const SipDialog& sipDialog, UtlString& userAgent) const
{
   OsStatus result = OS_INVALID;

   OsPtrLock<XSipConnection> ptrLock; // auto pointer lock
   UtlBoolean resFind = findConnection(sipDialog, ptrLock);
   if (resFind)
   {
      ptrLock->getRemoteUserAgent(userAgent);
      result = OS_SUCCESS;
   }

   return result;
}

OsStatus XCpAbstractCall::getMediaConnectionId(const SipDialog& sipDialog, int& mediaConnID) const
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XSipConnection> ptrLock; // auto pointer lock
   UtlBoolean resFind = findConnection(sipDialog, ptrLock);
   if (resFind)
   {
      mediaConnID = ptrLock->getMediaConnectionId();
      result = OS_SUCCESS;
   }

   return result;
}

OsStatus XCpAbstractCall::getSipDialog(const SipDialog& sipDialog,
                                       SipDialog& sOutputSipDialog) const
{
   OsStatus result = OS_NOT_FOUND;

   OsPtrLock<XSipConnection> ptrLock; // auto pointer lock
   UtlBoolean resFind = findConnection(sipDialog, ptrLock);
   if (resFind)
   {
      ptrLock->getSipDialog(sOutputSipDialog);
      result = OS_SUCCESS;
   }

   return result;
}

UtlBoolean XCpAbstractCall::isConnectionEstablished(const SipDialog& sipDialog) const
{
   OsPtrLock<XSipConnection> ptrLock; // auto pointer lock
   UtlBoolean resFind = findConnection(sipDialog, ptrLock);
   if (resFind)
   {
      return ptrLock->isConnectionEstablished();
   }

   return FALSE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean XCpAbstractCall::handleCommandMessage(const AcCommandMsg& rRawMsg)
{
   switch ((AcCommandMsg::SubTypeEnum)rRawMsg.getMsgSubType())
   {
   case AcCommandMsg::AC_GAIN_FOCUS:
      handleGainFocus((const AcGainFocusMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_YIELD_FOCUS:
      handleDefocus((const AcYieldFocusMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_AUDIO_BUFFER_PLAY:
      handleAudioBufferPlay((const AcAudioBufferPlayMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_AUDIO_FILE_PLAY:
      handleAudioFilePlay((const AcAudioFilePlayMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_AUDIO_STOP_PLAYBACK:
      handleAudioStopPlayback((const AcAudioStopPlaybackMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_AUDIO_PAUSE_PLAYBACK:
      handleAudioPausePlayback((const AcAudioPausePlaybackMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_AUDIO_RESUME_PLAYBACK:
      handleAudioResumePlayback((const AcAudioResumePlaybackMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_AUDIO_RECORD_START:
      handleAudioRecordStart((const AcAudioRecordStartMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_AUDIO_RECORD_STOP:
      handleAudioRecordStop((const AcAudioRecordStopMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_AUDIO_TONE_START:
      handleAudioToneStart((const AcAudioToneStartMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_AUDIO_TONE_STOP:
      handleAudioToneStop((const AcAudioToneStopMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_MUTE_INPUT_CONNECTION:
      handleMuteInputConnection((const AcMuteInputConnectionMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_UNMUTE_INPUT_CONNECTION:
      handleUnmuteInputConnection((const AcUnmuteInputConnectionMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_LIMIT_CODEC_PREFERENCES:
      handleLimitCodecPreferences((const AcLimitCodecPreferencesMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_SUBSCRIBE:
      handleSubscribe((const AcSubscribeMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_UNSUBSCRIBE:
      handleUnsubscribe((const AcUnsubscribeMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_ACCEPT_CONNECTION:
      handleAcceptConnection((const AcAcceptConnectionMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_REJECT_CONNECTION:
      handleRejectConnection((const AcRejectConnectionMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_REDIRECT_CONNECTION:
      handleRedirectConnection((const AcRedirectConnectionMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_ANSWER_CONNECTION:
      handleAnswerConnection((const AcAnswerConnectionMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_ACCEPT_TRANSFER:
      handleAcceptTransfer((const AcAcceptTransferMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_REJECT_TRANSFER:
      handleRejectTransfer((const AcRejectTransferMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_DESTROY_CONNECTION:
      handleDestroyConnection((const AcDestroyConnectionMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_TRANSFER_BLIND:
      handleTransferBlind((const AcTransferBlindMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_TRANSFER_CONSULTATIVE:
      handleTransferConsultative((const AcTransferConsultativeMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_HOLD_CONNECTION:
      handleHoldConnection((const AcHoldConnectionMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_UNHOLD_CONNECTION:
      handleUnholdConnection((const AcUnholdConnectionMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_RENEGOTIATE_CODECS:
      handleRenegotiateCodecs((const AcRenegotiateCodecsMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_SEND_INFO:
      handleSendInfo((const AcSendInfoMsg&)rRawMsg);
      return TRUE;
   default:
      break;
   }

   return FALSE;
}

UtlBoolean XCpAbstractCall::handleNotificationMessage(const AcNotificationMsg& rRawMsg)
{
   switch ((AcNotificationMsg::SubTypesEnum)rRawMsg.getMsgSubType())
   {
   case AcNotificationMsg::ACN_STARTED:
      onStarted();
      return TRUE;
   default:
      break;
   }

   return FALSE;
}

UtlBoolean XCpAbstractCall::handleTimerMessage(const CpTimerMsg& rRawMsg)
{
   switch ((CpTimerMsg::SubTypeEnum)rRawMsg.getMsgSubType())
   {
   case CpTimerMsg::CP_ABSTRACT_CALL_TIMER:
      return handleCallTimer((const AcTimerMsg&)rRawMsg);
   case CpTimerMsg::CP_SIP_CONNECTION_TIMER:
      return handleSipConnectionTimer((const ScTimerMsg&)rRawMsg);
   default:
      break;
   }

   return FALSE;
}

OsStatus XCpAbstractCall::handleSipMessageEvent(const SipMessageEvent& rSipMsgEvent)
{
   const SipMessage* pSipMessage = rSipMsgEvent.getMessage();
   if (pSipMessage)
   {
      OsPtrLock<XSipConnection> ptrLock;
      UtlBoolean resFound = findConnection(*pSipMessage, ptrLock);
      if (resFound)
      {
         if (ptrLock->handleSipMessageEvent(rSipMsgEvent))
         {
            return OS_SUCCESS;
         }
      }
      else
      {
         return OS_NOT_FOUND;
      }
   }

   return OS_FAILED;
}

UtlBoolean XCpAbstractCall::findConnection(const SipMessage& sipMessage, OsPtrLock<XSipConnection>& ptrLock) const
{
   SipDialog sipDialog(&sipMessage);

   return findConnection(sipDialog, ptrLock);
}

OsStatus XCpAbstractCall::gainFocus(UtlBoolean bGainOnlyIfNoFocusedCall)
{
#ifndef DISABLE_LOCAL_AUDIO
   CmGainFocusMsg gainFocusMsg(m_sId, bGainOnlyIfNoFocusedCall);
   return m_rCallManagerQueue.send(gainFocusMsg);
#else
   return OS_SUCCESS;
#endif
}

OsStatus XCpAbstractCall::yieldFocus()
{
#ifndef DISABLE_LOCAL_AUDIO
   CmYieldFocusMsg yieldFocusMsg(m_sId);
   return m_rCallManagerQueue.send(yieldFocusMsg);
#else
   return OS_SUCCESS;
#endif
}

void XCpAbstractCall::onConnectionAddded(const UtlString& sSipCallId)
{
   if (m_pCallConnectionListener)
   {
      m_pCallConnectionListener->onConnectionAdded(sSipCallId, this);
   }
}

void XCpAbstractCall::onConnectionRemoved(const UtlString& sSipCallId)
{
   if (m_pCallConnectionListener)
   {
      m_pCallConnectionListener->onConnectionRemoved(sSipCallId, this);
   }
}

UtlBoolean XCpAbstractCall::handleCallTimer(const AcTimerMsg& timerMsg)
{
   switch (timerMsg.getPayloadType())
   {
   case AcTimerMsg::PAYLOAD_TYPE_FIRST:
   default:
      ;
   }

   return FALSE;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

OsStatus XCpAbstractCall::handleGainFocus(const AcGainFocusMsg& rMsg)
{
#ifndef DISABLE_LOCAL_AUDIO
   if (m_pMediaInterface && !m_pMediaInterface->hasFocus())
   {
      return m_pMediaInterface->giveFocus();
   }

   return OS_FAILED;
#else
   return OS_SUCCESS;
#endif
}

OsStatus XCpAbstractCall::handleDefocus(const AcYieldFocusMsg& rMsg)
{
#ifndef DISABLE_LOCAL_AUDIO
   if (m_pMediaInterface && m_pMediaInterface->hasFocus())
   {
      return m_pMediaInterface->defocus();
   }

   return OS_FAILED;
#else
   return OS_SUCCESS;
#endif
}

OsStatus XCpAbstractCall::handleAudioBufferPlay(const AcAudioBufferPlayMsg& rMsg)
{
   if (m_pMediaInterface)
   {
      return m_pMediaInterface->playBuffer(rMsg.getAudiobuf(), rMsg.getBufSize(),
         rMsg.getType(), rMsg.getRepeat(), rMsg.getLocal(), rMsg.getRemote(),
         rMsg.getMixWithMic(), rMsg.getDownScaling(), rMsg.getCookie());
   }

   return OS_FAILED;
}

OsStatus XCpAbstractCall::handleAudioFilePlay(const AcAudioFilePlayMsg& rMsg)
{
   if (m_pMediaInterface)
   {
      return m_pMediaInterface->playAudio(rMsg.getAudioFile(), rMsg.getRepeat(), rMsg.getLocal(), rMsg.getRemote(),
         rMsg.getMixWithMic(), rMsg.getDownScaling(), rMsg.getCookie());
   }

   return OS_FAILED;
}

OsStatus XCpAbstractCall::handleAudioPausePlayback(const AcAudioPausePlaybackMsg& rMsg)
{
   if (m_pMediaInterface)
   {
      return m_pMediaInterface->pausePlayback();
   }

   return OS_FAILED;
}

OsStatus XCpAbstractCall::handleAudioResumePlayback(const AcAudioResumePlaybackMsg& rMsg)
{
   if (m_pMediaInterface)
   {
      return m_pMediaInterface->resumePlayback();
   }

   return OS_FAILED;
}

OsStatus XCpAbstractCall::handleAudioStopPlayback(const AcAudioStopPlaybackMsg& rMsg)
{
   if (m_pMediaInterface)
   {
      return m_pMediaInterface->stopAudio();
   }

   return OS_FAILED;
}

OsStatus XCpAbstractCall::handleAudioRecordStart(const AcAudioRecordStartMsg& rMsg)
{
   if (m_pMediaInterface)
   {
      return m_pMediaInterface->recordAudio(rMsg.getFile());
   }

   return OS_FAILED;
}

OsStatus XCpAbstractCall::handleAudioRecordStop(const AcAudioRecordStopMsg& rMsg)
{
   if (m_pMediaInterface)
   {
      return m_pMediaInterface->stopRecording();
   }

   return OS_FAILED;
}

OsStatus XCpAbstractCall::handleAudioToneStart(const AcAudioToneStartMsg& rMsg)
{
   if (m_pMediaInterface)
   {
      return m_pMediaInterface->startTone(rMsg.getToneId(), rMsg.getLocal(), rMsg.getRemote(),
         rMsg.getDuration());
   }

   return OS_FAILED;
}

OsStatus XCpAbstractCall::handleAudioToneStop(const AcAudioToneStopMsg& rMsg)
{
   if (m_pMediaInterface)
   {
      return m_pMediaInterface->stopTone();
   }

   return OS_FAILED;
}

OsStatus XCpAbstractCall::handleMuteInputConnection(const AcMuteInputConnectionMsg& rMsg)
{
   SipDialog sipDialog;
   rMsg.getSipDialog(sipDialog);
   // find connection by sip dialog
   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFound = findConnection(sipDialog, ptrLock);
   if (resFound)
   {
      return ptrLock->muteInputConnection();
   }

   return OS_NOT_FOUND;
}

OsStatus XCpAbstractCall::handleUnmuteInputConnection(const AcUnmuteInputConnectionMsg& rMsg)
{
   SipDialog sipDialog;
   rMsg.getSipDialog(sipDialog);
   // find connection by sip dialog
   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFound = findConnection(sipDialog, ptrLock);
   if (resFound)
   {
      return ptrLock->unmuteInputConnection();
   }

   return OS_NOT_FOUND;
}

OsStatus XCpAbstractCall::handleLimitCodecPreferences(const AcLimitCodecPreferencesMsg& rMsg)
{
   UtlString audioCodecs = SdpCodecFactory::getFixedAudioCodecs(rMsg.getAudioCodecs()); // add "telephone-event" if its missing
   return doLimitCodecPreferences(audioCodecs, rMsg.getVideoCodecs());
}

OsStatus XCpAbstractCall::handleSubscribe(const AcSubscribeMsg& rMsg)
{
   SipDialog targetSipDialog;
   SipDialog callbackSipDialog;
   rMsg.getTargetSipDialog(targetSipDialog);
   rMsg.getCallbackSipDialog(callbackSipDialog);
   // find connection by sip dialog
   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFound = findConnection(targetSipDialog, ptrLock);
   if (resFound)
   {
      return ptrLock->subscribe(rMsg.getNotificationType(), callbackSipDialog);
   }

   return OS_NOT_FOUND;
}

OsStatus XCpAbstractCall::handleUnsubscribe(const AcUnsubscribeMsg& rMsg)
{
   SipDialog targetSipDialog;
   SipDialog callbackSipDialog;
   rMsg.getTargetSipDialog(targetSipDialog);
   rMsg.getCallbackSipDialog(callbackSipDialog);
   // find connection by sip dialog
   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFound = findConnection(targetSipDialog, ptrLock);
   if (resFound)
   {
      return ptrLock->unsubscribe(rMsg.getNotificationType(), callbackSipDialog);
   }

   return OS_NOT_FOUND;
}

OsStatus XCpAbstractCall::handleAcceptTransfer(const AcAcceptTransferMsg& rMsg)
{
   SipDialog sipDialog;
   rMsg.getSipDialog(sipDialog);
   // find connection by sip dialog
   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFound = findConnection(sipDialog, ptrLock);
   if (resFound)
   {
      return ptrLock->acceptTransfer();
   }

   return OS_NOT_FOUND;
}

OsStatus XCpAbstractCall::handleRejectTransfer(const AcRejectTransferMsg& rMsg)
{
   SipDialog sipDialog;
   rMsg.getSipDialog(sipDialog);
   // find connection by sip dialog
   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFound = findConnection(sipDialog, ptrLock);
   if (resFound)
   {
      return ptrLock->rejectTransfer();
   }

   return OS_NOT_FOUND;
}

UtlBoolean XCpAbstractCall::handlePhoneAppMessage(const OsMsg& rRawMsg)
{
   UtlBoolean bResult = FALSE;
   int msgSubType = rRawMsg.getMsgSubType();

   switch (msgSubType)
   {
   case SipMessage::NET_SIP_MESSAGE:
      {
         OsStatus res = handleSipMessageEvent((const SipMessageEvent&)rRawMsg);
         if (res == OS_SUCCESS)
         {
            bResult = TRUE;
         }
         else if (res == OS_NOT_FOUND)
         {
            // if connection for sip message was not found, repost message back to call manager
            getGlobalQueue().send(rRawMsg);
         }
         break;
      }
   default:
      {
         OsSysLog::add(FAC_CP, PRI_ERR, "Unknown PHONE_APP XCpAbstractCall message subtype: %d\n", msgSubType);
         break;
      }
   }

   return bResult;
}

UtlBoolean XCpAbstractCall::handleConnectionNotfMessage(const OsIntPtrMsg& rMsg)
{
   CpNotificationMsgMedia media = (CpNotificationMsgMedia)rMsg.getMsgSubType();
   CpNotificationMsgType type = (CpNotificationMsgType)rMsg.getData1();
   int mediaConnectionId = rMsg.getData2();
   intptr_t pData1 = rMsg.getData3();
   intptr_t pData2 = rMsg.getData4();

   switch(type)
   {
   case CP_NOTIFICATION_DTMF_INBAND:
      fireSipXMediaConnectionEvent(CP_MEDIA_REMOTE_DTMF, CP_MEDIA_CAUSE_DTMF_INBAND, (CP_MEDIA_TYPE)media, mediaConnectionId, pData1, pData2);
      break;
   case CP_NOTIFICATION_DTMF_RFC2833:
      fireSipXMediaConnectionEvent(CP_MEDIA_REMOTE_DTMF, CP_MEDIA_CAUSE_DTMF_RFC2833, (CP_MEDIA_TYPE)media, mediaConnectionId, pData1, pData2);
      break;
   case CP_NOTIFICATION_START_RTP_SEND:
      {
         UtlBoolean bCodecKnown = FALSE;
         if (m_pMediaInterface)
         {
            // try to get codec from media interface
            CpCodecInfo codec;
            if (m_pMediaInterface->getPrimaryCodec(mediaConnectionId, codec.m_audioCodec.m_codecName,
               codec.m_videoCodec.m_codecName, &codec.m_audioCodec.m_iPayloadId,
               &codec.m_videoCodec.m_iPayloadType, codec.m_bIsEncrypted) == OS_SUCCESS)
            {
               bCodecKnown = TRUE;
               fireSipXMediaConnectionEvent(CP_MEDIA_LOCAL_START, CP_MEDIA_CAUSE_NORMAL, (CP_MEDIA_TYPE)media, mediaConnectionId, (intptr_t)&codec, NULL);
            }
         }
         if (!bCodecKnown)
         {
            // fire event without codec
            fireSipXMediaConnectionEvent(CP_MEDIA_LOCAL_START, CP_MEDIA_CAUSE_NORMAL, (CP_MEDIA_TYPE)media, mediaConnectionId, NULL, NULL);
         }
         break;
      }
   case CP_NOTIFICATION_STOP_RTP_SEND:
      fireSipXMediaConnectionEvent(CP_MEDIA_LOCAL_STOP, CP_MEDIA_CAUSE_NORMAL, (CP_MEDIA_TYPE)media, mediaConnectionId, NULL, NULL);
      break;
   case CP_NOTIFICATION_START_RTP_RECEIVE:
      fireSipXMediaConnectionEvent(CP_MEDIA_REMOTE_START, CP_MEDIA_CAUSE_NORMAL, (CP_MEDIA_TYPE)media, mediaConnectionId, pData1, pData2);
      break;
   case CP_NOTIFICATION_STOP_RTP_RECEIVE:
      fireSipXMediaConnectionEvent(CP_MEDIA_REMOTE_STOP, CP_MEDIA_CAUSE_NORMAL, (CP_MEDIA_TYPE)media, mediaConnectionId, pData1, pData2);
      break;
   case CP_NOTIFICATION_REMOTE_SILENT:
      fireSipXMediaConnectionEvent(CP_MEDIA_REMOTE_SILENT, CP_MEDIA_CAUSE_NORMAL, (CP_MEDIA_TYPE)media, mediaConnectionId, pData1, NULL);
      break;
   case CP_NOTIFICATION_REMOTE_ACTIVE:
      fireSipXMediaConnectionEvent(CP_MEDIA_REMOTE_ACTIVE, CP_MEDIA_CAUSE_NORMAL, (CP_MEDIA_TYPE)media, mediaConnectionId, NULL, NULL);
      break;
   default:
      assert(false);
   }

   return TRUE;
}

UtlBoolean XCpAbstractCall::handleInterfaceNotfMessage(const OsIntPtrMsg& rMsg)
{
   CpNotificationMsgMedia media = (CpNotificationMsgMedia)rMsg.getMsgSubType();
   CpNotificationMsgType type = (CpNotificationMsgType)rMsg.getData1();
   intptr_t pData1 = rMsg.getData2();
   intptr_t pData2 = rMsg.getData3();

   switch(type)
   {
   case CP_NOTIFICATION_START_PLAY_FILE:
      fireSipXMediaInterfaceEvent(CP_MEDIA_PLAYFILE_START, CP_MEDIA_CAUSE_NORMAL, (CP_MEDIA_TYPE)media, pData1, pData2);
      break;
   case CP_NOTIFICATION_STOP_PLAY_FILE:
      fireSipXMediaInterfaceEvent(CP_MEDIA_PLAYFILE_STOP, CP_MEDIA_CAUSE_NORMAL, (CP_MEDIA_TYPE)media, pData1, pData2);
      break;
   case CP_NOTIFICATION_START_PLAY_BUFFER:
      fireSipXMediaInterfaceEvent(CP_MEDIA_PLAYBUFFER_START, CP_MEDIA_CAUSE_NORMAL, (CP_MEDIA_TYPE)media, pData1, pData2);
      break;
   case CP_NOTIFICATION_STOP_PLAY_BUFFER:
      fireSipXMediaInterfaceEvent(CP_MEDIA_PLAYBUFFER_STOP, CP_MEDIA_CAUSE_NORMAL, (CP_MEDIA_TYPE)media, pData1, pData2);
      break;
   case CP_NOTIFICATION_PAUSE_PLAYBACK:
      fireSipXMediaInterfaceEvent(CP_MEDIA_PLAYBACK_PAUSED, CP_MEDIA_CAUSE_NORMAL, (CP_MEDIA_TYPE)media, pData1, pData2);
      break;
   case CP_NOTIFICATION_RESUME_PLAYBACK:
      fireSipXMediaInterfaceEvent(CP_MEDIA_PLAYBACK_RESUMED, CP_MEDIA_CAUSE_NORMAL, (CP_MEDIA_TYPE)media, pData1, pData2);
      break;
   case CP_NOTIFICATION_RECORDING_STARTED:
      fireSipXMediaInterfaceEvent(CP_MEDIA_RECORDING_START, CP_MEDIA_CAUSE_NORMAL, (CP_MEDIA_TYPE)media, pData1, pData2);
      break;
   case CP_NOTIFICATION_RECORDING_STOPPED:
      fireSipXMediaInterfaceEvent(CP_MEDIA_RECORDING_STOP, CP_MEDIA_CAUSE_NORMAL, (CP_MEDIA_TYPE)media, pData1, pData2);
      break;
   case CP_NOTIFICATION_FOCUS_GAINED:
      onFocusGained();
      break;
   case CP_NOTIFICATION_FOCUS_LOST:
      onFocusLost();
      break;
   default:
      assert(false);
   }

   return TRUE;
}

OsStatus XCpAbstractCall::handleAcceptConnection(const AcAcceptConnectionMsg& rMsg)
{
   SipDialog sipDialog;
   rMsg.getSipDialog(sipDialog);

   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFound = findConnection(sipDialog, ptrLock);
   if (resFound)
   {
      return ptrLock->acceptConnection(rMsg.getSendSDP() ,rMsg.getLocationHeader(),
         rMsg.getContactId(), rMsg.getTransport());
   }

   return OS_NOT_FOUND;
}

OsStatus XCpAbstractCall::handleRejectConnection(const AcRejectConnectionMsg& rMsg)
{
   SipDialog sipDialog;
   rMsg.getSipDialog(sipDialog);

   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFound = findConnection(sipDialog, ptrLock);
   if (resFound)
   {
      return ptrLock->rejectConnection();
   }

   return OS_NOT_FOUND;
}

OsStatus XCpAbstractCall::handleRedirectConnection(const AcRedirectConnectionMsg& rMsg)
{
   SipDialog sipDialog;
   rMsg.getSipDialog(sipDialog);

   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFound = findConnection(sipDialog, ptrLock);
   if (resFound)
   {
      return ptrLock->redirectConnection(rMsg.getRedirectSipUrl());
   }

   return OS_NOT_FOUND;
}

OsStatus XCpAbstractCall::handleAnswerConnection(const AcAnswerConnectionMsg& rMsg)
{
   SipDialog sipDialog;
   rMsg.getSipDialog(sipDialog);

   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFound = findConnection(sipDialog, ptrLock);
   if (resFound)
   {
      return ptrLock->answerConnection();
   }

   return OS_NOT_FOUND;
}

OsStatus XCpAbstractCall::handleTransferBlind(const AcTransferBlindMsg& rMsg)
{
   SipDialog sipDialog;
   rMsg.getSipDialog(sipDialog);
   // find connection by sip dialog
   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFound = findConnection(sipDialog, ptrLock);
   if (resFound)
   {
      return ptrLock->transferBlind(rMsg.getTransferSipUrl());
   }

   return OS_NOT_FOUND;
}

OsStatus XCpAbstractCall::handleTransferConsultative(const AcTransferConsultativeMsg& rMsg)
{
   SipDialog sourceSipDialog;
   SipDialog targetSipDialog;
   rMsg.getSourceSipDialog(sourceSipDialog);
   rMsg.getTargetSipDialog(targetSipDialog);
   // find connection by sip dialog
   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFound = findConnection(sourceSipDialog, ptrLock);
   if (resFound)
   {
      return ptrLock->transferConsultative(targetSipDialog);
   }

   return OS_NOT_FOUND;
}

OsStatus XCpAbstractCall::handleHoldConnection(const AcHoldConnectionMsg& rMsg)
{
   SipDialog sipDialog;
   rMsg.getSipDialog(sipDialog);
   // find connection by sip dialog if call-id is not null
   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFound = findConnection(sipDialog, ptrLock);
   if (resFound)
   {
      return ptrLock->holdConnection();
   }

   return OS_NOT_FOUND;
}

OsStatus XCpAbstractCall::handleUnholdConnection(const AcUnholdConnectionMsg& rMsg)
{
   SipDialog sipDialog;
   rMsg.getSipDialog(sipDialog);
   // find connection by sip dialog
   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFound = findConnection(sipDialog, ptrLock);
   if (resFound)
   {
      return ptrLock->unholdConnection();
   }

   return OS_NOT_FOUND;
}

OsStatus XCpAbstractCall::handleRenegotiateCodecs(const AcRenegotiateCodecsMsg& rMsg)
{
   SipDialog sipDialog;
   rMsg.getSipDialog(sipDialog);
   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFound = findConnection(sipDialog, ptrLock);
   if (resFound)
   {
      UtlString audioCodecs = SdpCodecFactory::getFixedAudioCodecs(rMsg.getAudioCodecs()); // add "telephone-event" if its missing
      if (doLimitCodecPreferences(audioCodecs, rMsg.getVideoCodecs()) == OS_SUCCESS)
      {
         return ptrLock->renegotiateCodecsConnection();
      }
      else
      {
         return OS_FAILED;
      }
   }

   return OS_NOT_FOUND;
}

OsStatus XCpAbstractCall::handleSendInfo(const AcSendInfoMsg& rMsg)
{
   SipDialog sipDialog;
   rMsg.getSipDialog(sipDialog);
   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFound = findConnection(sipDialog, ptrLock);
   if (resFound)
   {
      return ptrLock->sendInfo(rMsg.getContentType(), rMsg.getContent(), rMsg.getContentLength(), rMsg.getCookie());
   }

   return OS_NOT_FOUND;
}

void XCpAbstractCall::releaseMediaInterface()
{
   if (m_pMediaInterface)
   {
      // lock is not needed
      m_pMediaInterface->release();
      m_pMediaInterface = NULL;
   }
}

CpMediaInterface* XCpAbstractCall::getMediaInterface(UtlBoolean bCreateIfNull)
{
   // if called from OsServerTask only, then thread safe
   if (!m_pMediaInterface && bCreateIfNull)
   {
      m_pMediaInterface = m_rMediaInterfaceFactory.createMediaInterface(getMessageQueue(),
         &m_rDefaultSdpCodecList,
         NULL, // public IP address is ignored by media interface factory
         m_sBindIpAddress,
         m_sLocale,
         QOS_LAYER3_LOW_DELAY_IP_TOS,
         m_natTraversalConfig.m_sStunServer,
         m_natTraversalConfig.m_iStunPort,
         m_natTraversalConfig.m_iStunKeepAlivePeriodSecs,
         m_natTraversalConfig.m_sTurnServer,
         m_natTraversalConfig.m_iTurnPort,
         m_natTraversalConfig.m_sTurnUsername,
         m_natTraversalConfig.m_sTurnPassword,
         m_natTraversalConfig.m_iTurnKeepAlivePeriodSecs,
         m_natTraversalConfig.m_bEnableICE);
#ifndef DISABLE_LOCAL_AUDIO
      if (m_focusConfig == CP_FOCUS_ALWAYS)
      {
         gainFocus(FALSE); // always gain focus
      }
      else if (m_focusConfig == CP_FOCUS_IF_AVAILABLE)
      {
         gainFocus(TRUE); // only gain focus if there is no focused call
      }
#endif
   }

   return m_pMediaInterface;
}

OsMsgQ& XCpAbstractCall::getLocalQueue()
{
   return mIncomingQ;
}

OsMsgQ& XCpAbstractCall::getGlobalQueue()
{
   return m_rCallManagerQueue;
}

OsStatus XCpAbstractCall::doLimitCodecPreferences(const UtlString& sAudioCodecs,
                                                  const UtlString& sVideoCodecs)
{
   if (m_pMediaInterface)
   {
      SdpCodecList sdpCodecList;
      sdpCodecList.addCodecs(sAudioCodecs);// appends selected audio codecs
      sdpCodecList.addCodecs(sVideoCodecs);// appends selected video codecs
      sdpCodecList.bindPayloadIds();
      return m_pMediaInterface->setCodecList(sdpCodecList);
   }

   return OS_FAILED;
}

UtlString XCpAbstractCall::getRealLineIdentity(const SipMessage& sipRequest) const
{
   UtlString sLineIdentity;

   if (m_pSipLineProvider)
   {
      m_pSipLineProvider->getFullLineUrl(sipRequest, sLineIdentity);
      return sLineIdentity;
   }

   // we didn't find line, or line provider is not set, use request uri
   sipRequest.getRequestUri(&sLineIdentity);
   return sLineIdentity;
}

OsStatus XCpAbstractCall::acquire(const OsTime& rTimeout /*= OsTime::OS_INFINITY*/)
{
   return m_instanceRWMutex.acquireRead();
}

OsStatus XCpAbstractCall::tryAcquire()
{
   return m_instanceRWMutex.tryAcquireRead();
}

OsStatus XCpAbstractCall::release()
{
   return m_instanceRWMutex.releaseRead();
}

UtlBoolean XCpAbstractCall::handleSipConnectionTimer(const ScTimerMsg& timerMsg)
{
   UtlBoolean msgHandled = FALSE;

   SipDialog sipDialog;
   timerMsg.getSipDialog(sipDialog);

   OsPtrLock<XSipConnection> ptrLock; // auto pointer lock
   UtlBoolean resFind = findConnection(sipDialog, ptrLock);
   if (resFind)
   {
      msgHandled = ptrLock->handleTimerMessage(timerMsg);
   }
   else
   {
      OsSysLog::add(FAC_CP, PRI_DEBUG, "No connection was found for sip connection timer message subtype %d. Posting to call manager.\n",
         timerMsg.getMsgSubType());
      // connection was not found, send message to call manager, it might be able to find the correct call
      // this might happen after conference join/split
      getGlobalQueue().send(timerMsg);
   }

   return resFind && msgHandled;
}

UtlBoolean XCpAbstractCall::handleSipConnectionCommandMessage(const ScCommandMsg& rMsg)
{
   UtlBoolean msgHandled = FALSE;

   SipDialog sipDialog;
   rMsg.getSipDialog(sipDialog);

   OsPtrLock<XSipConnection> ptrLock; // auto pointer lock
   UtlBoolean resFind = findConnection(sipDialog, ptrLock);
   if (resFind)
   {
      msgHandled = ptrLock->handleCommandMessage(rMsg);
   }

   return resFind && msgHandled;
}

UtlBoolean XCpAbstractCall::handleSipConnectionNotificationMessage(const ScNotificationMsg& rMsg)
{
   UtlBoolean msgHandled = FALSE;

   SipDialog sipDialog;
   rMsg.getSipDialog(sipDialog);

   OsPtrLock<XSipConnection> ptrLock; // auto pointer lock
   UtlBoolean resFind = findConnection(sipDialog, ptrLock);
   if (resFind)
   {
      msgHandled = ptrLock->handleNotificationMessage(rMsg);
   }

   return resFind && msgHandled;
}

/* ============================ FUNCTIONS ================================= */

