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
#include <cp/msg/ScByeRetryTimerMsg.h>

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

ScByeRetryTimerMsg::ScByeRetryTimerMsg(const UtlString& sCallId,
                                       const UtlString& sLocalTag,
                                       const UtlString& sRemoteTag,
                                       UtlBoolean isFromLocal)
: ScTimerMsg(ScTimerMsg::PAYLOAD_TYPE_BYE_RETRY, sCallId, sLocalTag, sRemoteTag, isFromLocal)
{

}

ScByeRetryTimerMsg::ScByeRetryTimerMsg(const ScByeRetryTimerMsg& rhs)
: ScTimerMsg(rhs)
{
}

OsMsg* ScByeRetryTimerMsg::createCopy(void) const
{
   return new ScByeRetryTimerMsg(*this);
}

ScByeRetryTimerMsg::~ScByeRetryTimerMsg()
{

}

/* ============================ MANIPULATORS ============================== */

ScByeRetryTimerMsg& ScByeRetryTimerMsg::operator=(const ScByeRetryTimerMsg& rhs)
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
