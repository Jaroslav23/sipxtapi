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
#include <os/OsStunResultSuccessMsg.h>

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

OsStunResultSuccessMsg::OsStunResultSuccessMsg(const UtlString& sAdapterName,
                                               const UtlString& sLocalIp,
                                               int localPort,
                                               const UtlString& sMappedIp,
                                               int mappedPort)
: OsStunResultMsg(OsStunResultMsg::STUN_RESULT_SUCCESS, sAdapterName, sLocalIp, localPort)
, m_sMappedIp(sMappedIp)
, m_mappedPort(mappedPort)
{

}

OsStunResultSuccessMsg::~OsStunResultSuccessMsg()
{

}

OsMsg* OsStunResultSuccessMsg::createCopy(void) const
{
   return new OsStunResultSuccessMsg(m_sAdapterName, m_sLocalIp, m_localPort, m_sMappedIp, m_mappedPort);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
