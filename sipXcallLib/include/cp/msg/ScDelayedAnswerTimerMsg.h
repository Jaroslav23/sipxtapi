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

#ifndef ScDelayedAnswerTimerMsg_h__
#define ScDelayedAnswerTimerMsg_h__

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
 * ScDelayedAnswerTimerMsg is meant to trigger answering call later.
 */
class ScDelayedAnswerTimerMsg : public ScTimerMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /**
    * Constructor.
    */
   ScDelayedAnswerTimerMsg(const UtlString& sCallId,
                           const UtlString& sLocalTag,
                           const UtlString& sRemoteTag,
                           UtlBoolean isFromLocal = TRUE);

   /** Copy constructor */
   ScDelayedAnswerTimerMsg(const ScDelayedAnswerTimerMsg& rhs);

   /** Create a copy of this msg object (which may be of a derived type) */
   virtual OsMsg* createCopy(void) const;

   /** Destructor. */
   virtual ~ScDelayedAnswerTimerMsg();

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   ScDelayedAnswerTimerMsg& operator=(const ScDelayedAnswerTimerMsg& rhs);

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

#endif // ScDelayedAnswerTimerMsg_h__
