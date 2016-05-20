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
#include <cp/XCpCallIdUtil.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const char* callIdPrefix = "call_"; // id prefix for all calls. Only used internally.
const char* conferenceIdPrefix = "conf_"; // id prefix for all conferences. Only used internally.
SipCallIdGenerator XCpCallIdUtil::ms_callIdGenerator(callIdPrefix);
SipCallIdGenerator XCpCallIdUtil::ms_conferenceIdGenerator(conferenceIdPrefix);

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

UtlString XCpCallIdUtil::getNewCallId()
{
   return ms_callIdGenerator.getNewCallId();
}

UtlString XCpCallIdUtil::getNewConferenceId()
{
   return ms_conferenceIdGenerator.getNewCallId();
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

UtlBoolean XCpCallIdUtil::isCallId(const UtlString& sId)
{
   XCpCallIdUtil::ID_TYPE type = getIdType(sId);
   return type == XCpCallIdUtil::ID_TYPE_CALL;
}

UtlBoolean XCpCallIdUtil::isConferenceId(const UtlString& sId)
{
   XCpCallIdUtil::ID_TYPE type = getIdType(sId);
   return type == XCpCallIdUtil::ID_TYPE_CONFERENCE;
}

XCpCallIdUtil::ID_TYPE XCpCallIdUtil::getIdType(const UtlString& sId)
{
   if (sId.first(callIdPrefix) >= 0)
   {
      return XCpCallIdUtil::ID_TYPE_CALL;
   }
   else if (sId.first(conferenceIdPrefix) >= 0)
   {
      return XCpCallIdUtil::ID_TYPE_CONFERENCE;
   }
   else return XCpCallIdUtil::ID_TYPE_UNKNOWN;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
