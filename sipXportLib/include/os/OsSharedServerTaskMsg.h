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

#ifndef OsSharedServerTaskMsg_h__
#define OsSharedServerTaskMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>

// DEFINES
#define OS_SHARED_SERVER_TASK_MSG OsMsg::USER_START + 1

// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS
class OsSharedTaskInfo;

/**
 * Message that wraps another OsMsg inside it. Doesn't clone internal pMsg to avoid
 * excessive cloning (once when this message is constructed, and 2nd time when it is
 * cloned when inserted into OsMsgQ).
 */
class OsSharedServerTaskMsg : public OsMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   OsSharedServerTaskMsg(OsSharedTaskInfo* pTaskInfo, OsMsg* pMsg = NULL);

   virtual ~OsSharedServerTaskMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   OsMsg* getMsg() const { return m_pMsg; }

   OsSharedTaskInfo* getTaskInfo() const { return m_pTaskInfo; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsSharedServerTaskMsg(const OsSharedServerTaskMsg& rhs);

   OsSharedServerTaskMsg& operator=(const OsSharedServerTaskMsg& rhs);

   OsMsg* m_pMsg; ///< message that needs to be delivered
   OsSharedTaskInfo* m_pTaskInfo; ///< information about task this message is destined for
};

#endif // OsSharedServerTaskMsg_h__
