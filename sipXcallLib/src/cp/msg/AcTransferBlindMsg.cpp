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
#include <cp/msg/AcTransferBlindMsg.h>

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

AcTransferBlindMsg::AcTransferBlindMsg(const SipDialog& sipDialog,
                                       const UtlString& sTransferSipUrl)
: AcCommandMsg(AC_TRANSFER_BLIND)
, m_sipDialog(sipDialog)
, m_sTransferSipUrl(sTransferSipUrl)
{

}

AcTransferBlindMsg::~AcTransferBlindMsg()
{

}

OsMsg* AcTransferBlindMsg::createCopy(void) const
{
   return new AcTransferBlindMsg(m_sipDialog, m_sTransferSipUrl);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

