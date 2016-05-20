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
#include <cp/msg/AcUnsubscribeMsg.h>

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

AcUnsubscribeMsg::AcUnsubscribeMsg(CP_NOTIFICATION_TYPE notificationType,
                                   const SipDialog& targetSipDialog,
                                   const SipDialog& callbackSipDialog)
: AcCommandMsg(AC_UNSUBSCRIBE)
, m_notificationType(notificationType)
, m_targetSipDialog(targetSipDialog)
, m_callbackSipDialog(callbackSipDialog)
{

}

AcUnsubscribeMsg::~AcUnsubscribeMsg()
{

}

OsMsg* AcUnsubscribeMsg::createCopy(void) const
{
   return new AcUnsubscribeMsg(m_notificationType, m_targetSipDialog, m_callbackSipDialog);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

