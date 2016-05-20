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
#include <cp/msg/ScStartRtpRedirectCommandMsg.h>

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

ScStartRtpRedirectCommandMsg::ScStartRtpRedirectCommandMsg(const SipDialog& targetSipDialog,
                                                           const SipDialog& sourceSipDialog,
                                                           const SdpBody& sourceRemoteSdpBody)
: ScCommandMsg(ScCommandMsg::SCC_START_RTP_REDIRECT, targetSipDialog)
, m_sourceSipDialog(sourceSipDialog)
, m_sourceRemoteSdpBody(sourceRemoteSdpBody)
{

}

ScStartRtpRedirectCommandMsg::ScStartRtpRedirectCommandMsg(const ScStartRtpRedirectCommandMsg& rMsg)
: ScCommandMsg(rMsg)
, m_sourceSipDialog(rMsg.m_sourceSipDialog)
, m_sourceRemoteSdpBody(rMsg.m_sourceRemoteSdpBody)
{

}

ScStartRtpRedirectCommandMsg::~ScStartRtpRedirectCommandMsg()
{

}

OsMsg* ScStartRtpRedirectCommandMsg::createCopy(void) const
{
   return new ScStartRtpRedirectCommandMsg(*this);
}

/* ============================ MANIPULATORS ============================== */

ScStartRtpRedirectCommandMsg& ScStartRtpRedirectCommandMsg::operator=(const ScStartRtpRedirectCommandMsg& rhs)
{
   if (this == &rhs)
   {
      return *this;
   }

   ScCommandMsg::operator=(rhs); // assign fields for parent class

   m_sourceSipDialog = rhs.m_sourceSipDialog;
   m_sourceRemoteSdpBody = rhs.m_sourceRemoteSdpBody;

   return *this;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

