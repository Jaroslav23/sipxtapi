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
#include <cp/msg/ScStopRtpRedirectCommandMsg.h>

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

ScStopRtpRedirectCommandMsg::ScStopRtpRedirectCommandMsg(const SipDialog& targetSipDialog,
                                                         const SipDialog& sourceSipDialog)
: ScCommandMsg(ScCommandMsg::SCC_STOP_RTP_REDIRECT, targetSipDialog)
, m_sourceSipDialog(sourceSipDialog)
{

}

ScStopRtpRedirectCommandMsg::ScStopRtpRedirectCommandMsg(const ScStopRtpRedirectCommandMsg& rMsg)
: ScCommandMsg(rMsg)
, m_sourceSipDialog(rMsg.m_sourceSipDialog)
{

}

ScStopRtpRedirectCommandMsg::~ScStopRtpRedirectCommandMsg()
{

}

OsMsg* ScStopRtpRedirectCommandMsg::createCopy(void) const
{
   return new ScStopRtpRedirectCommandMsg(*this);
}

/* ============================ MANIPULATORS ============================== */

ScStopRtpRedirectCommandMsg& ScStopRtpRedirectCommandMsg::operator=(const ScStopRtpRedirectCommandMsg& rhs)
{
   if (this == &rhs)
   {
      return *this;
   }

   ScCommandMsg::operator=(rhs); // assign fields for parent class

   m_sourceSipDialog = rhs.m_sourceSipDialog;

   return *this;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

