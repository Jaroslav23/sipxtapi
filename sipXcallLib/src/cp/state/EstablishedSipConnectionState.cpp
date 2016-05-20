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
#include <net/SipMessage.h>
#include <cp/state/EstablishedSipConnectionState.h>
#include <cp/state/UnknownSipConnectionState.h>
#include <cp/state/DisconnectedSipConnectionState.h>
#include <cp/state/StateTransitionEventDispatcher.h>
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

EstablishedSipConnectionState::EstablishedSipConnectionState(SipConnectionStateContext& rStateContext,
                                                             SipUserAgent& rSipUserAgent,
                                                             XCpCallControl& rCallControl,
                                                             CpMediaInterfaceProvider* pMediaInterfaceProvider,
                                                             CpMessageQueueProvider* pMessageQueueProvider,
                                                             XSipConnectionEventSink& rSipConnectionEventSink,
                                                             const CpNatTraversalConfig& natTraversalConfig)
: BaseSipConnectionState(rStateContext, rSipUserAgent, rCallControl, pMediaInterfaceProvider, pMessageQueueProvider,
                         rSipConnectionEventSink, natTraversalConfig)
{

}

EstablishedSipConnectionState::EstablishedSipConnectionState(const BaseSipConnectionState& rhs)
: BaseSipConnectionState(rhs)
{

}

EstablishedSipConnectionState::~EstablishedSipConnectionState()
{

}

/* ============================ MANIPULATORS ============================== */

void EstablishedSipConnectionState::handleStateEntry(StateEnum previousState, const StateTransitionMemory* pTransitionMemory)
{
   StateTransitionEventDispatcher eventDispatcher(m_rSipConnectionEventSink, pTransitionMemory);
   eventDispatcher.dispatchEvent(getCurrentState());

   notifyConnectionStateObservers();

   // fire held/remotely held events if media session is held
   fireMediaSessionEvents(TRUE, TRUE);

   m_rStateContext.m_bRedirecting = FALSE;
   m_rStateContext.m_redirectContactList.destroyAll();

   // if we are referencing a call, drop it
   if (m_rStateContext.m_bDropReferencedCall)
   {
      m_rCallControl.dropAbstractCallConnection(m_rStateContext.m_referencedSipDialog);
      m_rStateContext.m_bDropReferencedCall = FALSE;
      m_rStateContext.m_referencedSipDialog = SipDialog();
   }

   // set SDP offering to immediate, even if late SDP offering was configured for initial INVITE
   // the reason is, that with late SDP offering, we wouldn't be able to take call off hold
   // if stream was marked with "inactive" flag in SDP offer contained in 200 OK.
   m_rStateContext.m_sdpNegotiation.setSdpOfferingMode(CpSdpNegotiation::SDP_OFFERING_IMMEDIATE);

   OsSysLog::add(FAC_CP, PRI_DEBUG, "Entry established connection state from state: %d, sip call-id: %s\r\n",
      (int)previousState, getCallId().data());
}

void EstablishedSipConnectionState::handleStateExit(StateEnum nextState, const StateTransitionMemory* pTransitionMemory)
{

}

SipConnectionStateTransition* EstablishedSipConnectionState::dropConnection(OsStatus& result)
{
   if (!isLocalInitiatedDialog())
   {
      // inbound established call
      if (m_rStateContext.m_bAckReceived)
      {
         // we may send BYE
         return doByeConnection(result);
      }
      else
      {
         // we may not send BYE, we must wait
         startByeRetryTimer(); // we will try BYE again later
         return NULL;
      }
   }
   else
   {
      // outbound established call, we may use BYE
      return doByeConnection(result);
   }
}

