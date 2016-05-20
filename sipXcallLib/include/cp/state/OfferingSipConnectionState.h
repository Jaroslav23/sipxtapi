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

#ifndef OfferingSipConnectionState_h__
#define OfferingSipConnectionState_h__

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
 * Class representing offering state.
 */
class OfferingSipConnectionState : public BaseSipConnectionState
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /** Constructor. */
   OfferingSipConnectionState(SipConnectionStateContext& rStateContext,
                              SipUserAgent& rSipUserAgent,
                              XCpCallControl& rCallControl,
                              CpMediaInterfaceProvider* pMediaInterfaceProvider,
                              CpMessageQueueProvider* pMessageQueueProvider,
                              XSipConnectionEventSink& rSipConnectionEventSink,
                              const CpNatTraversalConfig& natTraversalConfig);

   /** Constructor. */
   OfferingSipConnectionState(const BaseSipConnectionState& rhs);

   /** Destructor. */
   virtual ~OfferingSipConnectionState();

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

   /* ============================ ACCESSORS ================================= */

   virtual ISipConnectionState::StateEnum getCurrentState() const { return ISipConnectionState::CONNECTION_OFFERING; }

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

#endif // OfferingSipConnectionState_h__
