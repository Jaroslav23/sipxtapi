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
#include <cp/msg/Sc100RelTimerMsg.h>

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

Sc100RelTimerMsg::Sc100RelTimerMsg(const SipMessage& c100relResponse,
                                   const UtlString& sCallId,
                                   const UtlString& sLocalTag,
                                   const UtlString& sRemoteTag,
                                   UtlBoolean isFromLocal)
: ScTimerMsg(ScTimerMsg::PAYLOAD_TYPE_100REL, sCallId, sLocalTag, sRemoteTag, isFromLocal)
, m_100relResponse(c100relResponse)
{

}

Sc100RelTimerMsg::Sc100RelTimerMsg(const Sc100RelTimerMsg& rhs)
: ScTimerMsg(rhs)
, m_100relResponse(rhs.m_100relResponse)
{
}

OsMsg* Sc100RelTimerMsg::createCopy(void) const
{
   return new Sc100RelTimerMsg(*this);
}

Sc100RelTimerMsg::~Sc100RelTimerMsg()
{

}

/* ============================ MANIPULATORS ============================== */

Sc100RelTimerMsg& Sc100RelTimerMsg::operator=(const Sc100RelTimerMsg& rhs)
{
   if (this == &rhs)
   {
      return *this;
   }

   ScTimerMsg::operator=(rhs); // assign fields for parent class
   
   m_100relResponse = rhs.m_100relResponse;

   return *this;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
