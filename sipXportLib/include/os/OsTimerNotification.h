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

#ifndef OsTimerNotification_h__
#define OsTimerNotification_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsgQ.h>
#include <os/OsQueuedNotification.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS
class OsTimerMsg;

/**
 * OsTimerNotification is designed to supersede OsQueuedEvent for firing timer
 * notifications. Inherits from OsQueuedNotification, but accepts only OsTimerMsg.
 *
 * When creating timer, pass this notification into its constructor along with destination
 * queue, and subclassed OsTimerMsg.
 */
class OsTimerNotification : public OsQueuedNotification
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /**
    * Constructor. Creates notification which will send supplied message into
    * specified queue when notification is fired. Internally a copy of supplied message
    * is made that is automatically deleted.
    */
   OsTimerNotification(OsMsgQ& rMsgQ, const OsTimerMsg& rMsg);

   /**
    * Destructor. Frees copy of message.
    */
   virtual ~OsTimerNotification();

   /* ============================ MANIPULATORS ============================== */

   /** 
    * Signal the occurrence of the event. Sends message to specified queue.
    *
    * @param eventData Ignored as its deprecated.
    */
   virtual OsStatus signal(const intptr_t eventData = 0);

   /* ============================ ACCESSORS ================================= */

   const OsTimerMsg* getOsTimerMsg() const { return (OsTimerMsg*)m_pOsMsg; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Copy constructor (not implemented for this class) */
   OsTimerNotification(const OsTimerNotification& rOsTimerNotification);

   /** Assignment operator (not implemented for this class) */
   OsTimerNotification& operator=(const OsTimerNotification& rhs);
};

#endif // OsTimerNotification_h__
