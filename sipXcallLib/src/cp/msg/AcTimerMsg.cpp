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
#include <cp/msg/AcTimerMsg.h>

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

AcTimerMsg::AcTimerMsg(PayloadTypeEnum payloadType)
: CpTimerMsg(CpTimerMsg::CP_ABSTRACT_CALL_TIMER)
, m_payloadType(payloadType)
{

}

AcTimerMsg::AcTimerMsg(const AcTimerMsg& rhs)
: CpTimerMsg(rhs)
, m_payloadType(rhs.m_payloadType)
{
}

OsMsg* AcTimerMsg::createCopy(void) const
{
   return new AcTimerMsg(*this);
}

AcTimerMsg::~AcTimerMsg()
{

}

/* ============================ MANIPULATORS ============================== */

AcTimerMsg& AcTimerMsg::operator=(const AcTimerMsg& rhs)
{
   if (this == &rhs)
   {
      return *this;
   }

   CpTimerMsg::operator=(rhs); // assign fields for parent class

   m_payloadType = rhs.m_payloadType;

   return *this;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
