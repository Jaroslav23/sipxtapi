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
#include <os/OsSharedServerTask.h>
#include <os/OsSharedServerTaskMgr.h>
#include <os/OsLock.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

OsMutex OsSharedServerTask::m_idGeneratorMutex(OsMutex::Q_FIFO);
int OsSharedServerTask::m_nextId = 0;

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

OsSharedServerTask::OsSharedServerTask(const UtlString& name)
: m_pSharedServerTaskMgr(NULL)
, m_sTaskName(name)
, m_iTaskId(getNextTaskId())
{

}

OsSharedServerTask::~OsSharedServerTask()
{
   release();
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean OsSharedServerTask::release()
{
   OsLock lock(m_memberMutex);
   if (m_pSharedServerTaskMgr)
   {
      return m_pSharedServerTaskMgr->release(*this);
   }

   return FALSE;
}

OsStatus OsSharedServerTask::postMessage(const OsMsg& rMsg,
                                         const OsTime& rTimeout /*= OsTime::OS_INFINITY*/,
                                         UtlBoolean sentFromISR /*= FALSE*/)
{
   OsStatus res = OS_FAILED;

   OsStatus mutexResult = m_memberMutex.acquire(rTimeout);
   if (mutexResult == OS_SUCCESS)
   {
      if (m_pSharedServerTaskMgr)
      {
         res = m_pSharedServerTaskMgr->postMessage(this, rMsg, rTimeout, sentFromISR);
      }
      m_memberMutex.release();
   }

   return res;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

UtlBoolean OsSharedServerTask::isManaged()
{
   return m_pSharedServerTaskMgr != NULL;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

OsSharedServerTask::OsSharedServerTask(const OsSharedServerTask& rhs)
: m_memberMutex(OsMutex::Q_FIFO)
{
   // not supported
}

OsSharedServerTask& OsSharedServerTask::operator=(const OsSharedServerTask& rhs)
{
   return *this; // not supported
}

int OsSharedServerTask::getNextTaskId()
{
   OsLock lock(m_idGeneratorMutex);
   return m_nextId++;
}

void OsSharedServerTask::taskReleased()
{
   OsLock lock(m_memberMutex);
   m_pSharedServerTaskMgr = NULL;
}

void OsSharedServerTask::taskAttached(OsSharedServerTaskMgr* pSharedServerTaskMgr)
{
   OsLock lock(m_memberMutex);
   m_pSharedServerTaskMgr = pSharedServerTaskMgr;
}

/* ============================ FUNCTIONS ================================= */
