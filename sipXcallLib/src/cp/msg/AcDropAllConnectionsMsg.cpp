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
#include <cp/msg/AcDropAllConnectionsMsg.h>

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

AcDropAllConnectionsMsg::AcDropAllConnectionsMsg(UtlBoolean bDestroyAbstractCall)
: AcCommandMsg(AC_DROP_ALL_CONNECTIONS)
, m_bDestroyAbstractCall(bDestroyAbstractCall)
{

}

AcDropAllConnectionsMsg::~AcDropAllConnectionsMsg()
{

}

OsMsg* AcDropAllConnectionsMsg::createCopy(void) const
{
   return new AcDropAllConnectionsMsg(m_bDestroyAbstractCall);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

