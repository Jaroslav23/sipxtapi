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

#ifndef StateTransitionEventDispatcher_h__
#define StateTransitionEventDispatcher_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <cp/state/ISipConnectionState.h>
#include <cp/CpDefs.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS
class XSipConnectionEventSink;
class StateTransitionMemory;

/**
 * StateTransitionEventDispatcher is a helper class for dispatching events
 * when entering or leaving a state based on memory class. 
 */
class StateTransitionEventDispatcher
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /** Constructor */
   StateTransitionEventDispatcher(XSipConnectionEventSink& rSipConnectionEventSink,
                                  const StateTransitionMemory* pMemory);

   /** Destructor */
   ~StateTransitionEventDispatcher();

   /* ============================ MANIPULATORS ============================== */

   /**
    * Dispatches event based on state and memory contents.
    */
   void dispatchEvent(ISipConnectionState::StateEnum state) const;

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Gets common event details */
   void getCallEventDetails(CP_CALLSTATE_CAUSE& cause,
                            UtlString& originalSessionCallId,
                            int& sipResponseCode,
                            UtlString& sipResponseText) const;

   /** Converts state into call state event*/
   CP_CALLSTATE_EVENT getCallEventFromState(ISipConnectionState::StateEnum state) const;

   /** Gets call state cause from sip response code*/
   CP_CALLSTATE_CAUSE getCauseFromSipResponseCode(int sipResponseCode) const;

   XSipConnectionEventSink& m_rSipConnectionEventSink; ///< sink for firing events
   const StateTransitionMemory* m_pMemory; ///< memory object with reason for transition
};

#endif // StateTransitionEventDispatcher_h__
