//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _SipMessageEvent_h_
#define _SipMessageEvent_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <net/SipMessage.h>
#include <os/OsMsg.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * OsMsg wrapper for transporting SipMessage instances between OsServerTasks
 */
class SipMessageEvent : public OsMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum MessageStatusTypes
   {
      APPLICATION = 0, // payload is normal sip message
      TRANSPORT_ERROR = 1,
      AUTHENTICATION_RETRY,
      SESSION_REINVITE_TIMER,
      TRANSACTION_GARBAGE_COLLECTION,
      TRANSACTION_RESEND,
      TRANSACTION_EXPIRATION
   };

   /* ============================ CREATORS ================================== */

   /** Default constructor. Uses supplied instance of SipMessage. */
   SipMessageEvent(SipMessage* message = NULL, int status = APPLICATION);

   /** Default constructor. Makes a copy of supplied sip message */
   SipMessageEvent(const SipMessage& rSipMessage, int status = APPLICATION);

   /** Destructor */
   virtual ~SipMessageEvent();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   const SipMessage* getMessage() const;
   void setMessageStatus(int status);
   int getMessageStatus() const;

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   // disable Copy constructor
   SipMessageEvent(const SipMessageEvent& rSipMessageEvent);

   // disable Assignment operator
   SipMessageEvent& operator=(const SipMessageEvent& rhs);

   SipMessage* m_pSipMessage;
   int m_messageStatus;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipMessageEvent_h_
