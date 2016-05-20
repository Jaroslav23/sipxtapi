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

#ifndef EstablishedSipConnectionState_h__
#define EstablishedSipConnectionState_h__

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
 * Class representing established state.
 */
class EstablishedSipConnectionState : public BaseSipConnectionState
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /** Constructor. */
   EstablishedSipConnectionState(SipConnectionStateContext& rStateContext,
                                 SipUserAgent& rSipUserAgent,
                                 XCpCallControl& rCallControl,
                                 CpMediaInterfaceProvider* pMediaInterfaceProvider,
                                 CpMessageQueueProvider* pMessageQueueProvider,
                                 XSipConnectionEventSink& rSipConnectionEventSink,
                                 const CpNatTraversalConfig& natTraversalConfig);

   /** Constructor. */
   EstablishedSipConnectionState(const BaseSipConnectionState& rhs);

   /** Destructor. */
   virtual ~EstablishedSipConnectionState();

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

   /** Handles inbound SIP re-INVITE requests. It won't handle initial INVITEs. */
   virtual SipConnectionStateTransition* processInviteRequest(const SipMessage& sipMessage);

   /** Handles inbound SIP BYE requests */
   virtual SipConnectionStateTransition* processByeRequest(const SipMessage& sipMessage);

   virtual SipConnectionStateTransition* handleSipMessageEvent(const SipMessageEvent& rEvent);

   /** Handles inbound SIP INVITE responses */
   virtual SipConnectionStateTransition* processInviteResponse(const SipMessage& sipMessage);

   /* ============================ ACCESSORS ================================= */

   virtual ISipConnectionState::StateEnum getCurrentState() const { return ISipConnectionState::CONNECTION_ESTABLISHED; }

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

#endif // EstablishedSipConnectionState_h__
