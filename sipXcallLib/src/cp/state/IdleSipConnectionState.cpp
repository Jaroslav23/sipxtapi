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
#include <cp/state/IdleSipConnectionState.h>
#include <cp/state/UnknownSipConnectionState.h>
#include <cp/state/DisconnectedSipConnectionState.h>
#include <cp/state/NewCallSipConnectionState.h>
#include <cp/state/DialingSipConnectionState.h>

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

IdleSipConnectionState::IdleSipConnectionState(SipConnectionStateContext& rStateContext,
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

IdleSipConnectionState::IdleSipConnectionState(const BaseSipConnectionState& rhs)
: BaseSipConnectionState(rhs)
{

}

IdleSipConnectionState::~IdleSipConnectionState()
{

}

/* ============================ MANIPULATORS ============================== */

void IdleSipConnectionState::handleStateEntry(StateEnum previousState, const StateTransitionMemory* pTransitionMemory)
{
   OsSysLog::add(FAC_CP, PRI_DEBUG, "Entry idle connection state from state: %d, sip call-id: %s\r\n",
      (int)previousState, getCallId().data());
}

void IdleSipConnectionState::handleStateExit(StateEnum nextState, const StateTransitionMemory* pTransitionMemory)
{

}

SipConnectionStateTransition* IdleSipConnectionState::dropConnection(OsStatus& result)
{
   result = OS_SUCCESS;
   return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, NULL);
}

SipConnectionStateTransition* IdleSipConnectionState::handleSipMessageEvent(const SipMessageEvent& rEvent)
{
   // handle event here

   // as a last resort, let parent handle event
   return BaseSipConnectionState::handleSipMessageEvent(rEvent);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

SipConnectionStateTransition* IdleSipConnectionState::getTransition(ISipConnectionState::StateEnum nextState,
                                                                    const StateTransitionMemory* pTansitionMemory) const
{
   if (this->getCurrentState() != nextState)
   {
      BaseSipConnectionState* pDestination = NULL;
      switch(nextState)
      {
      case ISipConnectionState::CONNECTION_NEWCALL:
         pDestination = new NewCallSipConnectionState(*this);
         break;
      case ISipConnectionState::CONNECTION_DIALING:
         pDestination = new DialingSipConnectionState(*this);
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