SipConnectionStateTransition* EstablishedSipConnectionState::processInviteRequest(const SipMessage& sipMessage)
{
   int seqNum;
   sipMessage.getCSeqField(&seqNum, NULL);
   UtlBoolean bProtocolError = TRUE;
   UtlBoolean bSdpNegotiationStarted = FALSE;
   SipMessage sipResponse;
   SipMessage errorResponse;
   ERROR_RESPONSE_TYPE errorResponseType = ERROR_RESPONSE_488;

   CpSipTransactionManager::TransactionState inviteState = getServerTransactionManager().getTransactionState(sipMessage);

   // inbound INVITE will always be found, since its started automatically, unless its 2nd INVITE transaction
   // at time. In that case, 2nd INVITE transaction won't be started.
   if (inviteState == CpSipTransactionManager::TRANSACTION_ACTIVE)
   {
      // this is either new transaction or retransmit of re-INVITE
      UtlBoolean bIsRetransmit = m_rStateContext.m_sdpNegotiation.isInSdpNegotiation(sipMessage);

      // this is new INVITE transaction, and we are allowed to continue processing
      if (!isUpdateActive() && getClientInviteTransactionState() == CpSipTransactionManager::INVITE_INACTIVE)
      {
         bSdpNegotiationStarted = TRUE;
         UtlString sLocalContact(getLocalContactUrl());

         // prepare and send 200 OK
         sipResponse.setOkResponseData(&sipMessage, sLocalContact);
         const SdpBody* pSdpBody = sipMessage.getSdpBody(m_rStateContext.m_pSecurity);
         if (pSdpBody)
         {
            // there is SDP offer
            handleSdpOffer(sipMessage);
            // send answer in 200 OK
            if (prepareSdpAnswer(sipResponse))
            {
               if (commitMediaSessionChanges()) // SDP offer/answer complete, we may commit changes
               {
                  bProtocolError = FALSE;
               }
            }
         }
         else
         {
            // there is no SDP offer, send one in 200 OK
            if (prepareSdpOffer(sipResponse))
            {
               bProtocolError = FALSE;
            }
         }
      }
      else
      {
         errorResponseType = ERROR_RESPONSE_491;
      }
   }

   if (bProtocolError)
   {
      if (bSdpNegotiationStarted)
      {
         // only restart if we started it, don't kill negotiation in progress
         m_rStateContext.m_sdpNegotiation.resetSdpNegotiation();
      }

      prepareErrorResponse(sipMessage, errorResponse, errorResponseType);
      sendMessage(errorResponse);
   }
   else
   {
      m_rStateContext.m_bAckReceived = FALSE;
      m_rStateContext.m_i2xxInviteRetransmitCount = 0;
      setLastReceivedInvite(sipMessage);
      setLastSent2xxToInvite(sipResponse);
      prepareSessionTimerResponse(sipMessage, sipResponse); // construct session timer response
      handleSessionTimerResponse(sipResponse); // handle our own response, start session timer..
      sendMessage(sipResponse);
      start2xxRetransmitTimer();

      // 200 OK was sent, update connected identity
      if (sipMessage.isInSupportedField(SIP_FROM_CHANGE_EXTENSION))
      {
         updateSipDialogRemoteField(sipMessage);
      }
   }

   return NULL;
}

SipConnectionStateTransition* EstablishedSipConnectionState::processByeRequest(const SipMessage& sipMessage)
{
   // inbound BYE
   SipMessage sipResponse;
   sipResponse.setOkResponseData(&sipMessage, getLocalContactUrl());
   sendMessage(sipResponse);

   return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, NULL);
}

SipConnectionStateTransition* EstablishedSipConnectionState::handleSipMessageEvent(const SipMessageEvent& rEvent)
{
   // handle event here

   // as a last resort, let parent handle event
   return BaseSipConnectionState::handleSipMessageEvent(rEvent);
}

SipConnectionStateTransition* EstablishedSipConnectionState::processInviteResponse(const SipMessage& sipMessage)
{
   // first let parent handle response
   SipConnectionStateTransition* pTransition = BaseSipConnectionState::processInviteResponse(sipMessage);
   if (pTransition)
   {
      return pTransition;
   }

   int responseCode = sipMessage.getResponseStatusCode();
   UtlString responseText;
   sipMessage.getResponseStatusText(&responseText);

   switch (responseCode)
   {
   case SIP_RINGING_CODE:
   case SIP_CALL_BEING_FORWARDED_CODE:
   case SIP_SESSION_PROGRESS_CODE:
   case SIP_QUEUED_CODE:
      {
         // if established then there will be no transition
         return processProvisionalInviteResponse(sipMessage);
      }
   case SIP_OK_CODE:
   case SIP_ACCEPTED_CODE:
      {
         // send ACK to retransmitted 200 OK, or to 200 OK from re-INVITE
         return handleInvite2xxResponse(sipMessage);
      }
   default:
      ;
   }

   // no transition
   return NULL;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

SipConnectionStateTransition* EstablishedSipConnectionState::getTransition(ISipConnectionState::StateEnum nextState,
                                                                           const StateTransitionMemory* pTansitionMemory) const
{
   if (this->getCurrentState() != nextState)
   {
      BaseSipConnectionState* pDestination = NULL;
      switch(nextState)
      {
      case ISipConnectionState::CONNECTION_ESTABLISHED:
         pDestination = new EstablishedSipConnectionState(*this);
         break;
      case ISipConnectionState::CONNECTION_DISCONNECTED:
         pDestination = new DisconnectedSipConnectionState(*this);
         break;
      case ISipConnectionState::CONNECTION_UNKNOWN:
      default:
         pDestination = new UnknownSipConnectionState(*this);
         break;
      }

      return getTransitionObject(pDestination, pTansitionMemory);
   }
   else
   {
      return NULL;
   }
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
