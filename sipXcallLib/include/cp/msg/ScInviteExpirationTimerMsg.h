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

#ifndef ScInviteExpirationTimerMsg_h__
#define ScInviteExpirationTimerMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <cp/msg/ScTimerMsg.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * ScInviteExpirationTimerMsg is meant to trigger check if session timeout occurred (session timer support).
 */
class ScInviteExpirationTimerMsg : public ScTimerMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /**
    * Constructor.
    */
   ScInviteExpirationTimerMsg(int cseqNum,
                              UtlBoolean bIsOutbound,
                              const UtlString& sCallId,
                              const UtlString& sLocalTag,
                              const UtlString& sRemoteTag,
                              UtlBoolean isFromLocal = TRUE);

   /** Copy constructor */
   ScInviteExpirationTimerMsg(const ScInviteExpirationTimerMsg& rhs);

   /** Create a copy of this msg object (which may be of a derived type) */
   virtual OsMsg* createCopy(void) const;

   /** Destructor. */
   virtual ~ScInviteExpirationTimerMsg();

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   ScInviteExpirationTimerMsg& operator=(const ScInviteExpirationTimerMsg& rhs);

   /* ============================ ACCESSORS ================================= */

   int getCseqNum() const { return m_cseqNum; }
   UtlBoolean getIsOutbound() const { return m_bIsOutbound; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   int m_cseqNum;
   UtlBoolean m_bIsOutbound;
};

#endif // ScInviteExpirationTimerMsg_h__
