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

#ifndef ScReInviteTimerMsg_h__
#define ScReInviteTimerMsg_h__

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
 * ScReInviteTimerMsg is sent when re-INVITE timer fires. It is meant to trigger
 * sending re-INVITE message (session refresh, codec negotiation) 
 */
class ScReInviteTimerMsg : public ScTimerMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /**
    * There are several reasons why re-INVITE/UPDATE is needed. This reason is sent
    * as part of ScReInviteTimerMsg.
    * REASON_NORMAL, REASON_HOLD, REASON_UNHOLD - always renegotiate codecs
    * REASON_SESSION_EXTENSION - only re-INVITE renegotiates codecs, UPDATE doesn't
    */
   typedef enum
   {
      REASON_NORMAL, ///< re-INVITE to renegotiate codecs, hold/unhold. Failure will not cause problem.
      REASON_HOLD,
      REASON_UNHOLD,
      REASON_SESSION_EXTENSION ///< re-INVITE to extend session (after session timer fires)
   } ReInviteReason;

   /* ============================ CREATORS ================================== */

   /**
    * Constructor.
    */
   ScReInviteTimerMsg(ReInviteReason reason,
                      const UtlString& sCallId,
                      const UtlString& sLocalTag,
                      const UtlString& sRemoteTag,
                      UtlBoolean isFromLocal = TRUE);

   /** Copy constructor */
   ScReInviteTimerMsg(const ScReInviteTimerMsg& rhs);

   /** Create a copy of this msg object (which may be of a derived type) */
   virtual OsMsg* createCopy(void) const;

   /** Destructor. */
   virtual ~ScReInviteTimerMsg();

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   ScReInviteTimerMsg& operator=(const ScReInviteTimerMsg& rhs);

   /* ============================ ACCESSORS ================================= */

   ScReInviteTimerMsg::ReInviteReason getReason() const { return m_reason; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   ReInviteReason m_reason; ///< m_reason for re-INVITE
   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

#endif // ScReInviteTimerMsg_h__
