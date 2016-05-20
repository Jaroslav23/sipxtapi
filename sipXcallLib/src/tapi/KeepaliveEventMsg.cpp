//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <tapi/KeepaliveEventMsg.h>

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

KeepaliveEventMsg::KeepaliveEventMsg() : OsMsg(KEEPALIVE_MSG, 0)
, m_event(KEEPALIVE_START)
, m_eventCause(KEEPALIVE_CAUSE_NORMAL)
, m_eventPayload()
{

}

KeepaliveEventMsg::KeepaliveEventMsg(SIPX_KEEPALIVE_EVENT event,
                                     SIPX_KEEPALIVE_CAUSE eventCause,
                                     const OsNatKeepaliveEvent& eventPayload) : OsMsg(KEEPALIVE_MSG, 0)
, m_event(event)
, m_eventCause(eventCause)
, m_eventPayload(eventPayload)
{

}

KeepaliveEventMsg::~KeepaliveEventMsg()
{

}

OsMsg* KeepaliveEventMsg::createCopy(void) const
{
   return new KeepaliveEventMsg(m_event, m_eventCause, m_eventPayload);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

