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

#ifndef OsQueuedNotification_h__
#define OsQueuedNotification_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsgQ.h>
#include <os/OsNotification.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS
class OsMsg;

/**
 * OsQueuedNotification is designed to supersede OsQueuedEvent. OsQueuedEvent sent
 * only 1 type of message, forcing programmers to pass object pointers as intptr_t.
 * This class enables user to define what type of message should be sent, and where.
 */
class OsQueuedNotification : public OsNotification
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /**
    * Constructor. Creates notification which will send supplied message into
    * specified queue when notification is fired. Internally a copy of supplied message
    * is made that is automatically deleted.
    */
   OsQueuedNotification(OsMsgQ& rMsgQ, const OsMsg& rOsMsg);

   /**
    * Destructor. Frees copy of message.
    */
   virtual ~OsQueuedNotification();

   /* ============================ MANIPULATORS ============================== */

   /** 
    * Signal the occurrence of the event. Sends message to specified queue.
    *
    * @param eventData Ignored as its deprecated.
    */
   virtual OsStatus signal(const intptr_t eventData = 0);

   /* ============================ ACCESSORS ================================= */

   const OsMsg* getOsMsg() const { return m_pOsMsg; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   OsMsgQ& m_rMsgQ; ///< queue where we should send message when notification is signaled
   OsMsg* m_pOsMsg; ///< message that should be sent when notification is signaled
   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Copy constructor (not implemented for this class) */
   OsQueuedNotification(const OsQueuedNotification& rOsQueuedNotification);

   /** Assignment operator (not implemented for this class) */
   OsQueuedNotification& operator=(const OsQueuedNotification& rhs);
};

#endif // OsQueuedNotification_h__
