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
#include <os/OsSharedServerTaskMgr.h>
#include <os/OsSharedServerTask.h>
#include <os/OsSharedTaskInfo.h>
#include <os/OsLock.h>
#include <os/OsPtrLock.h>
#include <os/OsSharedServerTaskMsg.h>
#include <os/OsSharedServerTaskWorker.h>
#include <utl/UtlInt.h>
#include <utl/UtlHashBagIterator.h>
#include <utl/UtlSListIterator.h>

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

OsSharedServerTaskMgr::OsSharedServerTaskMgr(unsigned int iNumberOfThreads /*= SHARED_TASK_DEFAULT_THREADS*/,
                                             const int maxRequestQMsgs)
: m_msgQ(maxRequestQMsgs, OsMsgQ::DEF_MAX_MSG_LEN, OsMsgQ::Q_PRIORITY)
, m_state(SHARED_TASK_MGR_UNINITIALIZED)
{
   unsigned int realNumberOfThreads = iNumberOfThreads > 0 ? iNumberOfThreads : 1;

   for (unsigned int i = 0; i < realNumberOfThreads; i++)
   {
      OsSharedServerTaskWorker* pWorker = new OsSharedServerTaskWorker("OsSharedServerTaskMgr-%d", i, NULL, &m_msgQ);
      m_threadList.insert(pWorker);
      // do not start workers
   }
}

OsSharedServerTaskMgr::~OsSharedServerTaskMgr()
{
   shutdownAllWorkers(); // stop all threads
   releaseAllSharedTasks(); // unmanage all shared tasks
   flushMessages(); // delete all messages from queue
}

/* ============================ MANIPULATORS ============================== */

void OsSharedServerTaskMgr::start()
{
   OsLock lock(m_memberMutex);
   UtlSListIterator itor(m_threadList);
   while (itor())
   {
      OsSharedServerTaskWorker* pWorker = dynamic_cast<OsSharedServerTaskWorker*>(itor.item());
      if (pWorker)
      {
         pWorker->start();
      }
   }
   m_state = SHARED_TASK_MGR_STARTED;
}

void OsSharedServerTaskMgr::shutdown()
{
   shutdownAllWorkers();
}

UtlBoolean OsSharedServerTaskMgr::manage(OsSharedServerTask& serverTask)
{
   if (serverTask.isManaged())
   {
      // we cannot manage OsSharedServerTask if its already managed
      return FALSE;
   }

   UtlInt key(serverTask.getTaskId());
   OsLock lock(m_memberMutex);
   // first try to find if we are already managing it
   OsSharedTaskInfo* pTaskInfo = dynamic_cast<OsSharedTaskInfo*>(m_managedTaskInfo.find(&key));
   if (!pTaskInfo)
   {
      // add it
      pTaskInfo = new OsSharedTaskInfo(serverTask.getTaskId(), &serverTask);
      m_managedTaskInfo.insert(pTaskInfo);
      serverTask.taskAttached(this);
      return TRUE;
   }

   return FALSE;
}

UtlBoolean OsSharedServerTaskMgr::release(OsSharedServerTask& serverTask)
{
   UtlInt key(serverTask.getTaskId());
   m_memberMutex.acquire();
   OsSharedTaskInfo* pTaskInfo = dynamic_cast<OsSharedTaskInfo*>(m_managedTaskInfo.remove(&key));
   if (pTaskInfo)
   {
      pTaskInfo->acquire(); // lock and never unlock
      m_memberMutex.release();

      // we have to wait until all messages of this task have been processed. There is no way we can remove them from queue.
      // only current thread is blocked
      while (pTaskInfo->getMsgCount() > 0 && m_state != SHARED_TASK_MGR_SHUT_DOWN)
      {
         OsTask::delay(20); // wait 20ms
      }
      delete pTaskInfo; // once all messages have been processed, its safe to delete the task
      pTaskInfo = NULL;
      serverTask.taskReleased();
      return TRUE;
   }
   else
   {
      // already removed
      m_memberMutex.release();
   }

   return FALSE;
}

void OsSharedServerTaskMgr::releaseAllSharedTasks()
{
   OsLock lock(m_memberMutex);
   UtlHashBagIterator itor(m_managedTaskInfo);
   while (itor())
   {
      OsSharedTaskInfo* pTaskInfo = dynamic_cast<OsSharedTaskInfo*>(itor.key());
      if (pTaskInfo)
      {
         pTaskInfo->acquire(); // lock to make deletion safe, never unlock
         OsSharedServerTask* pSharedTask = pTaskInfo->getSharedServerTask();
         if (pSharedTask)
         {
            pSharedTask->taskReleased();
         }
      }
   }

   m_managedTaskInfo.destroyAll();
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

OsSharedServerTaskMgr::OsSharedServerTaskMgr(const OsSharedServerTaskMgr& rhs)
{
}

OsSharedServerTaskMgr& OsSharedServerTaskMgr::operator=(const OsSharedServerTaskMgr& rhs)
{
   return *this; // not supported
}

OsStatus OsSharedServerTaskMgr::postMessage(OsSharedServerTask* pServerTask,
                                            const OsMsg& rMsg,
                                            const OsTime& rTimeout /*= OsTime::OS_INFINITY*/,
                                            UtlBoolean sentFromISR /*= FALSE*/)
{
   if (pServerTask)
   {
      UtlInt key(pServerTask->getTaskId());
      m_memberMutex.acquire();
      OsSharedTaskInfo* pTaskInfo = dynamic_cast<OsSharedTaskInfo*>(m_managedTaskInfo.find(&key));
      if (pTaskInfo)
      {
         OsLock lock(*pTaskInfo); // autolock  pTaskInfo
         m_memberMutex.release();

         pTaskInfo->incrementMsgCount();
         OsSharedServerTaskMsg taskMsg(pTaskInfo, rMsg.createCopy());
         return m_msgQ.send(taskMsg);
      }
   }

   return OS_FAILED;
}

void OsSharedServerTaskMgr::flushMessages()
{
   OsLock lock(m_memberMutex);
   OsMsg* pMsg = NULL;
   OsStatus result = OS_FAILED;

   do 
   {
      // receive all messages and delete them without delivery
      result = m_msgQ.receive(pMsg, OsTime::NO_WAIT_TIME);
      OsSharedServerTaskMsg* pTaskMsg = dynamic_cast<OsSharedServerTaskMsg*>(pMsg);
      if (pTaskMsg)
      {
         OsMsg* pInnerMsg = pTaskMsg->getMsg();
         if (pInnerMsg)
         {
            pInnerMsg->releaseMsg();
         }
         pTaskMsg->releaseMsg();
      }
   }
   while(result == OS_SUCCESS);
}

void OsSharedServerTaskMgr::shutdownAllWorkers()
{
   m_state = SHARED_TASK_MGR_SHUTTING_DOWN;
   OsLock lock(m_memberMutex);
   UtlSListIterator itor(m_threadList);
   while (itor())
   {
      OsSharedServerTaskWorker* pWorker = dynamic_cast<OsSharedServerTaskWorker*>(itor.item());
      if (pWorker)
      {
         pWorker->waitUntilShutDown();
      }
   }

   m_threadList.destroyAll();
   m_state = SHARED_TASK_MGR_SHUT_DOWN;
}

/* ============================ FUNCTIONS ================================= */
