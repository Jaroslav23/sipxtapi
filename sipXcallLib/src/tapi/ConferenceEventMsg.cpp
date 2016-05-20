//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <tapi/ConferenceEventMsg.h>

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

ConferenceEventMsg::ConferenceEventMsg()
: OsMsg(CONFERENCEEVENT_MSG, 0)
, m_event(CONFERENCE_CREATED)
, m_eventPayload()
{

}

ConferenceEventMsg::ConferenceEventMsg(SIPX_CONFERENCE_EVENT event,
                                       const CpConferenceEvent& eventPayload)
: OsMsg(CONFERENCEEVENT_MSG, 0)
, m_event(event)
, m_eventPayload(eventPayload)
{

}

ConferenceEventMsg::~ConferenceEventMsg()
{

}

OsMsg* ConferenceEventMsg::createCopy(void) const
{
   return new ConferenceEventMsg(m_event, m_eventPayload);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
