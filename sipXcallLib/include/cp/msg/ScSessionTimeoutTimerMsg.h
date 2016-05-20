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

#ifndef ScSessionTimeoutTimerMsg_h__
#define ScSessionTimeoutTimerMsg_h__

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
 * ScSessionTimeoutTimerMsg is meant to trigger check if session timeout occurred (session timer support).
 */
class ScSessionTimeoutTimerMsg : public ScTimerMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /**
    * Constructor.
    */
   ScSessionTimeoutTimerMsg(const UtlString& sCallId,
                            const UtlString& sLocalTag,
                            const UtlString& sRemoteTag,
                            UtlBoolean isFromLocal = TRUE);

   /** Copy constructor */
   ScSessionTimeoutTimerMsg(const ScSessionTimeoutTimerMsg& rhs);

   /** Create a copy of this msg object (which may be of a derived type) */
   virtual OsMsg* createCopy(void) const;

   /** Destructor. */
   virtual ~ScSessionTimeoutTimerMsg();

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   ScSessionTimeoutTimerMsg& operator=(const ScSessionTimeoutTimerMsg& rhs);

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

#endif // ScSessionTimeoutTimerMsg_h__
