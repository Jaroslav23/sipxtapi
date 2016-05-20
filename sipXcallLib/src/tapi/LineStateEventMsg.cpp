//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <tapi/LineStateEventMsg.h>

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

LineStateEventMsg::LineStateEventMsg() : OsMsg(LINESTATEEVENT_MSG, 0)
, m_event(LINESTATE_UNKNOWN)
, m_eventPayload()
{

}

LineStateEventMsg::LineStateEventMsg(SIPX_LINESTATE_EVENT event,
                                     const SipLineStateEvent& eventPayload) : OsMsg(LINESTATEEVENT_MSG, 0)
, m_event(event)
, m_eventPayload(eventPayload)
{

}

LineStateEventMsg::~LineStateEventMsg()
{

}

OsMsg* LineStateEventMsg::createCopy(void) const
{
   return new LineStateEventMsg(m_event, m_eventPayload);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
