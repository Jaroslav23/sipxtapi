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
#include <cp/state/AlertingSipConnectionState.h>
#include <cp/state/UnknownSipConnectionState.h>
#include <cp/state/DisconnectedSipConnectionState.h>
#include <cp/state/EstablishedSipConnectionState.h>
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

AlertingSipConnectionState::AlertingSipConnectionState(SipConnectionStateContext& rStateContext,
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

AlertingSipConnectionState::AlertingSipConnectionState(const BaseSipConnectionState& rhs)
: BaseSipConnectionState(rhs)
{

}

AlertingSipConnectionState::~AlertingSipConnectionState()
{

}

/* ============================ MANIPULATORS ============================== */

void AlertingSipConnectionState::handleStateEntry(StateEnum previousState, const StateTransitionMemory* pTransitionMemory)
{
   StateTransitionEventDispatcher eventDispatcher(m_rSipConnectionEventSink, pTransitionMemory);
   eventDispatcher.dispatchEvent(getCurrentState());

   notifyConnectionStateObservers();

   OsSysLog::add(FAC_CP, PRI_DEBUG, "Entry alerting connection state from state: %d, sip call-id: %s\r\n",
      (int)previousState, getCallId().data());
}

void AlertingSipConnectionState::handleStateExit(StateEnum nextState, const StateTransitionMemory* pTransitionMemory)
{

}

SipConnectionStateTransition* AlertingSipConnectionState::dropConnection(OsStatus& result)
{
   // we are callee. We sent 180, but not 200 OK yet
   // to drop call, reject it
   return rejectConnection(result);
}

SipConnectionStateTransition* AlertingSipConnectionState::handleSipMessageEvent(const SipMessageEvent& rEvent)
{
   // handle event here

   // as a last resort, let parent handle event
   return BaseSipConnectionState::handleSipMessageEvent(rEvent);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

SipConnectionStateTransition* AlertingSipConnectionState::getTransition(ISipConnectionState::StateEnum nextState,
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
