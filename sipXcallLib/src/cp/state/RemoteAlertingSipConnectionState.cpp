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
#include <cp/state/SipResponseTransitionMemory.h>
#include <cp/state/RemoteAlertingSipConnectionState.h>
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

RemoteAlertingSipConnectionState::RemoteAlertingSipConnectionState(SipConnectionStateContext& rStateContext,
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

RemoteAlertingSipConnectionState::RemoteAlertingSipConnectionState(const BaseSipConnectionState& rhs)
: BaseSipConnectionState(rhs)
{

}

RemoteAlertingSipConnectionState::~RemoteAlertingSipConnectionState()
{

}

/* ============================ MANIPULATORS ============================== */

void RemoteAlertingSipConnectionState::handleStateEntry(StateEnum previousState, const StateTransitionMemory* pTransitionMemory)
{
   StateTransitionEventDispatcher eventDispatcher(m_rSipConnectionEventSink, pTransitionMemory);
   eventDispatcher.dispatchEvent(getCurrentState());

   notifyConnectionStateObservers();

   OsSysLog::add(FAC_CP, PRI_DEBUG, "Entry remote alerting connection state from state: %d, sip call-id: %s\r\n",
      (int)previousState, getCallId().data());
}

void RemoteAlertingSipConnectionState::handleStateExit(StateEnum nextState, const StateTransitionMemory* pTransitionMemory)
{

}

SipConnectionStateTransition* RemoteAlertingSipConnectionState::dropConnection(OsStatus& result)
{
   // we are caller. We received 180 ringing, but not 200 OK yet
   // to drop call, send CANCEL
   return doCancelConnection(result);
}

SipConnectionStateTransition* RemoteAlertingSipConnectionState::handleSipMessageEvent(const SipMessageEvent& rEvent)
{
   // handle event here

   // as a last resort, let parent handle event
   return BaseSipConnectionState::handleSipMessageEvent(rEvent);
}

SipConnectionStateTransition* RemoteAlertingSipConnectionState::processInviteResponse(const SipMessage& sipMessage)
{
   // first let parent handle response
   SipConnectionStateTransition* pTransition = BaseSipConnectionState::processInviteResponse(sipMessage);
   if (pTransition)
   {
      return pTransition;
   }

   int cseqNum;
   UtlString cseqMethod;
   sipMessage.getCSeqField(&cseqNum, &cseqMethod);
   int responseCode = sipMessage.getResponseStatusCode();
   UtlString responseText;
   sipMessage.getResponseStatusText(&responseText);

   switch (responseCode)
   {
   case SIP_OK_CODE:
   case SIP_ACCEPTED_CODE:
      {
         // send ACK to 200 OK, and progress to established state
         return handleInvite2xxResponse(sipMessage);
      }
   case SIP_RINGING_CODE:
   case SIP_CALL_BEING_FORWARDED_CODE:
   case SIP_SESSION_PROGRESS_CODE:
   case SIP_QUEUED_CODE:
      {
         return processProvisionalInviteResponse(sipMessage);
      }
   case SIP_ALTERNATIVE_SERVICE_CODE:
      {
         // proceed to disconnected state
         SipResponseTransitionMemory memory(responseCode, responseText);
         return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, &memory);
      }
   case SIP_MULTI_CHOICE_CODE:
   case SIP_PERMANENT_MOVE_CODE:
   case SIP_TEMPORARY_MOVE_CODE:
   case SIP_USE_PROXY_CODE:
      {
         getClientTransactionManager().endTransaction(cseqMethod, cseqNum); // end INVITE transaction so that new one can be started
         return handleInviteRedirectResponse(sipMessage);
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

SipConnectionStateTransition* RemoteAlertingSipConnectionState::getTransition(ISipConnectionState::StateEnum nextState,
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
