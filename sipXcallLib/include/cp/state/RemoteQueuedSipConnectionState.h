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

#ifndef RemoteQueuedSipConnectionState_h__
#define RemoteQueuedSipConnectionState_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <cp/state/BaseSipConnectionState.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * Class representing remote queued state.
 */
class RemoteQueuedSipConnectionState : public BaseSipConnectionState
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /** Constructor. */
   RemoteQueuedSipConnectionState(SipConnectionStateContext& rStateContext,
                                  SipUserAgent& rSipUserAgent,
                                  XCpCallControl& rCallControl,
                                  CpMediaInterfaceProvider* pMediaInterfaceProvider,
                                  CpMessageQueueProvider* pMessageQueueProvider,
                                  XSipConnectionEventSink& rSipConnectionEventSink,
                                  const CpNatTraversalConfig& natTraversalConfig);

   /** Constructor. */
   RemoteQueuedSipConnectionState(const BaseSipConnectionState& rhs);

   /** Destructor. */
   virtual ~RemoteQueuedSipConnectionState();

   /* ============================ MANIPULATORS ============================== */

   /**
   * State entry handler.
   */
   virtual void handleStateEntry(StateEnum previousState, const StateTransitionMemory* pTransitionMemory);

   /**
   * State exit handler.
   */
   virtual void handleStateExit(StateEnum nextState, const StateTransitionMemory* pTransitionMemory);

   /** Disconnects call */
   virtual SipConnectionStateTransition* dropConnection(OsStatus& result);

   virtual SipConnectionStateTransition* handleSipMessageEvent(const SipMessageEvent& rEvent);

   /** Handles inbound SIP INVITE responses */
   virtual SipConnectionStateTransition* processInviteResponse(const SipMessage& sipMessage);

   /* ============================ ACCESSORS ================================= */

   virtual ISipConnectionState::StateEnum getCurrentState() const { return ISipConnectionState::CONNECTION_REMOTE_QUEUED; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   /**
   * Constructs transition from current state into given destination state.
   * @pTansitionMemory Optional memory object which should be supplied to state transition. Local copy
   * will be made.
   */
   virtual SipConnectionStateTransition* getTransition(ISipConnectionState::StateEnum nextState,
                                                       const StateTransitionMemory* pTansitionMemory) const;

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

#endif // RemoteQueuedSipConnectionState_h__
