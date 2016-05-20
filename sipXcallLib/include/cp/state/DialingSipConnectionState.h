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

#ifndef DialingSipConnectionState_h__
#define DialingSipConnectionState_h__

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
 * Class representing dialing state.
 */
class DialingSipConnectionState : public BaseSipConnectionState
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /** Constructor. */
   DialingSipConnectionState(SipConnectionStateContext& rStateContext,
                             SipUserAgent& rSipUserAgent,
                             XCpCallControl& rCallControl,
                             CpMediaInterfaceProvider* pMediaInterfaceProvider,
                             CpMessageQueueProvider* pMessageQueueProvider,
                             XSipConnectionEventSink& rSipConnectionEventSink,
                             const CpNatTraversalConfig& natTraversalConfig);

   /** Constructor. */
   DialingSipConnectionState(const BaseSipConnectionState& rhs);

   /** Destructor. */
   virtual ~DialingSipConnectionState();

   /* ============================ MANIPULATORS ============================== */

   /**
   * State entry handler.
   */
   virtual void handleStateEntry(StateEnum previousState, const StateTransitionMemory* pTransitionMemory);

   /**
   * State exit handler.
   */
   virtual void handleStateExit(StateEnum nextState, const StateTransitionMemory* pTransitionMemory);

   virtual SipConnectionStateTransition* handleSipMessageEvent(const SipMessageEvent& rEvent);

   /** Connects call to given address. Uses supplied sip call-id. */
   virtual SipConnectionStateTransition* connect(OsStatus& result,
                                                 const UtlString& sipCallId,
                                                 const UtlString& localTag,
                                                 const UtlString& toAddress,
                                                 const UtlString& fromAddress,
                                                 const UtlString& locationHeader,
                                                 CP_CONTACT_ID contactId,
                                                 SIP_TRANSPORT_TYPE transport,
                                                 const UtlString& replacesField);

   /** Disconnects call */
   virtual SipConnectionStateTransition* dropConnection(OsStatus& result);

   /* ============================ ACCESSORS ================================= */

   virtual ISipConnectionState::StateEnum getCurrentState() const { return ISipConnectionState::CONNECTION_DIALING; }

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

#endif // DialingSipConnectionState_h__
