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
#include <os/OsStunResultMsg.h>

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

OsStunResultMsg::OsStunResultMsg(SubTypeEnum subType,
                                 const UtlString& sAdapterName,
                                 const UtlString& sLocalIp,
                                 int localPort)
: OsMsg(OsMsg::OS_STUN_RESULT_MSG, (unsigned char)subType)
, m_sAdapterName(sAdapterName)
, m_sLocalIp(sLocalIp)
, m_localPort(localPort)
{

}

OsStunResultMsg::~OsStunResultMsg()
{

}

OsMsg* OsStunResultMsg::createCopy(void) const
{
   return new OsStunResultMsg((SubTypeEnum)getMsgSubType(), m_sAdapterName, m_sLocalIp, m_localPort);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
