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

#ifndef SipConnectionStateObserver_h__
#define SipConnectionStateObserver_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <cp/state/ISipConnectionState.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * Observer for sip connection state machine
 */
class SipConnectionStateObserver
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /* ============================ MANIPULATORS ============================== */

   /**
   * Called when we enter new state. This is typically called after we handle
   * SipMessageEvent, resulting in new state transition.
   */
   virtual void handleStateEntry(ISipConnectionState::StateEnum state) = 0;

   /**
   * Called when we progress to new state, before old state is destroyed.
   */
   virtual void handleStateExit(ISipConnectionState::StateEnum state) = 0;

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // SipConnectionStateObserver_h__
