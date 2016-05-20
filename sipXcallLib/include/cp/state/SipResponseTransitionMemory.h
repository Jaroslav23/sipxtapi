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

#ifndef SipResponseTransitionMemory_h__
#define SipResponseTransitionMemory_h__

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
 * SipResponseTransitionMemory keeps SIP response code and text.
 */
class SipResponseTransitionMemory : public StateTransitionMemory
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /** Constructor. */
   SipResponseTransitionMemory(int sipResponseCode, const UtlString& sipResponseText);

   /** Destructor. */
   virtual ~SipResponseTransitionMemory();

   /* ============================ MANIPULATORS ============================== */

   /** Creates copy of transition memory. */
   virtual StateTransitionMemory* clone() const;

   /* ============================ ACCESSORS ================================= */

   int getSipResponseCode() const { return m_sipResponseCode; }
   UtlString getSipResponseText() const { return m_sipResponseText; }

   virtual StateTransitionMemory::Type getType() const
   {
      return SIP_RESPONSE_MEMORY;
   }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   int m_sipResponseCode; ///< stores sip response code
   UtlString m_sipResponseText; ///< stores response text
};

#endif // SipResponseTransitionMemory_h__
