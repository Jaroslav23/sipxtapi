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
#include <os/OsSysLog.h>
#include <os/OsWriteLock.h>
#include <net/SipMessageEvent.h>
#include <net/SipUserAgent.h>
#include <cp/CpMediaInterfaceProvider.h>
#include <cp/CpMessageQueueProvider.h>
#include <cp/state/SipConnectionStateMachine.h>
#include <cp/state/IdleSipConnectionState.h>
#include <cp/state/SipConnectionStateObserver.h>
#include <cp/state/SipConnectionStateTransition.h>
#include <cp/state/DialingSipConnectionState.h>
#include <cp/state/NewCallSipConnectionState.h>
#include <cp/state/DisconnectedSipConnectionState.h>
#include <cp/state/GeneralTransitionMemory.h>
#include <cp/XCpCallControl.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

SipConnectionStateMachine::SipConnectionStateMachine(SipUserAgent& rSipUserAgent,
                                                     XCpCallControl& rCallControl,
                                                     const UtlString& sBindlIpAddress,
                                                     CpMediaInterfaceProvider* pMediaInterfaceProvider,
                                                     CpMessageQueueProvider* pMessageQueueProvider,
                                                     XSipConnectionEventSink& rSipConnectionEventSink,
                                                     const CpNatTraversalConfig& natTraversalConfig)
: m_rStateContext()
, m_pSipConnectionState(NULL)
, m_pStateObserver(NULL)
, m_rSipUserAgent(rSipUserAgent)
, m_rCallControl(rCallControl)
, m_pMediaInterfaceProvider(pMediaInterfaceProvider)
, m_pMessageQueueProvider(pMessageQueueProvider)
, m_rSipConnectionEventSink(rSipConnectionEventSink)
, m_natTraversalConfig(natTraversalConfig)
{
   m_rStateContext.m_sBindIpAddress = sBindlIpAddress;
   m_rStateContext.m_sdpNegotiation.setSecurity(m_rStateContext.m_pSecurity); // wire security into sdp negotiation

   // deleted in handleStateTransition if unsuccessful
   BaseSipConnectionState* pSipConnectionState = new IdleSipConnectionState(m_rStateContext, m_rSipUserAgent,
      m_rCallControl, m_pMediaInterfaceProvider, m_pMessageQueueProvider, m_rSipConnectionEventSink, m_natTraversalConfig);
   SipConnectionStateTransition transition(m_pSipConnectionState, pSipConnectionState);

   handleStateTransition(transition);
}

