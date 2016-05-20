//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
//
// Copyright (C) 2005-2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef SipXLineEventListener_h__
#define SipXLineEventListener_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsSharedServerTask.h>
#include <net/SipLineStateEventListener.h>
#include "tapi/sipXtapi.h"
#include <tapi/sipXtapiEvents.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
// STRUCTS
// TYPEDEFS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/**
* Listener for Line state events
*/
class SipXLineEventListener : public OsSharedServerTask, public SipLineStateEventListener
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */

   /* ============================ CREATORS ================================== */
public:
   SipXLineEventListener(SIPX_INST pInst);
   virtual ~SipXLineEventListener();

   /* ============================ MANIPULATORS ============================== */

   virtual void OnLineRegistering(const SipLineStateEvent& event);

   virtual void OnLineRegistered(const SipLineStateEvent& event);

   virtual void OnLineUnregistering(const SipLineStateEvent& event);

   virtual void OnLineUnregistered(const SipLineStateEvent& event);

   virtual void OnLineRegisterFailed(const SipLineStateEvent& event);

   virtual void OnLineUnregisterFailed(const SipLineStateEvent& event);

   virtual void OnLineProvisioned(const SipLineStateEvent& event);

   virtual UtlBoolean handleMessage(OsMsg& rRawMsg);

   void sipxFireLineEvent(const UtlString& lineIdentifier,
                          SIPX_LINESTATE_EVENT event,
                          SIPX_LINESTATE_CAUSE cause,
                          int sipResponseCode = 0,
                          const UtlString& sResponseText = NULL);

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   SipXLineEventListener(const SipXLineEventListener& rhs);

   SipXLineEventListener& operator=(const SipXLineEventListener& rhs);

   void handleLineEvent(const UtlString& lineIdentifier,
                        SIPX_LINESTATE_EVENT event,
                        SIPX_LINESTATE_CAUSE cause,
                        int sipResponseCode,
                        const UtlString& sResponseText);

   SIPX_INST m_pInst;
};

#endif // SipXLineEventListener_h__
