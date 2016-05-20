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

#ifndef OsSharedServerTask_h__
#define OsSharedServerTask_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMutex.h>
#include <os/OsTime.h>
#include <utl/UtlDefs.h>
#include <utl/UtlString.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS
class OsSharedServerTaskMgr;
class OsMsg;

/**
* OsSharedServerTask is a class which works in a similar manner like OsServerTask, but shares thread
* with other tasks. This class doesn't contain any threads. handleMessage is executed from
* OsSharedServerTaskMgr.
*/
class OsSharedServerTask
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   friend class OsSharedServerTaskMgr;

   /* ============================ CREATORS ================================== */

   OsSharedServerTask(const UtlString& name="");

   virtual ~OsSharedServerTask();

   /* ============================ MANIPULATORS ============================== */

   /**
    * Releases OsSharedServerTask from its current OsSharedServerTaskMgr in a thread
    * safe manner.
    */
   UtlBoolean release();

   /**
   * Override in subclass. Handles inbound messages.
   */
   virtual UtlBoolean handleMessage(OsMsg& rMsg) = 0;

   /**
    * Posts new message into queue.
    */
   OsStatus postMessage(const OsMsg& rMsg,
                        const OsTime& rTimeout = OsTime::OS_INFINITY,
                        UtlBoolean sentFromISR = FALSE);

   /* ============================ ACCESSORS ================================= */

   /**
    * Gets shared task id.
    */
   int getTaskId() const { return m_iTaskId; }

   /**
    * Gets shared task name.
    */
   UtlString getTaskName() const { return m_sTaskName; }

   /* ============================ INQUIRY =================================== */

   /**
    * Checks if this shared server task is managed. If it is managed, then messages
    * sent into it will be processed. Messages sent to unmanaged shared task will be
    * rejected.
    */
   UtlBoolean isManaged();

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsSharedServerTask(const OsSharedServerTask& rhs);

   OsSharedServerTask& operator=(const OsSharedServerTask& rhs);

   /**
    * Generates new id for task.
    */
   static int getNextTaskId();

   /**
    * Called by OsSharedServerTaskMgr when this task is released.
    */
   void taskReleased();

   /**
    * Called by OsSharedServerTaskMgr when this task is attached to it.
    */
   void taskAttached(OsSharedServerTaskMgr* pSharedServerTaskMgr);

   OsSharedServerTaskMgr* m_pSharedServerTaskMgr;
   OsMutex m_memberMutex;
   UtlString m_sTaskName;
   int m_iTaskId;

   static int m_nextId;
   static OsMutex m_idGeneratorMutex;
};

#endif // OsSharedServerTask_h__
