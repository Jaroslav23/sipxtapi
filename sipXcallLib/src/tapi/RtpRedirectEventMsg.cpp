//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <tapi/RtpRedirectEventMsg.h>

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

RtpRedirectEventMsg::RtpRedirectEventMsg()
: OsMsg(RTPREDIRECTEVENT_MSG, 0)
, m_event(RTP_REDIRECT_REQUESTED)
, m_eventPayload()
{

}

RtpRedirectEventMsg::RtpRedirectEventMsg(SIPX_RTP_REDIRECT_EVENT event,
                                         const CpRtpRedirectEvent& eventPayload)
: OsMsg(RTPREDIRECTEVENT_MSG, 0)
, m_event(event)
, m_eventPayload(eventPayload)
{

}

RtpRedirectEventMsg::~RtpRedirectEventMsg()
{

}

OsMsg* RtpRedirectEventMsg::createCopy(void) const
{
   return new RtpRedirectEventMsg(m_event, m_eventPayload);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
