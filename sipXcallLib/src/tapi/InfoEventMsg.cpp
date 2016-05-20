//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <tapi/InfoEventMsg.h>

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

InfoEventMsg::InfoEventMsg()
: OsMsg(INFO_MSG, 0)
{

}

InfoEventMsg::InfoEventMsg(const SipInfoEvent& eventPayload)
: OsMsg(INFO_MSG, 0)
, m_eventPayload(eventPayload)
{

}

InfoEventMsg::~InfoEventMsg()
{

}

OsMsg* InfoEventMsg::createCopy(void) const
{
   return new InfoEventMsg(m_eventPayload);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
