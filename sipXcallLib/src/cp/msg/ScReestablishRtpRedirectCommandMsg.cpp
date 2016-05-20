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
#include <cp/msg/ScReestablishRtpRedirectCommandMsg.h>

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

ScReestablishRtpRedirectCommandMsg::ScReestablishRtpRedirectCommandMsg(const SipDialog& targetSipDialog,
                                                                       const SipDialog& sourceSipDialog,
                                                                       const SdpBody& sourceRemoteSdpBody)
: ScCommandMsg(ScCommandMsg::SCC_REESTABLISH_RTP_REDIRECT, targetSipDialog)
, m_sourceSipDialog(sourceSipDialog)
, m_sourceRemoteSdpBody(sourceRemoteSdpBody)
{

}

ScReestablishRtpRedirectCommandMsg::ScReestablishRtpRedirectCommandMsg(const ScReestablishRtpRedirectCommandMsg& rMsg)
: ScCommandMsg(rMsg)
, m_sourceSipDialog(rMsg.m_sourceSipDialog)
, m_sourceRemoteSdpBody(rMsg.m_sourceRemoteSdpBody)
{

}

ScReestablishRtpRedirectCommandMsg::~ScReestablishRtpRedirectCommandMsg()
{

}

OsMsg* ScReestablishRtpRedirectCommandMsg::createCopy(void) const
{
   return new ScReestablishRtpRedirectCommandMsg(*this);
}

/* ============================ MANIPULATORS ============================== */

ScReestablishRtpRedirectCommandMsg& ScReestablishRtpRedirectCommandMsg::operator=(const ScReestablishRtpRedirectCommandMsg& rhs)
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

