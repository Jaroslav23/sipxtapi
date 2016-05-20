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
#include <cp/msg/AcRedirectConnectionMsg.h>

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

AcRedirectConnectionMsg::AcRedirectConnectionMsg(const SipDialog& sSipDialog,
                                                 const UtlString& sRedirectSipUrl)
: AcCommandMsg(AC_REDIRECT_CONNECTION)
, m_sSipDialog(sSipDialog)
, m_sRedirectSipUrl(sRedirectSipUrl)
{

}

AcRedirectConnectionMsg::~AcRedirectConnectionMsg()
{

}

OsMsg* AcRedirectConnectionMsg::createCopy(void) const
{
   return new AcRedirectConnectionMsg(m_sSipDialog, m_sRedirectSipUrl);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

