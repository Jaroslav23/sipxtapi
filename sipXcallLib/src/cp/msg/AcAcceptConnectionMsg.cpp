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
#include <cp/msg/AcAcceptConnectionMsg.h>

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

AcAcceptConnectionMsg::AcAcceptConnectionMsg(const SipDialog& sSipDialog,
                                             UtlBoolean bSendSDP,
                                             const UtlString& sLocationHeader,
                                             CP_CONTACT_ID contactId,
                                             SIP_TRANSPORT_TYPE transport)
: AcCommandMsg(AC_ACCEPT_CONNECTION)
, m_sSipDialog(sSipDialog)
, m_bSendSDP(bSendSDP)
, m_sLocationHeader(sLocationHeader)
, m_contactId(contactId)
, m_transport(transport)
{

}

AcAcceptConnectionMsg::~AcAcceptConnectionMsg()
{

}

OsMsg* AcAcceptConnectionMsg::createCopy(void) const
{
   return new AcAcceptConnectionMsg(m_sSipDialog, m_bSendSDP, m_sLocationHeader,
      m_contactId, m_transport);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

