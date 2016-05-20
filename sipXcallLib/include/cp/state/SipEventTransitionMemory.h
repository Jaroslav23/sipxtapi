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

#ifndef SipEventTransitionMemory_h__
#define SipEventTransitionMemory_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <cp/state/StateTransitionMemory.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * SipEventTransitionMemory keeps status code from SipMessageEvent
 */
class SipEventTransitionMemory : public StateTransitionMemory
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /** Constructor. */
   SipEventTransitionMemory(int sipMessageStatus);

   /** Destructor. */
   virtual ~SipEventTransitionMemory();

   /* ============================ MANIPULATORS ============================== */

   /** Creates copy of transition memory. */
   virtual StateTransitionMemory* clone() const;

   /* ============================ ACCESSORS ================================= */

   virtual StateTransitionMemory::Type getType() const
   {
      return SIP_MESSAGE_EVENT_MEMORY;
   }

   int getSipMessageStatus() const { return m_sipMessageStatus; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   int m_sipMessageStatus; ///< stores sip message status code from SipMessageEvent
};

#endif // SipEventTransitionMemory_h__
