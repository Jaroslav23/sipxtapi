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
#include <cp/msg/ScStartRtpRedirectAckNotificationMsg.h>

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

ScStartRtpRedirectAckNotificationMsg::ScStartRtpRedirectAckNotificationMsg(const SipDialog& targetSipDialog,
                                                                           const SipDialog& sourceSipDialog,
                                                                           const SdpBody& sourceRemoteSdpBody)
: ScNotificationMsg(ScNotificationMsg::SCN_START_RTP_REDIRECT_ACK, targetSipDialog)
, m_sourceSipDialog(sourceSipDialog)
, m_sourceRemoteSdpBody(sourceRemoteSdpBody)
{

}

ScStartRtpRedirectAckNotificationMsg::ScStartRtpRedirectAckNotificationMsg(const ScStartRtpRedirectAckNotificationMsg& rMsg)
: ScNotificationMsg(rMsg)
, m_sourceSipDialog(rMsg.m_sourceSipDialog)
, m_sourceRemoteSdpBody(rMsg.m_sourceRemoteSdpBody)
{

}

ScStartRtpRedirectAckNotificationMsg::~ScStartRtpRedirectAckNotificationMsg()
{

}

OsMsg* ScStartRtpRedirectAckNotificationMsg::createCopy(void) const
{
   return new ScStartRtpRedirectAckNotificationMsg(*this);
}

/* ============================ MANIPULATORS ============================== */

ScStartRtpRedirectAckNotificationMsg& ScStartRtpRedirectAckNotificationMsg::operator=(const ScStartRtpRedirectAckNotificationMsg& rhs)
{
   if (this == &rhs)
   {
      return *this;
   }

   ScNotificationMsg::operator=(rhs); // assign fields for parent class

   m_sourceSipDialog = rhs.m_sourceSipDialog;
   m_sourceRemoteSdpBody = rhs.m_sourceRemoteSdpBody;

   return *this;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

