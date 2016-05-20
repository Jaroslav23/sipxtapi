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
#include <cp/msg/ScSessionTimeoutTimerMsg.h>

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

ScSessionTimeoutTimerMsg::ScSessionTimeoutTimerMsg(const UtlString& sCallId,
                                                   const UtlString& sLocalTag,
                                                   const UtlString& sRemoteTag,
                                                   UtlBoolean isFromLocal)
: ScTimerMsg(ScTimerMsg::PAYLOAD_TYPE_SESSION_TIMEOUT_CHECK, sCallId, sLocalTag, sRemoteTag, isFromLocal)
{

}

ScSessionTimeoutTimerMsg::ScSessionTimeoutTimerMsg(const ScSessionTimeoutTimerMsg& rhs)
: ScTimerMsg(rhs)
{
}

OsMsg* ScSessionTimeoutTimerMsg::createCopy(void) const
{
   return new ScSessionTimeoutTimerMsg(*this);
}

ScSessionTimeoutTimerMsg::~ScSessionTimeoutTimerMsg()
{

}

/* ============================ MANIPULATORS ============================== */

ScSessionTimeoutTimerMsg& ScSessionTimeoutTimerMsg::operator=(const ScSessionTimeoutTimerMsg& rhs)
{
   if (this == &rhs)
   {
      return *this;
   }

   ScTimerMsg::operator=(rhs); // assign fields for parent class
   
   return *this;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
