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
#include <cp/msg/ScReInviteTimerMsg.h>

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

ScReInviteTimerMsg::ScReInviteTimerMsg(ReInviteReason reason,
                                       const UtlString& sCallId,
                                       const UtlString& sLocalTag,
                                       const UtlString& sRemoteTag,
                                       UtlBoolean isFromLocal)
: ScTimerMsg(ScTimerMsg::PAYLOAD_TYPE_REINVITE, sCallId, sLocalTag, sRemoteTag, isFromLocal)
, m_reason(reason)
{

}

ScReInviteTimerMsg::ScReInviteTimerMsg(const ScReInviteTimerMsg& rhs)
: ScTimerMsg(rhs)
, m_reason(rhs.m_reason)
{
}

OsMsg* ScReInviteTimerMsg::createCopy(void) const
{
   return new ScReInviteTimerMsg(*this);
}

ScReInviteTimerMsg::~ScReInviteTimerMsg()
{

}

/* ============================ MANIPULATORS ============================== */

ScReInviteTimerMsg& ScReInviteTimerMsg::operator=(const ScReInviteTimerMsg& rhs)
{
   if (this == &rhs)
   {
      return *this;
   }

   ScTimerMsg::operator=(rhs); // assign fields for parent class
   
   m_reason = rhs.m_reason;

   return *this;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
