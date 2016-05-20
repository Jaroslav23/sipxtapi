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
#include <os/OsSharedTaskInfo.h>
#include <os/OsSharedServerTask.h>
#include <os/OsSharedServerTaskMgr.h>
#include <os/OsLock.h>

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

OsSharedTaskInfo::OsSharedTaskInfo(const int iTaskId,
                                   OsSharedServerTask* pSharedServerTask)
: UtlInt(iTaskId)
, m_pSharedServerTask(pSharedServerTask)
, m_deletionGuard(OsMutex::Q_FIFO)
, m_msgCountMutex(OsMutex::Q_FIFO)
, m_msgCount(0)
{

}

OsSharedTaskInfo::~OsSharedTaskInfo()
{
   m_pSharedServerTask = NULL;
}

/* ============================ MANIPULATORS ============================== */

OsStatus OsSharedTaskInfo::acquire(const OsTime& rTimeout /*= OsTime::OS_INFINITY*/)
{
   return m_deletionGuard.acquire(rTimeout);
}

OsStatus OsSharedTaskInfo::tryAcquire()
{
   return m_deletionGuard.tryAcquire();
}

OsStatus OsSharedTaskInfo::release()
{
   return m_deletionGuard.release();
}

void OsSharedTaskInfo::incrementMsgCount()
{
   OsLock lock(m_msgCountMutex);
   m_msgCount++;
}

void OsSharedTaskInfo::decrementMsgCount()
{
   OsLock lock(m_msgCountMutex);
   m_msgCount--;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

OsSharedTaskInfo::OsSharedTaskInfo(const OsSharedTaskInfo& rhs)
: m_pSharedServerTask(NULL)
, m_deletionGuard(OsMutex::Q_FIFO)
, m_msgCount(rhs.m_msgCount)
{

}

OsSharedTaskInfo& OsSharedTaskInfo::operator=(const OsSharedTaskInfo& rhs)
{
   return *this;
}

/* ============================ FUNCTIONS ================================= */
