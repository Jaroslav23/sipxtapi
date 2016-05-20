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

#ifndef Sc2xxTimerMsg_h__
#define Sc2xxTimerMsg_h__

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
 * Sc2xxTimerMsg is meant to trigger retransmission of 2xx response to invite.
 */
class Sc2xxTimerMsg : public ScTimerMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /**
    * Constructor.
    */
   Sc2xxTimerMsg(const UtlString& sCallId,
                 const UtlString& sLocalTag,
                 const UtlString& sRemoteTag,
                 UtlBoolean isFromLocal = TRUE);

   /** Copy constructor */
   Sc2xxTimerMsg(const Sc2xxTimerMsg& rhs);

   /** Create a copy of this msg object (which may be of a derived type) */
   virtual OsMsg* createCopy(void) const;

   /** Destructor. */
   virtual ~Sc2xxTimerMsg();

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   Sc2xxTimerMsg& operator=(const Sc2xxTimerMsg& rhs);

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

#endif // Sc2xxTimerMsg_h__
