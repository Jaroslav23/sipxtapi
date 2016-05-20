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
#include <net/SipMessage.h>
#include <cp/state/NewCallSipConnectionState.h>
#include <cp/state/UnknownSipConnectionState.h>
#include <cp/state/DisconnectedSipConnectionState.h>
#include <cp/state/OfferingSipConnectionState.h>
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

NewCallSipConnectionState::NewCallSipConnectionState(SipConnectionStateContext& rStateContext,
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

NewCallSipConnectionState::NewCallSipConnectionState(const BaseSipConnectionState& rhs)
: BaseSipConnectionState(rhs)
{

}

NewCallSipConnectionState::~NewCallSipConnectionState()
{

}

/* ============================ MANIPULATORS ============================== */

void NewCallSipConnectionState::handleStateEntry(StateEnum previousState, const StateTransitionMemory* pTransitionMemory)
{
   StateTransitionEventDispatcher eventDispatcher(m_rSipConnectionEventSink, pTransitionMemory);
   eventDispatcher.dispatchEvent(getCurrentState());

   notifyConnectionStateObservers();

   OsSysLog::add(FAC_CP, PRI_DEBUG, "Entry newcall connection state from state: %d, sip call-id: %s\r\n",
      (int)previousState, getCallId().data());
}

void NewCallSipConnectionState::handleStateExit(StateEnum nextState, const StateTransitionMemory* pTransitionMemory)
{

}

SipConnectionStateTransition* NewCallSipConnectionState::dropConnection(OsStatus& result)
{
   return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, NULL);
}

SipConnectionStateTransition* NewCallSipConnectionState::handleSipMessageEvent(const SipMessageEvent& rEvent)
{
   // handle event here

   // as a last resort, let parent handle event
   return BaseSipConnectionState::handleSipMessageEvent(rEvent);
}

SipConnectionStateTransition* NewCallSipConnectionState::processInviteRequest(const SipMessage& sipMessage)
{
   // SIP dialog is already initialized with SIP message, via XSipConnection constructor, when it is created in
   // XCpCall::createSipConnection

   // don't call superclass method
   // here we process the initial INVITE
   setLastReceivedInvite(sipMessage); // remember the INVITE
   m_rStateContext.m_sBindIpAddress = sipMessage.getLocalIp(); // override bind IP for inbound calls
   // Invite transaction was already started automatically
   // 100 Trying is sent by SipUserAgent

   updateRemoteCapabilities(sipMessage);
   trackTransactionRequest(sipMessage);

   progressToEarlyEstablishedDialog();

   m_rStateContext.m_sessionTimerProperties.reset(TRUE);

   int inviteExpiresSeconds;
   if (sipMessage.getExpiresField(&inviteExpiresSeconds) && inviteExpiresSeconds > 0)
   {
      int seqNum;
      sipMessage.getCSeqField(&seqNum, NULL);
      startInviteExpirationTimer(inviteExpiresSeconds, seqNum, FALSE); // it will check again in m_inviteExpiresSeconds, if INVITE is finished
   }

   // automatically transition to offering state
   return getTransition(ISipConnectionState::CONNECTION_OFFERING, NULL);
}

SipConnectionStateTransition* NewCallSipConnectionState::getTransition(ISipConnectionState::StateEnum nextState,
                                                                       const StateTransitionMemory* pTansitionMemory) const
{
   if (this->getCurrentState() != nextState)
   {
      BaseSipConnectionState* pDestination = NULL;
      switch(nextState)
      {
      case ISipConnectionState::CONNECTION_OFFERING:
         pDestination = new OfferingSipConnectionState(*this);
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

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void NewCallSipConnectionState::progressToEarlyEstablishedDialog()
{
   if (m_rStateContext.m_pLastReceivedInvite)
   {
      UtlString toField;
      m_rStateContext.m_pLastReceivedInvite->getToField(&toField);
      Url toFieldUrl(toField);
      toFieldUrl.setFieldParameter("tag", m_rStateContext.m_sipTagGenerator.getNewTag()); // generate new tag

      // generate to tag for sip dialog
      {
         OsWriteLock lock(m_rStateContext);
         m_rStateContext.m_sipDialog.setLocalField(toFieldUrl); // override local field, so that it has a tag
      }
   }
}

/* ============================ FUNCTIONS ================================= */
