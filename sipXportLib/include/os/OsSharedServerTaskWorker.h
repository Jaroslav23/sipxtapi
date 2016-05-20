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

#ifndef OsSharedServerTaskWorker_h__
#define OsSharedServerTaskWorker_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlDefs.h>
#include <utl/UtlInt.h>
#include <os/OsDefs.h>
#include <os/OsTask.h>
#include <os/OsMsgQ.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS
class OsSharedServerTaskMsg;
class OsSharedServerTaskMgr;

/**
* OsSharedServerTaskWorker is a thread processing messages from OsSharedServerTaskMgr
* for OsSharedServerTasks.
*/
class OsSharedServerTaskWorker : public OsTask, public UtlInt
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   friend class OsSharedServerTaskMgr;

   /* ============================ CREATORS ================================== */

   OsSharedServerTaskWorker(const UtlString& name= "",
                            const int workerId = 0, // needed for putting worker in UtlContainers
                            void* pArg = NULL,
                            OsMsgQ* pMsgQ = NULL,
                            const int priority = DEF_PRIO,
                            const int options = DEF_OPTIONS,
                            const int stackSize = DEF_STACKSIZE);

   virtual ~OsSharedServerTaskWorker();

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   /**
   * Sends a shutdown request to task. Protected as this thread doesn't have its own
   * queue, and it shouldn't be possible to shut it down from outside.
   */
   virtual void requestShutdown(void);

   /**
    * Method executed from thread context.
    */
   virtual int run(void* pArg);

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlBoolean handleMessage(OsMsg& rMsg);
   UtlBoolean handleOsSharedServerTaskMessage(OsSharedServerTaskMsg& rMsg);

   OsMsgQ* m_pMsgQ;
};

#endif // OsSharedServerTaskWorker_h__
