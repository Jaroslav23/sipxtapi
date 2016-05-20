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
#include <cp/state/DisconnectedSipConnectionState.h>
#include <cp/state/UnknownSipConnectionState.h>
#include <cp/state/StateTransitionEventDispatcher.h>

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

DisconnectedSipConnectionState::DisconnectedSipConnectionState(SipConnectionStateContext& rStateContext,
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

DisconnectedSipConnectionState::DisconnectedSipConnectionState(const BaseSipConnectionState& rhs)
: BaseSipConnectionState(rhs)
{
}

DisconnectedSipConnectionState::~DisconnectedSipConnectionState()
{

}

/* ============================ MANIPULATORS ============================== */

void DisconnectedSipConnectionState::handleStateEntry(StateEnum previousState, const StateTransitionMemory* pTransitionMemory)
{
   notifyConnectionStateObservers();

   // if we are in the middle of transfer, also terminate implicit subscription
   if (m_rStateContext.m_localEntityType == SipConnectionStateContext::ENTITY_TRANSFER_CONTROLLER &&
      m_rStateContext.m_referInSubscriptionActive)
   {
      terminateInReferSubscription();
   }
   else if (m_rStateContext.m_localEntityType == SipConnectionStateContext::ENTITY_TRANSFEREE &&
      m_rStateContext.m_referOutSubscriptionActive)
   {
      terminateOutReferSubscription(); // implicit REFER subscription is still active
   }
   
   terminateSipDialog();
   deleteMediaConnection();
   deleteAllTimers();

   StateTransitionEventDispatcher eventDispatcher(m_rSipConnectionEventSink, pTransitionMemory);
   eventDispatcher.dispatchEvent(getCurrentState());

   m_rStateContext.m_bRedirecting = FALSE;
   m_rStateContext.m_redirectContactList.destroyAll();

   OsSysLog::add(FAC_CP, PRI_DEBUG, "Entry disconnected connection state from state: %d, sip call-id: %s\r\n",
      (int)previousState, getCallId().data());

   requestConnectionDestruction(); // request connection deletion
}

void DisconnectedSipConnectionState::handleStateExit(StateEnum nextState, const StateTransitionMemory* pTransitionMemory)
{
}

SipConnectionStateTransition* DisconnectedSipConnectionState::dropConnection(OsStatus& result)
{
   result = OS_SUCCESS;
   return NULL;
}

SipConnectionStateTransition* DisconnectedSipConnectionState::handleSipMessageEvent(const SipMessageEvent& rEvent)
{
   // handle event here

   // as a last resort, let parent handle event
   return BaseSipConnectionState::handleSipMessageEvent(rEvent);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

SipConnectionStateTransition* DisconnectedSipConnectionState::getTransition(ISipConnectionState::StateEnum nextState,
                                                                            const StateTransitionMemory* pTansitionMemory) const
{
   if (this->getCurrentState() != nextState)
   {
      BaseSipConnectionState* pDestination = NULL;
      switch(nextState)
      {
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

void DisconnectedSipConnectionState::terminateInReferSubscription()
{
   if (isMethodAllowed(SIP_SUBSCRIBE_METHOD))
   {
      // construct sip message
      SipMessage sipUnsubscribe;
      int seqNum = getNextLocalCSeq();
      prepareSipRequest(sipUnsubscribe, SIP_SUBSCRIBE_METHOD, seqNum);
      sipUnsubscribe.setExpiresField(0); // unsubscribe
      sipUnsubscribe.setEventField(SIP_EVENT_REFER, m_rStateContext.m_subscriptionId);
      sipUnsubscribe.setAcceptField(CONTENT_TYPE_MESSAGE_SIPFRAG);
      // send message
      sendMessage(sipUnsubscribe);
   }

   m_rStateContext.m_localEntityType = SipConnectionStateContext::ENTITY_NORMAL;
   m_rStateContext.m_referInSubscriptionActive = FALSE;
   // we don't care about response code, response will be ignored
   delete m_rStateContext.m_pLastSentRefer;
   m_rStateContext.m_pLastSentRefer = NULL;
}

void DisconnectedSipConnectionState::terminateOutReferSubscription()
{
   if (isMethodAllowed(SIP_NOTIFY_METHOD))
   {
      // construct sip message
      SipMessage sipNotify;
      int seqNum = getNextLocalCSeq();
      prepareSipRequest(sipNotify, SIP_NOTIFY_METHOD, seqNum);
      // we don't use any body, since we have nothing to send
      sipNotify.setEventField(SIP_EVENT_REFER);
      sipNotify.setSubscriptionState(SIP_SUBSCRIPTION_TERMINATED, "noresource"); // terminate subscription
      // send message
      sendMessage(sipNotify);
   }

   m_rStateContext.m_localEntityType = SipConnectionStateContext::ENTITY_NORMAL;
   m_rStateContext.m_inboundReferResponse = SipConnectionStateContext::REFER_NO_RESPONSE;
   m_rStateContext.m_referOutSubscriptionActive = FALSE;
   // we don't care about response code, response will be ignored
   delete m_rStateContext.m_pLastReceivedRefer;
   m_rStateContext.m_pLastReceivedRefer = NULL;
}

/* ============================ FUNCTIONS ================================= */
