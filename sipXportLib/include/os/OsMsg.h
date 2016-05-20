//
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsMsg_h_
#define _OsMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "utl/UtlContainable.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Base class for message queue buffers

class OsMsg : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum MsgTypes
   {
      UNSPECIFIED = 0,
      OS_SHUTDOWN,           // Task shutdown request message
      OS_TIMERTASK_COMMAND,  // Timer request messages. Internal for OsTimerTask.
      OS_TIMER_MSG,          // Message sent when timer fires
      OS_EVENT,              // Event notification messages, old message type when timer fires for OsQueuedEvent
      OS_STUN_RESULT_MSG,     // STUN result message
      PHONE_APP,             // Phone application messages
      MP_TASK_MSG,           // Media processing task messages
      MP_FLOWGRAPH_MSG,      // Media processing flowgraph messages
      MP_RESOURCE_MSG,       // Media resource messages
      MP_CONNECTION_NOTF_MSG,// Media connection notification messages
      MP_INTERFACE_NOTF_MSG, // Media interface notification messages
      MP_BUFFER_MSG,         // Media processing buffer queue messages
      OS_SYSLOG,             // OS SysLog events
      STREAMING_MSG,         // Streaming related messages
      USER_START  = 128
   };

   static const UtlContainableType TYPE ;    /** < Class type used for runtime checking */
/* ============================ CREATORS ================================== */

   OsMsg(const unsigned char msgType, const unsigned char msgSubType);
     //:Constructor

   OsMsg(const OsMsg& rOsMsg);
     //:Copy constructor

   virtual OsMsg* createCopy(void) const;
     //:Create a copy of this msg object (which may be of a derived type)

   virtual void releaseMsg(void);
     //:Done with message, delete it or mark it unused

   virtual
      ~OsMsg();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsMsg& operator=(const OsMsg& rhs);
     //:Assignment operator

   virtual void setMsgSubType(unsigned char subType);
     //:Set the message subtype

   virtual void setSentFromISR(UtlBoolean sentFromISR);
     //:Set the SentFromISR (interrupt service routine) flag

   virtual void setReusable(UtlBoolean isReusable);
     //:Set the Is Reusable (from permanent pool) flag

   virtual void setInUse(UtlBoolean isInUse);
     //:Set the Is In Use flag

/* ============================ ACCESSORS ================================= */

   virtual unsigned char getMsgType(void) const;
     //:Return the message type

   virtual unsigned char getMsgSubType(void) const;
     //:Return the message subtype

   virtual int getMsgSize(void) const;
     //:Return the size of the message in bytes
     // This is a virtual method so that it will return the accurate size for
     // the message object even if that object has been upcast to the type of
     // an ancestor class.

   virtual UtlBoolean getSentFromISR(void) const;
     //:Return TRUE if msg was sent from an interrupt svc routine, else FALSE

   virtual UtlBoolean isMsgReusable(void) const;
     //:Return TRUE if msg is from a permanent pool, else FALSE

   virtual UtlBoolean isMsgInUse(void) const;
     //:Return TRUE if msg is currently in use, else FALSE

   //! Implements the interface for a UtlContainable
   virtual unsigned hash() const;

   virtual UtlContainableType getContainableType() const;

   virtual int compareTo(UtlContainable const *) const;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   unsigned char mMsgType;
   unsigned char mMsgSubType;
   UtlBoolean     mSentFromISR;
   UtlBoolean     mReusable;
   UtlBoolean     mInUse;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsMsg_h_
