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
#include <cp/msg/AcConnectMsg.h>

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

AcConnectMsg::AcConnectMsg(const UtlString& sSipCallId,
                           const UtlString& sToAddress,
                           const UtlString& sFromTag,
                           const UtlString& sFromAddress,
                           const UtlString& sLocationHeader,
                           CP_CONTACT_ID contactId,
                           SIP_TRANSPORT_TYPE transport,
                           const UtlString& replacesField,
                           CP_CALLSTATE_CAUSE callstateCause,
                           const SipDialog* pCallbackSipDialog)
: AcCommandMsg(AC_CONNECT)
, m_sSipCallId(sSipCallId)
, m_sToAddress(sToAddress)
, m_sLocalTag(sFromTag)
, m_sFromAddress(sFromAddress)
, m_sLocationHeader(sLocationHeader)
, m_contactId(contactId)
, m_transport(transport)
, m_replacesField(replacesField)
, m_callstateCause(callstateCause)
, m_pCallbackSipDialog(NULL)
{
   if (pCallbackSipDialog)
   {
      m_pCallbackSipDialog = new SipDialog(*pCallbackSipDialog);
   }
}

AcConnectMsg::~AcConnectMsg()
{
   delete m_pCallbackSipDialog;
   m_pCallbackSipDialog = NULL;
}

OsMsg* AcConnectMsg::createCopy(void) const
{
   return new AcConnectMsg(m_sSipCallId, m_sToAddress, m_sLocalTag, m_sFromAddress, m_sLocationHeader, m_contactId,
      m_transport, m_replacesField, m_callstateCause, m_pCallbackSipDialog);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

