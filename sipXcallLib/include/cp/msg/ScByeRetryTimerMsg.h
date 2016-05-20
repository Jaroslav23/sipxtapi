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

#ifndef ScByeRetryTimerMsg_h__
#define ScByeRetryTimerMsg_h__

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
 * ScByeRetryTimerMsg is meant to trigger trying to send BYE again for inbound call
 * that hasn't received ACK yet. We basically test if ACK was received, and if yes
 * then send the BYE.  
 */
class ScByeRetryTimerMsg : public ScTimerMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /* ============================ CREATORS ================================== */

   /**
    * Constructor.
    */
   ScByeRetryTimerMsg(const UtlString& sCallId,
                      const UtlString& sLocalTag,
                      const UtlString& sRemoteTag,
                      UtlBoolean isFromLocal = TRUE);

   /** Copy constructor */
   ScByeRetryTimerMsg(const ScByeRetryTimerMsg& rhs);

   /** Create a copy of this msg object (which may be of a derived type) */
   virtual OsMsg* createCopy(void) const;

   /** Destructor. */
   virtual ~ScByeRetryTimerMsg();

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   ScByeRetryTimerMsg& operator=(const ScByeRetryTimerMsg& rhs);

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

#endif // ScByeRetryTimerMsg_h__
