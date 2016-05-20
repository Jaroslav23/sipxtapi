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

#ifndef Sc100RelTimerMsg_h__
#define Sc100RelTimerMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <net/SipMessage.h>
#include <cp/msg/ScTimerMsg.h>
#include <cp/Cp100RelTracker.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * Sc100RelTimerMsg is message that gets sent when 100rel response should be retransmitted.
 * This should be done every T1 (500ms) up to 64 times until a PRACK is received. 
 */
class Sc100RelTimerMsg : public ScTimerMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /**
    * Constructor.
    */
   Sc100RelTimerMsg(const SipMessage& c100relResponse,
                    const UtlString& sCallId,
                    const UtlString& sLocalTag,
                    const UtlString& sRemoteTag,
                    UtlBoolean isFromLocal = TRUE);

   /** Copy constructor */
   Sc100RelTimerMsg(const Sc100RelTimerMsg& rhs);

   /** Create a copy of this msg object (which may be of a derived type) */
   virtual OsMsg* createCopy(void) const;

   /** Destructor. */
   virtual ~Sc100RelTimerMsg();

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   Sc100RelTimerMsg& operator=(const Sc100RelTimerMsg& rhs);

   /* ============================ ACCESSORS ================================= */

   UtlString get100RelId() const { return Cp100RelTracker::get100RelId(m_100relResponse); }
   void get100relResponse(SipMessage& response) const { response = m_100relResponse; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   SipMessage m_100relResponse;
   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

#endif // Sc100RelTimerMsg_h__
