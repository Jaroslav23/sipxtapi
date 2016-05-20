//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <tapi/CallStateEventMsg.h>

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

CallStateEventMsg::CallStateEventMsg() : OsMsg(CALLSTATEEVENT_MSG, 0)
, m_event(CALLSTATE_UNKNOWN)
, m_eventPayload()
{

}

CallStateEventMsg::CallStateEventMsg(SIPX_CALLSTATE_EVENT event,
                                     const CpCallStateEvent& eventPayload) : OsMsg(CALLSTATEEVENT_MSG, 0)
, m_event(event)
, m_eventPayload(eventPayload)
{

}

CallStateEventMsg::~CallStateEventMsg()
{

}

OsMsg* CallStateEventMsg::createCopy(void) const
{
   return new CallStateEventMsg(m_event, m_eventPayload);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
