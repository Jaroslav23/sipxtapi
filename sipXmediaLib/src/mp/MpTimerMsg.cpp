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
#include <mp/MpTimerMsg.h>

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

MpTimerMsg::MpTimerMsg(SubTypeEnum msgSubType)
: OsTimerMsg((const unsigned char)msgSubType)
{

}

MpTimerMsg::MpTimerMsg(const MpTimerMsg& rhs)
: OsTimerMsg(rhs)
{
}

OsMsg* MpTimerMsg::createCopy(void) const
{
   return new MpTimerMsg(*this);
}

MpTimerMsg::~MpTimerMsg()
{

}

/* ============================ MANIPULATORS ============================== */

MpTimerMsg& MpTimerMsg::operator=(const MpTimerMsg& rhs)
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
