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
#include <cp/state/OfferingSipConnectionState.h>
#include <cp/state/UnknownSipConnectionState.h>
#include <cp/state/DisconnectedSipConnectionState.h>
#include <cp/state/QueuedSipConnectionState.h>
#include <cp/state/AlertingSipConnectionState.h>
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

OfferingSipConnectionState::OfferingSipConnectionState(SipConnectionStateContext& rStateContext,
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

OfferingSipConnectionState::OfferingSipConnectionState(const BaseSipConnectionState& rhs)
: BaseSipConnectionState(rhs)
{

}

OfferingSipConnectionState::~OfferingSipConnectionState()
{

}

/* ============================ MANIPULATORS ============================== */

void OfferingSipConnectionState::handleStateEntry(StateEnum previousState, const StateTransitionMemory* pTransitionMemory)
{
   StateTransitionEventDispatcher eventDispatcher(m_rSipConnectionEventSink, pTransitionMemory);
   eventDispatcher.dispatchEvent(getCurrentState());

   notifyConnectionStateObservers();

   OsSysLog::add(FAC_CP, PRI_DEBUG, "Entry offering connection state from state: %d, sip call-id: %s\r\n",
      (int)previousState, getCallId().data());
}

void OfferingSipConnectionState::handleStateExit(StateEnum nextState, const StateTransitionMemory* pTransitionMemory)
{

}

SipConnectionStateTransition* OfferingSipConnectionState::dropConnection(OsStatus& result)
{
   // we are callee. We sent 100, but not 180 yet
   // to drop call, reject it
   return rejectConnection(result);
}

SipConnectionStateTransition* OfferingSipConnectionState::handleSipMessageEvent(const SipMessageEvent& rEvent)
{
   // handle event here

   // as a last resort, let parent handle event
   return BaseSipConnectionState::handleSipMessageEvent(rEvent);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

SipConnectionStateTransition* OfferingSipConnectionState::getTransition(ISipConnectionState::StateEnum nextState,
                                                                        const StateTransitionMemory* pTansitionMemory) const
{
   if (this->getCurrentState() != nextState)
   {
      BaseSipConnectionState* pDestination = NULL;
      switch(nextState)
      {
      case ISipConnectionState::CONNECTION_QUEUED:
         pDestination = new QueuedSipConnectionState(*this);
         break;
      case ISipConnectionState::CONNECTION_ALERTING:
         pDestination = new AlertingSipConnectionState(*this);
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
