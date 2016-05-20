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
#include <os/OsStunResultFailureMsg.h>

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

OsStunResultFailureMsg::OsStunResultFailureMsg(const UtlString& sAdapterName,
                                               const UtlString& sLocalIp,
                                               int localPort)
: OsStunResultMsg(OsStunResultMsg::STUN_RESULT_FAILURE, sAdapterName, sLocalIp, localPort)
{

}

OsStunResultFailureMsg::~OsStunResultFailureMsg()
{

}

OsMsg* OsStunResultFailureMsg::createCopy(void) const
{
   return new OsStunResultFailureMsg(m_sAdapterName, m_sLocalIp, m_localPort);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
