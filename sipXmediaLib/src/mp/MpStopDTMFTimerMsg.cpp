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
#include <mp/MpStopDTMFTimerMsg.h>

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

MpStopDTMFTimerMsg::MpStopDTMFTimerMsg()
: MpTimerMsg(MpTimerMsg::MP_STOP_DTMF_TONE_TIMER)
{

}

MpStopDTMFTimerMsg::MpStopDTMFTimerMsg(const MpStopDTMFTimerMsg& rhs)
: MpTimerMsg(rhs)
{
}

OsMsg* MpStopDTMFTimerMsg::createCopy(void) const
{
   return new MpStopDTMFTimerMsg(*this);
}

MpStopDTMFTimerMsg::~MpStopDTMFTimerMsg()
{

}

/* ============================ MANIPULATORS ============================== */

MpStopDTMFTimerMsg& MpStopDTMFTimerMsg::operator=(const MpStopDTMFTimerMsg& rhs)
{
   if (this == &rhs)
   {
      return *this;
   }

   MpTimerMsg::operator=(rhs); // assign fields for parent class
   
   return *this;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
