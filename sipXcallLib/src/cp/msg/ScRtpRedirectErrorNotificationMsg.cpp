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
#include <cp/msg/ScRtpRedirectErrorNotificationMsg.h>

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

ScRtpRedirectErrorNotificationMsg::ScRtpRedirectErrorNotificationMsg(const SipDialog& targetSipDialog,
                                                                     const SipDialog& sourceSipDialog,
                                                                     CP_RTP_REDIRECT_CAUSE cause,
                                                                     int responseCode,
                                                                     const UtlString& responseText)
: ScNotificationMsg(ScNotificationMsg::SCN_RTP_REDIRECT_ERROR, targetSipDialog)
, m_sourceSipDialog(sourceSipDialog)
, m_cause(cause)
, m_responseCode(responseCode)
, m_responseText(responseText)
{

}

ScRtpRedirectErrorNotificationMsg::ScRtpRedirectErrorNotificationMsg(const ScRtpRedirectErrorNotificationMsg& rMsg)
: ScNotificationMsg(rMsg)
, m_sourceSipDialog(rMsg.m_sourceSipDialog)
, m_cause(rMsg.m_cause)
, m_responseCode(rMsg.m_responseCode)
, m_responseText(rMsg.m_responseText)
{

}

ScRtpRedirectErrorNotificationMsg::~ScRtpRedirectErrorNotificationMsg()
{

}

OsMsg* ScRtpRedirectErrorNotificationMsg::createCopy(void) const
{
   return new ScRtpRedirectErrorNotificationMsg(*this);
}

/* ============================ MANIPULATORS ============================== */

ScRtpRedirectErrorNotificationMsg& ScRtpRedirectErrorNotificationMsg::operator=(const ScRtpRedirectErrorNotificationMsg& rhs)
{
   if (this == &rhs)
   {
      return *this;
   }

   ScNotificationMsg::operator=(rhs); // assign fields for parent class

   m_sourceSipDialog = rhs.m_sourceSipDialog;
   m_cause = rhs.m_cause;
   m_responseCode = rhs.m_responseCode;
   m_responseText = rhs.m_responseText;

   return *this;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

