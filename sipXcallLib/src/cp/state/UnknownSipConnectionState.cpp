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

UnknownSipConnectionState::UnknownSipConnectionState(SipConnectionStateContext& rStateContext,
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

UnknownSipConnectionState::UnknownSipConnectionState(const BaseSipConnectionState& rhs)
: BaseSipConnectionState(rhs)
{

}

UnknownSipConnectionState::~UnknownSipConnectionState()
{

}

/* ============================ MANIPULATORS ============================== */

void UnknownSipConnectionState::handleStateEntry(StateEnum previousState, const StateTransitionMemory* pTransitionMemory)
{
   notifyConnectionStateObservers();

   terminateSipDialog();
   deleteMediaConnection();
   deleteAllTimers();

   StateTransitionEventDispatcher eventDispatcher(m_rSipConnectionEventSink, pTransitionMemory);
   eventDispatcher.dispatchEvent(getCurrentState());

   m_rStateContext.m_bRedirecting = FALSE;
   m_rStateContext.m_redirectContactList.destroyAll();

   OsSysLog::add(FAC_CP, PRI_WARNING, "Entry unknown connection state from state: %d, sip call-id: %s\r\n",
      (int)previousState, getCallId().data());

   requestConnectionDestruction();
}

void UnknownSipConnectionState::handleStateExit(StateEnum nextState, const StateTransitionMemory* pTransitionMemory)
{

}

SipConnectionStateTransition* UnknownSipConnectionState::dropConnection(OsStatus& result)
{
   result = OS_SUCCESS;
   return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, NULL);
}

SipConnectionStateTransition* UnknownSipConnectionState::handleSipMessageEvent(const SipMessageEvent& rEvent)
{
   // handle event here

   // as a last resort, let parent handle event
   return BaseSipConnectionState::handleSipMessageEvent(rEvent);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

SipConnectionStateTransition* UnknownSipConnectionState::getTransition(ISipConnectionState::StateEnum nextState,
                                                                       const StateTransitionMemory* pTansitionMemory) const
{
   if (this->getCurrentState() != nextState)
   {
      BaseSipConnectionState* pDestination = NULL;
      switch(nextState)
      {
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
