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
#include <cp/msg/Sc2xxTimerMsg.h>

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

Sc2xxTimerMsg::Sc2xxTimerMsg(const UtlString& sCallId,
                             const UtlString& sLocalTag,
                             const UtlString& sRemoteTag,
                             UtlBoolean isFromLocal)
: ScTimerMsg(ScTimerMsg::PAYLOAD_TYPE_2XX, sCallId, sLocalTag, sRemoteTag, isFromLocal)
{

}

Sc2xxTimerMsg::Sc2xxTimerMsg(const Sc2xxTimerMsg& rhs)
: ScTimerMsg(rhs)
{
}

OsMsg* Sc2xxTimerMsg::createCopy(void) const
{
   return new Sc2xxTimerMsg(*this);
}

Sc2xxTimerMsg::~Sc2xxTimerMsg()
{

}

/* ============================ MANIPULATORS ============================== */

Sc2xxTimerMsg& Sc2xxTimerMsg::operator=(const Sc2xxTimerMsg& rhs)
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
