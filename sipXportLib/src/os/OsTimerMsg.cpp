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
#include <os/OsTimerMsg.h>

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

OsTimerMsg::OsTimerMsg(const unsigned char msgSubType)
: OsMsg(OS_TIMER_MSG, msgSubType)
, m_timestamp()
{

}

OsTimerMsg::OsTimerMsg(const OsTimerMsg& rhs)
: OsMsg(rhs)
, m_timestamp(rhs.m_timestamp)
{
}

OsMsg* OsTimerMsg::createCopy(void) const
{
   return new OsTimerMsg(*this);
}

OsTimerMsg::~OsTimerMsg()
{

}

/* ============================ MANIPULATORS ============================== */

OsTimerMsg& OsTimerMsg::operator=(const OsTimerMsg& rhs)
{
   if (this == &rhs)
   {
      return *this;
   }

   OsMsg::operator=(rhs); // assign fields for parent class

   m_timestamp = rhs.m_timestamp;

   return *this;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
