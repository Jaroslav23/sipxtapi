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
#include <cp/msg/ScDelayedAnswerTimerMsg.h>

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

ScDelayedAnswerTimerMsg::ScDelayedAnswerTimerMsg(const UtlString& sCallId,
                                                 const UtlString& sLocalTag,
                                                 const UtlString& sRemoteTag,
                                                 UtlBoolean isFromLocal)
: ScTimerMsg(ScTimerMsg::PAYLOAD_TYPE_DELAYED_ANSWER, sCallId, sLocalTag, sRemoteTag, isFromLocal)
{

}

ScDelayedAnswerTimerMsg::ScDelayedAnswerTimerMsg(const ScDelayedAnswerTimerMsg& rhs)
: ScTimerMsg(rhs)
{
}

OsMsg* ScDelayedAnswerTimerMsg::createCopy(void) const
{
   return new ScDelayedAnswerTimerMsg(*this);
}

ScDelayedAnswerTimerMsg::~ScDelayedAnswerTimerMsg()
{

}

/* ============================ MANIPULATORS ============================== */

ScDelayedAnswerTimerMsg& ScDelayedAnswerTimerMsg::operator=(const ScDelayedAnswerTimerMsg& rhs)
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
