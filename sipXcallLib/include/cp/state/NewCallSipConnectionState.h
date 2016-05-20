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

#ifndef NewCallSipConnectionState_h__
#define NewCallSipConnectionState_h__

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
 * Class representing newcall state.
 */
class NewCallSipConnectionState : public BaseSipConnectionState
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /** Constructor. */
   NewCallSipConnectionState(SipConnectionStateContext& rStateContext,
                             SipUserAgent& rSipUserAgent,
                             XCpCallControl& rCallControl,
                             CpMediaInterfaceProvider* pMediaInterfaceProvider,
                             CpMessageQueueProvider* pMessageQueueProvider,
                             XSipConnectionEventSink& rSipConnectionEventSink,
                             const CpNatTraversalConfig& natTraversalConfig);

   /** Constructor. */
   NewCallSipConnectionState(const BaseSipConnectionState& rhs);

   /** Destructor. */
   virtual ~NewCallSipConnectionState();

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

   /** Handles initial INVITE request */
   virtual SipConnectionStateTransition* processInviteRequest(const SipMessage& sipMessage);

   /* ============================ ACCESSORS ================================= */

   virtual ISipConnectionState::StateEnum getCurrentState() const { return ISipConnectionState::CONNECTION_NEWCALL; }

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

   /**
   * Must be called for inbound calls to progress to early established dialog, which results in local tag being
   * generated.
   */
   void progressToEarlyEstablishedDialog();

};

#endif // NewCallSipConnectionState_h__
