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

#ifndef OsSharedServerTaskMgr_h__
#define OsSharedServerTaskMgr_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlDefs.h>
#include <utl/UtlHashBag.h>
#include <utl/UtlSList.h>
#include <os/OsMsgQ.h>
#include <os/OsMutex.h>

// DEFINES
#define SHARED_TASK_DEFAULT_THREADS 1

// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS
class OsSharedServerTask;
class OsSharedTaskInfo;

/**
* OsSharedServerTaskMgr manages several OsSharedServerTasks. This class stores several threads,
* which are then used to process messages in OsSharedServerTasks.
* Destructor of this class is capable of handling cleanup of all resources cleanly.
*
*/
class OsSharedServerTaskMgr
{
   friend class OsSharedServerTask;
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   typedef enum
   {
      SHARED_TASK_MGR_UNINITIALIZED,
      SHARED_TASK_MGR_STARTED,
      SHARED_TASK_MGR_SHUTTING_DOWN,
      SHARED_TASK_MGR_SHUT_DOWN
   } SHARED_SERVER_TASK_STATE;

   OsSharedServerTaskMgr(unsigned int iNumberOfThreads = SHARED_TASK_DEFAULT_THREADS,// must be greater than 0
                         const int maxRequestQMsgs = OsMsgQ::DEF_MAX_MSGS);

   virtual ~OsSharedServerTaskMgr();

   /* ============================ MANIPULATORS ============================== */

   /**
    * Starts all OsSharedServerTaskMgr threads.
    */
   void start();

   /**
    * Blocking shutdown, waits until all threads are shut down.
    */
   void shutdown();

   /**
    * Starts managing given task.
    */
   UtlBoolean manage(OsSharedServerTask& serverTask);

   /**
    * Releases managed shared task.
    */
   UtlBoolean release(OsSharedServerTask& serverTask);

   /**
   * Flushes all messages from queue without delivering them, deleting each message.
   */
   void flushMessages();

   /* ============================ ACCESSORS ================================= */

   OsSharedServerTaskMgr::SHARED_SERVER_TASK_STATE getState() const { return m_state; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsSharedServerTaskMgr(const OsSharedServerTaskMgr& rhs);

   OsSharedServerTaskMgr& operator=(const OsSharedServerTaskMgr& rhs);

   /**
   * Posts new message into queue under given shared task.
   */
   OsStatus postMessage(OsSharedServerTask* pServerTask,
                        const OsMsg& rMsg,
                        const OsTime& rTimeout = OsTime::OS_INFINITY,
                        UtlBoolean sentFromISR = FALSE);

   void shutdownAllWorkers();

   /**
   * Releases all shared tasks.
   */
   void releaseAllSharedTasks();

   UtlHashBag m_managedTaskInfo; ///< stores information about all managed tasks
   OsMsgQ m_msgQ; ///< message queue shared by all OsSharedServerTasks and threads
   mutable OsMutex m_memberMutex;
   UtlSList m_threadList; ///< list of all OsSharedServerTaskWorkers
   SHARED_SERVER_TASK_STATE m_state; ///< state of OsSharedServerTaskMgr
};

#endif // OsSharedServerTaskMgr_h__
