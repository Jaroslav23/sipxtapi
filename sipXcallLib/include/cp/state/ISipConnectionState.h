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

#ifndef ISipConnectionState_h__
#define ISipConnectionState_h__

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
class SipMessageEvent;
class StateTransitionMemory;

/**
 * Interface for all connection states.
 */
class ISipConnectionState
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /**
    * Codes for possible connection states.
    */
   typedef enum
   {
      CONNECTION_IDLE = 0, ///< initial state of state machine, is switched to CONNECTION_NEWCALL or CONNECTION_DIALING
      CONNECTION_NEWCALL, ///< initial state for inbound calls
      CONNECTION_DIALING, ///< initial state for outbound calls
      CONNECTION_REMOTE_QUEUED, ///< for outbound calls
      CONNECTION_REMOTE_OFFERING, ///< for outbound calls
      CONNECTION_REMOTE_ALERTING, ///< for outbound calls
      CONNECTION_QUEUED, ///< for inbound calls
      CONNECTION_OFFERING, ///< for inbound calls
      CONNECTION_ALERTING, ///< for inbound calls
      CONNECTION_ESTABLISHED,
      CONNECTION_DISCONNECTED, ///< reached when call is hang up, refused by remote party etc
      CONNECTION_UNKNOWN ///< this state should never occur, it is only theoretical
   } StateEnum;

   /* ============================ CREATORS ================================== */

   /* ============================ MANIPULATORS ============================== */

   /**
    * Called when we enter new state. This is typically called after we handle
    * SipMessageEvent, resulting in new state transition. Should not be used to
    * send any sip messages.
    */
   virtual void handleStateEntry(StateEnum previousState, const StateTransitionMemory* pTransitionMemory) = 0;

   /**
    * Called before this state is destroyed. Should not be used to send any sip messages.
    */
   virtual void handleStateExit(StateEnum nextState, const StateTransitionMemory* pTransitionMemory) = 0;

   /* ============================ ACCESSORS ================================= */

   /**
    * Gets id of current state.
    */
   virtual ISipConnectionState::StateEnum getCurrentState() const = 0;

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // ISipConnectionState_h__
