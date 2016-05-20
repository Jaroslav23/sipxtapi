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
#include <cp/msg/ScDisconnectTimerMsg.h>

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

ScDisconnectTimerMsg::ScDisconnectTimerMsg(DisconnectReason reason,
                                           const UtlString& sCallId,
                                           const UtlString& sLocalTag,
                                           const UtlString& sRemoteTag,
                                           UtlBoolean isFromLocal)
: ScTimerMsg(ScTimerMsg::PAYLOAD_TYPE_DISCONNECT, sCallId, sLocalTag, sRemoteTag, isFromLocal)
, m_reason(reason)
{

}

ScDisconnectTimerMsg::ScDisconnectTimerMsg(const ScDisconnectTimerMsg& rhs)
: ScTimerMsg(rhs)
, m_reason(rhs.m_reason)
{
}

OsMsg* ScDisconnectTimerMsg::createCopy(void) const
{
   return new ScDisconnectTimerMsg(*this);
}

ScDisconnectTimerMsg::~ScDisconnectTimerMsg()
{

}

/* ============================ MANIPULATORS ============================== */

ScDisconnectTimerMsg& ScDisconnectTimerMsg::operator=(const ScDisconnectTimerMsg& rhs)
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
