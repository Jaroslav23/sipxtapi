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
#include <cp/msg/AcStartRtpRedirectMsg.h>

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

AcStartRtpRedirectMsg::AcStartRtpRedirectMsg(const UtlString& slaveAbstractCallId, const SipDialog& slaveSipDialog)
: AcCommandMsg(AC_START_RTP_REDIRECT)
, m_slaveAbstractCallId(slaveAbstractCallId)
, m_slaveSipDialog(slaveSipDialog)
{

}

AcStartRtpRedirectMsg::~AcStartRtpRedirectMsg()
{

}

OsMsg* AcStartRtpRedirectMsg::createCopy(void) const
{
   return new AcStartRtpRedirectMsg(m_slaveAbstractCallId, m_slaveSipDialog);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

