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
#include <cp/msg/CpTimerMsg.h>

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

CpTimerMsg::CpTimerMsg(SubTypeEnum msgSubType)
: OsTimerMsg((const unsigned char)msgSubType)
{

}

CpTimerMsg::CpTimerMsg(const CpTimerMsg& rhs)
: OsTimerMsg(rhs)
{
}

OsMsg* CpTimerMsg::createCopy(void) const
{
   return new CpTimerMsg(*this);
}

CpTimerMsg::~CpTimerMsg()
{

}

/* ============================ MANIPULATORS ============================== */

CpTimerMsg& CpTimerMsg::operator=(const CpTimerMsg& rhs)
{
   if (this == &rhs)
   {
      return *this;
   }

   OsTimerMsg::operator=(rhs); // assign fields for parent class

   return *this;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
