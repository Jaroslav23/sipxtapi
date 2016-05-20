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

#ifndef ScDisconnectTimerMsg_h__
#define ScDisconnectTimerMsg_h__

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
 * ScDisconnectTimerMsg is meant to trigger transition into disconnected connection state,
 * unless we are already disconnected. 
 */
class ScDisconnectTimerMsg : public ScTimerMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   typedef enum
   {
      REASON_BYE_TIMEOUT,
      REASON_CANCEL_TIMEOUT
   } DisconnectReason;

   /* ============================ CREATORS ================================== */

   /**
    * Constructor.
    */
   ScDisconnectTimerMsg(DisconnectReason reason,
                        const UtlString& sCallId,
                        const UtlString& sLocalTag,
                        const UtlString& sRemoteTag,
                        UtlBoolean isFromLocal = TRUE);

   /** Copy constructor */
   ScDisconnectTimerMsg(const ScDisconnectTimerMsg& rhs);

   /** Create a copy of this msg object (which may be of a derived type) */
   virtual OsMsg* createCopy(void) const;

   /** Destructor. */
   virtual ~ScDisconnectTimerMsg();

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   ScDisconnectTimerMsg& operator=(const ScDisconnectTimerMsg& rhs);

   /* ============================ ACCESSORS ================================= */

   ScDisconnectTimerMsg::DisconnectReason getReason() const { return m_reason; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   DisconnectReason m_reason; ///< reason for disconnecting
   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

#endif // ScDisconnectTimerMsg_h__
