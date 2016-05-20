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
#include <cp/msg/ScInviteExpirationTimerMsg.h>

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

ScInviteExpirationTimerMsg::ScInviteExpirationTimerMsg(int cseqNum,
                                                       UtlBoolean bIsOutbound,
                                                       const UtlString& sCallId,
                                                       const UtlString& sLocalTag,
                                                       const UtlString& sRemoteTag,
                                                       UtlBoolean isFromLocal)
: ScTimerMsg(ScTimerMsg::PAYLOAD_TYPE_INVITE_EXPIRATION, sCallId, sLocalTag, sRemoteTag, isFromLocal)
, m_cseqNum(cseqNum)
, m_bIsOutbound(bIsOutbound)
{

}

ScInviteExpirationTimerMsg::ScInviteExpirationTimerMsg(const ScInviteExpirationTimerMsg& rhs)
: ScTimerMsg(rhs)
, m_cseqNum(rhs.m_cseqNum)
, m_bIsOutbound(rhs.m_bIsOutbound)
{
}

OsMsg* ScInviteExpirationTimerMsg::createCopy(void) const
{
   return new ScInviteExpirationTimerMsg(*this);
}

ScInviteExpirationTimerMsg::~ScInviteExpirationTimerMsg()
{

}

/* ============================ MANIPULATORS ============================== */

ScInviteExpirationTimerMsg& ScInviteExpirationTimerMsg::operator=(const ScInviteExpirationTimerMsg& rhs)
{
   if (this == &rhs)
   {
      return *this;
   }

   ScTimerMsg::operator=(rhs); // assign fields for parent class

   m_cseqNum = rhs.m_cseqNum;
   m_bIsOutbound = rhs.m_bIsOutbound;
   
   return *this;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
