//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <tapi/SecurityEventMsg.h>

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

SecurityEventMsg::SecurityEventMsg() : OsMsg(SECURITY_MSG, 0)
, m_event(SECURITY_UNKNOWN)
, m_eventPayload()
{

}

SecurityEventMsg::SecurityEventMsg(SIPX_SECURITY_EVENT eventType,
                                   const SipSecurityEvent& eventPayload) : OsMsg(SECURITY_MSG, 0)
, m_event(eventType)
, m_eventPayload(eventPayload)
{

}

SecurityEventMsg::~SecurityEventMsg()
{

}

OsMsg* SecurityEventMsg::createCopy(void) const
{
   return new SecurityEventMsg(m_event, m_eventPayload);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
