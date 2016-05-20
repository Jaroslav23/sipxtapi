//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsSharedServerTaskMsg.h>

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

OsSharedServerTaskMsg::OsSharedServerTaskMsg(OsSharedTaskInfo* pTaskInfo, OsMsg* pMsg)
: OsMsg(OS_SHARED_SERVER_TASK_MSG, 0)
, m_pTaskInfo(pTaskInfo)
, m_pMsg(pMsg)
{
}

OsSharedServerTaskMsg::OsSharedServerTaskMsg(const OsSharedServerTaskMsg& rhs)
: OsMsg(OS_SHARED_SERVER_TASK_MSG, 0)
, m_pMsg(rhs.m_pMsg)
, m_pTaskInfo(rhs.m_pTaskInfo)
{
}

OsSharedServerTaskMsg::~OsSharedServerTaskMsg()
{
}

OsMsg* OsSharedServerTaskMsg::createCopy(void) const
{
   return new OsSharedServerTaskMsg(m_pTaskInfo, m_pMsg);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
