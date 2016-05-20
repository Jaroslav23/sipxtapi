//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <tapi/MediaEventMsg.h>

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

MediaEventMsg::MediaEventMsg() : OsMsg(MEDIAEVENT_MSG, 0)
, m_eventPayload()
{

}

MediaEventMsg::MediaEventMsg(const SipXMediaEvent& eventPayload) : OsMsg(MEDIAEVENT_MSG, 0)
, m_eventPayload(eventPayload)
{

}

MediaEventMsg::~MediaEventMsg()
{

}

OsMsg* MediaEventMsg::createCopy(void) const
{
   return new MediaEventMsg(m_eventPayload);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
