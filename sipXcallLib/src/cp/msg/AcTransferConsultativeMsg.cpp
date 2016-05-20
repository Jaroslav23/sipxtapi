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
#include <cp/msg/AcTransferConsultativeMsg.h>

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

AcTransferConsultativeMsg::AcTransferConsultativeMsg(const SipDialog& sourceSipDialog,
                                                     const SipDialog& targetSipDialog)
: AcCommandMsg(AC_TRANSFER_CONSULTATIVE)
, m_sourceSipDialog(sourceSipDialog)
, m_targetSipDialog(targetSipDialog)
{

}

AcTransferConsultativeMsg::~AcTransferConsultativeMsg()
{

}

OsMsg* AcTransferConsultativeMsg::createCopy(void) const
{
   return new AcTransferConsultativeMsg(m_sourceSipDialog, m_targetSipDialog);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

