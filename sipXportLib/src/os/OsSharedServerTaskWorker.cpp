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
#include <os/OsSharedServerTaskWorker.h>
#include <os/OsSharedServerTaskMsg.h>
#include <os/OsSharedServerTask.h>
#include <os/OsSharedTaskInfo.h>
#include <os/OsMsgQ.h>

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

OsSharedServerTaskWorker::OsSharedServerTaskWorker(const UtlString& name,
                                                   const int workerId,
                                                   void* pArg,
                                                   OsMsgQ* pMsgQ,
                                                   const int priority,
                                                   const int options,
                                                   const int stackSize)
: OsTask(name, pArg, priority, options, stackSize)
, UtlInt(workerId)
, m_pMsgQ(pMsgQ)
{

}

OsSharedServerTaskWorker::~OsSharedServerTaskWorker()
{
   waitUntilShutDown();
   m_pMsgQ = NULL;
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

void OsSharedServerTaskWorker::requestShutdown(void)
{
   if (m_pMsgQ)
   {
      OsStatus res;
      OsMsg msg(OsMsg::OS_SHUTDOWN, 0);

      OsTask::requestShutdown();
      res = m_pMsgQ->send(msg);
      assert(res = OS_SUCCESS);
   }
}

int OsSharedServerTaskWorker::run(void* pArg)
{
   UtlBoolean doShutdown;
   OsMsg* pMsg = NULL;
   OsStatus res;

   if (m_pMsgQ)
   {
      do
      {
         res = m_pMsgQ->receive(pMsg);
         assert(res == OS_SUCCESS);

         doShutdown = isShuttingDown();

         if (pMsg)
         {
            // always handle message that unblocked the queue, since message contains another msg which needs to be released
            handleMessage(*pMsg);
            pMsg->releaseMsg();
         }
      }
      while (!doShutdown);
   }

   return 0;        // and then exit
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean OsSharedServerTaskWorker::handleMessage(OsMsg& rMsg)
{
   UtlBoolean handled = FALSE;

   switch (rMsg.getMsgType())
   {
   case OsMsg::OS_SHUTDOWN:
      handled = TRUE;
      break;
   case OS_SHARED_SERVER_TASK_MSG:
      {
         OsSharedServerTaskMsg* pTaskMsg = static_cast<OsSharedServerTaskMsg*>(&rMsg);
         if (pTaskMsg)
         {
            handled = handleOsSharedServerTaskMessage(*pTaskMsg);
         }
      }
      break;
   default:
      osPrintf("OsSharedServerTaskWorker::handleMessage(): msg type is %d.%d, not OS_SHUTDOWN\n", rMsg.getMsgType(), rMsg.getMsgSubType());
      break;
   }

   return handled;
}

UtlBoolean OsSharedServerTaskWorker::handleOsSharedServerTaskMessage(OsSharedServerTaskMsg& rMsg)
{
   UtlBoolean handled = FALSE;
   OsMsg* pPayloadMsg = rMsg.getMsg();

   if (pPayloadMsg)
   {
      OsSharedTaskInfo* pTaskInfo = rMsg.getTaskInfo();
      if (pTaskInfo)
      {
         OsSharedServerTask* pTask = pTaskInfo->getSharedServerTask();
         if (pTask)
         {
            // handle message in original OsSharedServerTask
            handled = pTask->handleMessage(*pPayloadMsg);

            // decrease msg count
            pTaskInfo->decrementMsgCount();
         }
      }

      pPayloadMsg->releaseMsg();
   }

   return handled;
}

/* ============================ FUNCTIONS ================================= */
