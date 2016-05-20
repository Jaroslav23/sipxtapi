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

#ifndef SipConnectionStateTransition_h__
#define SipConnectionStateTransition_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlDefs.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS
class BaseSipConnectionState;
class StateTransitionMemory;

/**
 * This class represents a transition between source and destination state. It is meant
 * to contain pointer to original source state (that is managed by somebody else), and
 * new allocated state. Also some memory object may be passed, that is then managed by
 * the transition. Supplied memory object is then passed to stateEntry and stateExit
 * state functions to optionally pass some simple information between state changes.
 */
class SipConnectionStateTransition
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   SipConnectionStateTransition(const BaseSipConnectionState* pSource,
                                BaseSipConnectionState* pDestination,
                                StateTransitionMemory* pMemory = NULL);

   ~SipConnectionStateTransition();

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   const BaseSipConnectionState* getSource() const { return m_pSource; }
   BaseSipConnectionState* getDestination() const { return m_pDestination; }
   const StateTransitionMemory* getMemory() const { return m_pMemory; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   const BaseSipConnectionState* m_pSource;
   BaseSipConnectionState* m_pDestination;
   StateTransitionMemory* m_pMemory;
};

#endif // SipConnectionStateTransition_h__
