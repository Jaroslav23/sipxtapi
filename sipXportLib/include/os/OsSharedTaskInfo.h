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

#ifndef OsSharedTaskInfo_h__
#define OsSharedTaskInfo_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlDefs.h>
#include <utl/UtlInt.h>
#include <os/OsSyncBase.h>
#include <os/OsMutex.h>
#include <os/OsCSem.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS
class OsSharedServerTask;

class OsSharedTaskInfo : public UtlInt, public OsSyncBase
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   OsSharedTaskInfo(const int iTaskId, OsSharedServerTask* pSharedServerTask);

   ~OsSharedTaskInfo();

   /* ============================ MANIPULATORS ============================== */

   /** Block until the sync object is acquired or the timeout expires */
   virtual OsStatus acquire(const OsTime& rTimeout = OsTime::OS_INFINITY);

   /** Conditionally acquire the semaphore (i.e., don't block) */
   virtual OsStatus tryAcquire();

   /** Release the sync object */
   virtual OsStatus release();

   /** atomically increment message count */
   void incrementMsgCount();

   /** atomically decrement message count */
   void decrementMsgCount();

   /* ============================ ACCESSORS ================================= */

   OsSharedServerTask* getSharedServerTask() const { return m_pSharedServerTask; }

   /** Gets number of messages waiting for this OsSharedTask */
   long getMsgCount() const { return m_msgCount; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsSharedTaskInfo(const OsSharedTaskInfo& rhs);

   OsSharedTaskInfo& operator=(const OsSharedTaskInfo& rhs);

   OsSharedServerTask* m_pSharedServerTask;
   mutable OsMutex m_deletionGuard; ///< this mutex helps guard against deletion with a global lock
   mutable OsMutex m_msgCountMutex; ///< guards m_msgCount
   volatile long m_msgCount; ///< number of messages waiting in queue. Needed for correct shutdown.
};

#endif // OsSharedTaskInfo_h__
