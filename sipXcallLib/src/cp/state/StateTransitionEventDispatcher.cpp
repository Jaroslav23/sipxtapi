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
#include <net/SipMessageEvent.h>
#include <cp/state/StateTransitionEventDispatcher.h>
#include <cp/state/StateTransitionMemory.h>
#include <cp/state/SipResponseTransitionMemory.h>
#include <cp/state/SipEventTransitionMemory.h>
#include <cp/state/GeneralTransitionMemory.h>
#include <cp/XSipConnectionEventSink.h>

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

StateTransitionEventDispatcher::StateTransitionEventDispatcher(XSipConnectionEventSink& rSipConnectionEventSink,
                                                               const StateTransitionMemory* pMemory)
: m_rSipConnectionEventSink(rSipConnectionEventSink)
, m_pMemory(pMemory)
{

}

StateTransitionEventDispatcher::~StateTransitionEventDispatcher()
{
   m_pMemory = NULL;
}

/* ============================ MANIPULATORS ============================== */

void StateTransitionEventDispatcher::dispatchEvent(ISipConnectionState::StateEnum state) const
{
   UtlString originalSessionCallId;
   int sipResponseCode = 0;
   UtlString sipResponseText;
   CP_CALLSTATE_EVENT event = getCallEventFromState(state);
   CP_CALLSTATE_CAUSE cause = CP_CALLSTATE_CAUSE_NORMAL;

   getCallEventDetails(cause, originalSessionCallId, sipResponseCode, sipResponseText);

   m_rSipConnectionEventSink.fireSipXCallEvent(event, cause, originalSessionCallId, sipResponseCode, sipResponseText);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void StateTransitionEventDispatcher::getCallEventDetails(CP_CALLSTATE_CAUSE& cause,
                                                         UtlString& originalSessionCallId,
                                                         int& sipResponseCode,
                                                         UtlString& sipResponseText) const
{
   cause = CP_CALLSTATE_CAUSE_NORMAL;

   if (m_pMemory)
   {
      StateTransitionMemory::Type type = m_pMemory->getType();
      switch (type)
      {
      case StateTransitionMemory::SIP_RESPONSE_MEMORY:
         {
            const SipResponseTransitionMemory* pSipResponseMemory = dynamic_cast<const SipResponseTransitionMemory*>(m_pMemory);
            if (pSipResponseMemory)
            {
               sipResponseCode = pSipResponseMemory->getSipResponseCode();
               sipResponseText = pSipResponseMemory->getSipResponseText();
               cause = getCauseFromSipResponseCode(sipResponseCode);
            }
            break;
         }
      case StateTransitionMemory::SIP_MESSAGE_EVENT_MEMORY:
         {
            const SipEventTransitionMemory* pSipEventMemory = dynamic_cast<const SipEventTransitionMemory*>(m_pMemory);
            if (pSipEventMemory)
            {
               int sipMessageStatus = pSipEventMemory->getSipMessageStatus();
               if (sipMessageStatus == SipMessageEvent::TRANSPORT_ERROR)
               {
                  cause = CP_CALLSTATE_CAUSE_NETWORK;
               }
            }
            break;
         }
      case StateTransitionMemory::GENERAL_EVENT_MEMORY:
         {
            const GeneralTransitionMemory* pSipEventMemory = dynamic_cast<const GeneralTransitionMemory*>(m_pMemory);
            if (pSipEventMemory)
            {
               cause = pSipEventMemory->getCause();
               sipResponseCode = pSipEventMemory->getSipResponseCode();
               sipResponseText = pSipEventMemory->getSipResponseText();
               originalSessionCallId = pSipEventMemory->getOriginalSessionCallId();
            }
            break;
         }
      default:
         ;
      }
   }
}

CP_CALLSTATE_EVENT StateTransitionEventDispatcher::getCallEventFromState(ISipConnectionState::StateEnum state) const
{
   CP_CALLSTATE_EVENT event = CP_CALLSTATE_UNKNOWN;

   switch (state)
   {
   case ISipConnectionState::CONNECTION_IDLE:
      break; // no event
   case ISipConnectionState::CONNECTION_NEWCALL:
      event = CP_CALLSTATE_NEWCALL;
      break;
   case ISipConnectionState::CONNECTION_DIALING:
      event = CP_CALLSTATE_DIALTONE;
      break;
   case ISipConnectionState::CONNECTION_REMOTE_QUEUED:
      event = CP_CALLSTATE_REMOTE_QUEUED;
      break;
   case ISipConnectionState::CONNECTION_REMOTE_OFFERING:
      event = CP_CALLSTATE_REMOTE_OFFERING;
      break;
   case ISipConnectionState::CONNECTION_REMOTE_ALERTING:
      event = CP_CALLSTATE_REMOTE_ALERTING;
      break;
   case ISipConnectionState::CONNECTION_QUEUED:
      event = CP_CALLSTATE_QUEUED;
      break;
   case ISipConnectionState::CONNECTION_OFFERING:
      event = CP_CALLSTATE_OFFERING;
      break;
   case ISipConnectionState::CONNECTION_ALERTING:
      event = CP_CALLSTATE_ALERTING;
      break;
   case ISipConnectionState::CONNECTION_ESTABLISHED:
      event = CP_CALLSTATE_CONNECTED;
      break;
   case ISipConnectionState::CONNECTION_DISCONNECTED:
      event = CP_CALLSTATE_DISCONNECTED;
      break;
   case ISipConnectionState::CONNECTION_UNKNOWN:
   default:
      event = CP_CALLSTATE_UNKNOWN;
      break;
   }

   return event;
}

CP_CALLSTATE_CAUSE StateTransitionEventDispatcher::getCauseFromSipResponseCode(int sipResponseCode) const
{
   switch (sipResponseCode)
   {
   case SIP_BAD_ADDRESS_CODE:
   case SIP_NOT_FOUND_CODE:
      return CP_CALLSTATE_CAUSE_BAD_ADDRESS;
   case SIP_BUSY_CODE:
   case SIP_REQUEST_TERMINATED_CODE:
      return CP_CALLSTATE_CAUSE_BUSY;
   case SIP_REQUEST_TIMEOUT_CODE:
      return CP_CALLSTATE_CAUSE_NO_RESPONSE;
   case SIP_DECLINE_CODE:
   case SIP_FORBIDDEN_CODE:
      return CP_CALLSTATE_CAUSE_REQUEST_NOT_ACCEPTED;
   case SIP_REQUEST_UNDECIPHERABLE_CODE:
      return CP_CALLSTATE_CAUSE_REMOTE_SMIME_UNSUPPORTED;
   case SIP_REQUEST_NOT_ACCEPTABLE_HERE_CODE:
      return CP_CALLSTATE_CAUSE_REQUEST_NOT_ACCEPTED;
   case SIP_UNAUTHORIZED_CODE:
      return CP_CALLSTATE_CAUSE_AUTH;
   case SIP_MULTI_CHOICE_CODE:
   case SIP_PERMANENT_MOVE_CODE:
   case SIP_TEMPORARY_MOVE_CODE:
      return CP_CALLSTATE_CAUSE_REDIRECTED;
   case SIP_BAD_TRANSACTION_CODE:
      return CP_CALLSTATE_CAUSE_TRANSACTION_DOES_NOT_EXIST;
   default:
      if (sipResponseCode >= SIP_4XX_CLASS_CODE && sipResponseCode < SIP_5XX_CLASS_CODE)
      {
         return CP_CALLSTATE_CAUSE_CLIENT_ERROR;
      }
      else if (sipResponseCode >= SIP_5XX_CLASS_CODE && sipResponseCode < SIP_6XX_CLASS_CODE)
      {
         return CP_CALLSTATE_CAUSE_SERVER_ERROR;
      }
      else if (sipResponseCode >= SIP_6XX_CLASS_CODE && sipResponseCode < SIP_7XX_CLASS_CODE)
      {
         return CP_CALLSTATE_CAUSE_GLOBAL_ERROR;
      }
      ;
   }

   return CP_CALLSTATE_CAUSE_NORMAL;
}

/* ============================ FUNCTIONS ================================= */