SipConnectionStateMachine::~SipConnectionStateMachine()
{
   SipConnectionStateTransition transition(m_pSipConnectionState, NULL);
   handleStateTransition(transition);
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean SipConnectionStateMachine::handleSipMessageEvent(const SipMessageEvent& rEvent)
{
   if (getCurrentState() == ISipConnectionState::CONNECTION_IDLE)
   {
      const SipMessage* pSipMessage = rEvent.getMessage();
      if (pSipMessage && pSipMessage->isInviteRequest())
      {
         // we must switch state to newcall
         SipDialog sipDialog(pSipMessage); // construct new sip dialog
         {
            OsWriteLock lock(m_rStateContext);
            m_rStateContext.m_sipDialog = sipDialog; // save sip dialog
         }
         CP_CALLSTATE_CAUSE callstateCause = CP_CALLSTATE_CAUSE_NORMAL;
         UtlString originalSessionCallId;

         // investigate replaces header - used for consultative call transfer
         UtlString callId;
         UtlString fromTag;
         UtlString toTag;
         if (pSipMessage->getReplacesData(callId, toTag, fromTag))
         {
            // replaces header present, check if call really exists
            SipDialog sipDialog(callId, toTag, fromTag, FALSE);
            if (m_rCallControl.isCallEstablished(sipDialog))
            {
               // referenced call is established, remember referenced SipDialog
               m_rStateContext.m_bDropReferencedCall = TRUE;
               m_rStateContext.m_referencedSipDialog = sipDialog; // save referenced sip dialog
               callstateCause = CP_CALLSTATE_CAUSE_TRANSFERRED;
               originalSessionCallId = callId;
            }
            else
            {
               // referenced call doesn't exist
               SipMessage sipResponse;
               sipResponse.setRequestBadRequest(pSipMessage);
               m_rSipUserAgent.send(sipResponse); // send error response
               m_rStateContext.m_bSupressCallEvents = TRUE; // don't fire DISCONNECTED event
               // transition to disconnected state silently
               BaseSipConnectionState* pSipConnectionState = new DisconnectedSipConnectionState(m_rStateContext, m_rSipUserAgent,
                  m_rCallControl, m_pMediaInterfaceProvider, m_pMessageQueueProvider, m_rSipConnectionEventSink, m_natTraversalConfig);
               SipConnectionStateTransition transition(m_pSipConnectionState, pSipConnectionState);
               handleStateTransition(transition);
               return TRUE;
            }
         }

         // switch to newcall
         // deleted in doHandleStateTransition if unsuccessful
         BaseSipConnectionState* pSipConnectionState = new NewCallSipConnectionState(m_rStateContext, m_rSipUserAgent,
            m_rCallControl, m_pMediaInterfaceProvider, m_pMessageQueueProvider, m_rSipConnectionEventSink, m_natTraversalConfig);
         GeneralTransitionMemory* pMemory = new GeneralTransitionMemory(callstateCause, 0, NULL, originalSessionCallId);
         SipConnectionStateTransition transition(m_pSipConnectionState, pSipConnectionState, pMemory);
         handleStateTransition(transition);
      }
   }

   if (m_pSipConnectionState)
   {
      handleStateTransition(m_pSipConnectionState->handleSipMessageEvent(rEvent));
      return TRUE;
   }

   return FALSE;
}

OsStatus SipConnectionStateMachine::connect(const UtlString& sipCallId,
                                            const UtlString& localTag,
                                            const UtlString& toAddress,
                                            const UtlString& fromAddress,
                                            const UtlString& locationHeader,
                                            CP_CONTACT_ID contactId,
                                            SIP_TRANSPORT_TYPE transport,
                                            const UtlString& replacesField,
                                            CP_CALLSTATE_CAUSE callstateCause,
                                            const SipDialog* pCallbackSipDialog)
{
   OsStatus result = OS_FAILED;

   if (getCurrentState() == ISipConnectionState::CONNECTION_IDLE)
   {
      // switch to dialing
      // deleted in doHandleStateTransition if unsuccessful
      BaseSipConnectionState* pSipConnectionState = new DialingSipConnectionState(m_rStateContext, m_rSipUserAgent,
         m_rCallControl, m_pMediaInterfaceProvider, m_pMessageQueueProvider, m_rSipConnectionEventSink, m_natTraversalConfig);
      GeneralTransitionMemory* pMemory = new GeneralTransitionMemory(callstateCause);
      SipConnectionStateTransition transition(m_pSipConnectionState, pSipConnectionState, pMemory);
      handleStateTransition(transition);
   }

   if (pCallbackSipDialog)
   {
      // also subscribe for connection state notifications
      m_rStateContext.m_notificationRegister.subscribe(CP_NOTIFICATION_CONNECTION_STATE, *pCallbackSipDialog);
   }

   // now let state handle request
   if (m_pSipConnectionState)
   {
      handleStateTransition(m_pSipConnectionState->connect(result, sipCallId, localTag, toAddress, fromAddress,
         locationHeader, contactId, transport, replacesField));
   }

   return result;
}

OsStatus SipConnectionStateMachine::startRtpRedirect(const UtlString& slaveAbstractCallId, const SipDialog& slaveSipDialog)
{
   OsStatus result = OS_FAILED;

   // now let state handle request
   if (m_pSipConnectionState)
   {
      handleStateTransition(m_pSipConnectionState->startRtpRedirect(result, slaveAbstractCallId, slaveSipDialog));
   }

   return result;
}

OsStatus SipConnectionStateMachine::stopRtpRedirect()
{
   OsStatus result = OS_FAILED;

   // now let state handle request
   if (m_pSipConnectionState)
   {
      handleStateTransition(m_pSipConnectionState->stopRtpRedirect(result));
   }

   return result;
}

OsStatus SipConnectionStateMachine::acceptConnection(UtlBoolean bSendSDP,
                                                     const UtlString& locationHeader,
                                                     CP_CONTACT_ID contactId,
                                                     SIP_TRANSPORT_TYPE transport)
{
   OsStatus result = OS_FAILED;

   // now let state handle request
   if (m_pSipConnectionState)
   {
      handleStateTransition(m_pSipConnectionState->acceptConnection(result, bSendSDP, locationHeader,
         contactId, transport));
   }

   return result;
}

OsStatus SipConnectionStateMachine::rejectConnection()
{
   OsStatus result = OS_FAILED;

   // now let state handle request
   if (m_pSipConnectionState)
   {
      handleStateTransition(m_pSipConnectionState->rejectConnection(result));
   }

   return result;
}

OsStatus SipConnectionStateMachine::redirectConnection(const UtlString& sRedirectSipUrl)
{
   OsStatus result = OS_FAILED;

   // now let state handle request
   if (m_pSipConnectionState)
   {
      handleStateTransition(m_pSipConnectionState->redirectConnection(result, sRedirectSipUrl));
   }

   return result;
}

OsStatus SipConnectionStateMachine::answerConnection()
{
   OsStatus result = OS_FAILED;

   // now let state handle request
   if (m_pSipConnectionState)
   {
      handleStateTransition(m_pSipConnectionState->answerConnection(result));
   }

   return result;
}

OsStatus SipConnectionStateMachine::acceptTransfer()
{
   OsStatus result = OS_FAILED;

   // now let state handle request
   if (m_pSipConnectionState)
   {
      handleStateTransition(m_pSipConnectionState->acceptTransfer(result));
   }

   return result;
}

OsStatus SipConnectionStateMachine::rejectTransfer()
{
   OsStatus result = OS_FAILED;

   // now let state handle request
   if (m_pSipConnectionState)
   {
      handleStateTransition(m_pSipConnectionState->rejectTransfer(result));
   }

   return result;
}

OsStatus SipConnectionStateMachine::dropConnection()
{
   OsStatus result = OS_FAILED;

   if (m_pSipConnectionState)
   {
      handleStateTransition(m_pSipConnectionState->dropConnection(result));
   }

   return result;
}

OsStatus SipConnectionStateMachine::transferBlind(const UtlString& sTransferSipUrl)
{
   OsStatus result = OS_FAILED;

   if (m_pSipConnectionState)
   {
      handleStateTransition(m_pSipConnectionState->transferBlind(result, sTransferSipUrl));
   }

   return result;
}

OsStatus SipConnectionStateMachine::transferConsultative(const SipDialog& targetSipDialog)
{
   OsStatus result = OS_FAILED;

   if (m_pSipConnectionState)
   {
      handleStateTransition(m_pSipConnectionState->transferConsultative(result, targetSipDialog));
   }

   return result;
}

OsStatus SipConnectionStateMachine::holdConnection()
{
   OsStatus result = OS_FAILED;

   if (m_pSipConnectionState)
   {
      handleStateTransition(m_pSipConnectionState->holdConnection(result));
   }

   return result;
}

OsStatus SipConnectionStateMachine::unholdConnection()
{
   OsStatus result = OS_FAILED;

   if (m_pSipConnectionState)
   {
      handleStateTransition(m_pSipConnectionState->unholdConnection(result));
   }

   return result;
}

OsStatus SipConnectionStateMachine::renegotiateCodecsConnection()
{
   OsStatus result = OS_FAILED;

   if (m_pSipConnectionState)
   {
      handleStateTransition(m_pSipConnectionState->renegotiateCodecsConnection(result));
   }

   return result;
}

OsStatus SipConnectionStateMachine::sendInfo(const UtlString& sContentType,
                                             const char* pContent,
                                             const size_t nContentLength,
                                             void* pCookie)
{
   OsStatus result = OS_FAILED;

   if (m_pSipConnectionState)
   {
      handleStateTransition(m_pSipConnectionState->sendInfo(result, sContentType, pContent, nContentLength, pCookie));
   }

   return result;
}

OsStatus SipConnectionStateMachine::terminateMediaConnection()
{
   OsStatus result = OS_FAILED;

   if (m_pSipConnectionState)
   {
      handleStateTransition(m_pSipConnectionState->terminateMediaConnection(result));
   }

   return result;
}

OsStatus SipConnectionStateMachine::subscribe(CP_NOTIFICATION_TYPE notificationType,
                                              const SipDialog& callbackSipDialog)
{
   return m_rStateContext.m_notificationRegister.subscribe(notificationType, callbackSipDialog);
}

OsStatus SipConnectionStateMachine::unsubscribe(CP_NOTIFICATION_TYPE notificationType,
                                                const SipDialog& callbackSipDialog)
{
   return m_rStateContext.m_notificationRegister.unsubscribe(notificationType, callbackSipDialog);
}

UtlBoolean SipConnectionStateMachine::handleTimerMessage(const ScTimerMsg& timerMsg)
{
   if (m_pSipConnectionState)
   {
      handleStateTransition(m_pSipConnectionState->handleTimerMessage(timerMsg));
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}

UtlBoolean SipConnectionStateMachine::handleCommandMessage(const ScCommandMsg& rMsg)
{
   if (m_pSipConnectionState)
   {
      handleStateTransition(m_pSipConnectionState->handleCommandMessage(rMsg));
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}

UtlBoolean SipConnectionStateMachine::handleNotificationMessage(const ScNotificationMsg& rMsg)
{
   if (m_pSipConnectionState)
   {
      handleStateTransition(m_pSipConnectionState->handleNotificationMessage(rMsg));
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}

void SipConnectionStateMachine::configureSessionTimer(int sessionExpiration,
                                                      CP_SESSION_TIMER_REFRESH sessionTimerRefresh)
{
   m_rStateContext.m_sessionTimerProperties.setInitialSessionExpires(sessionExpiration);
   m_rStateContext.m_sessionTimerProperties.setInitialRefresher(sessionTimerRefresh);
}

void SipConnectionStateMachine::configureUpdate(CP_SIP_UPDATE_CONFIG updateSetting)
{
   m_rStateContext.m_updateSetting = updateSetting;
}

void SipConnectionStateMachine::configure100rel(CP_100REL_CONFIG c100relSetting)
{
   m_rStateContext.m_100relSetting = c100relSetting;
}

void SipConnectionStateMachine::configureInviteExpiration(int inviteExpiresSeconds)
{
   if (inviteExpiresSeconds > CP_MINIMUM_RINGING_EXPIRE_SECONDS)
   {
      m_rStateContext.m_inviteExpiresSeconds = inviteExpiresSeconds;
   }
   else
   {
      m_rStateContext.m_inviteExpiresSeconds = CP_MINIMUM_RINGING_EXPIRE_SECONDS;
   }
}

void SipConnectionStateMachine::configureSdpOfferingMode(CP_SDP_OFFERING_MODE sdpOfferingMode)
{
   m_rStateContext.m_sdpNegotiation.setSdpOfferingMode((CpSdpNegotiation::SdpOfferingMode)sdpOfferingMode);
}

/* ============================ ACCESSORS ================================= */

ISipConnectionState::StateEnum SipConnectionStateMachine::getCurrentState() const
{
   if (m_pSipConnectionState)
   {
      return m_pSipConnectionState->getCurrentState();
   }
   else
   {
      return ISipConnectionState::CONNECTION_UNKNOWN;
   }
}

XSipConnectionContext& SipConnectionStateMachine::getSipConnectionContext() const
{
   return m_rStateContext;
}

void SipConnectionStateMachine::setRealLineIdentity(const UtlString& sFullLineUrl)
{
   m_rStateContext.m_realLineIdentity.fromString(sFullLineUrl, FALSE);
}

void SipConnectionStateMachine::setMessageQueueProvider(CpMessageQueueProvider* pMessageQueueProvider)
{
   m_pMessageQueueProvider = pMessageQueueProvider;
   if (m_pSipConnectionState)
   {
      m_pSipConnectionState->setMessageQueueProvider(pMessageQueueProvider);
   }
}

void SipConnectionStateMachine::setMediaInterfaceProvider(CpMediaInterfaceProvider* pMediaInterfaceProvider)
{
   m_pMediaInterfaceProvider = pMediaInterfaceProvider;
   if (m_pSipConnectionState)
   {
      m_pSipConnectionState->setMediaInterfaceProvider(pMediaInterfaceProvider);
   }
}

/* ============================ INQUIRY =================================== */

SipConnectionStateContext::MediaSessionState SipConnectionStateMachine::getMediaSessionState() const
{
   return m_rStateContext.m_mediaSessionState;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void SipConnectionStateMachine::handleStateTransition(SipConnectionStateTransition* pStateTransition)
{
   if (pStateTransition)
   {
      handleStateTransition(*pStateTransition);
      delete pStateTransition;
      pStateTransition = NULL;
   }
}

void SipConnectionStateMachine::handleStateTransition(SipConnectionStateTransition& rStateTransition)
{
   if (rStateTransition.getSource() == m_pSipConnectionState)
   {
      // transition seems to be valid
      ISipConnectionState::StateEnum previousState = ISipConnectionState::CONNECTION_UNKNOWN;
      ISipConnectionState::StateEnum nextState = ISipConnectionState::CONNECTION_UNKNOWN;
      BaseSipConnectionState* m_pDestination = rStateTransition.getDestination();

      if (m_pDestination)
      {
         nextState = m_pDestination->getCurrentState();
      }
      if (m_pSipConnectionState)
      {
         previousState = m_pSipConnectionState->getCurrentState();
         m_pSipConnectionState->handleStateExit(nextState, rStateTransition.getMemory());
         notifyStateExit(); // also notify observer
         // delete old state
         delete m_pSipConnectionState;
         m_pSipConnectionState = NULL;
      }
      if (m_pDestination)
      {
         m_pSipConnectionState = m_pDestination;
         m_pSipConnectionState->handleStateEntry(previousState, rStateTransition.getMemory());
         notifyStateEntry(); // also notify observer
      }
   }
   else
   {
      OsSysLog::add(FAC_CP, PRI_ERR, "Invalid state transition in SipConnectionStateMachine, source state mismatch.");
      // delete destination state to avoid leaks
      BaseSipConnectionState* m_pDestination = rStateTransition.getDestination();
      if (m_pDestination)
      {
         delete m_pDestination;
         m_pDestination = NULL;
      }
   }
}

void SipConnectionStateMachine::notifyStateEntry()
{
   if (m_pStateObserver && m_pSipConnectionState)
   {
      m_pStateObserver->handleStateEntry(m_pSipConnectionState->getCurrentState());
   }
}

void SipConnectionStateMachine::notifyStateExit()
{
   if (m_pStateObserver && m_pSipConnectionState)
   {
      m_pStateObserver->handleStateExit(m_pSipConnectionState->getCurrentState());
   }
}

/* ============================ FUNCTIONS ================================= */
