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

#ifndef SipXKeepaliveEventListener_h__
#define SipXKeepaliveEventListener_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsSharedServerTask.h>
#include "os/OsNatKeepaliveListener.h"
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

class SipXKeepaliveEventListener : public OsSharedServerTask, public OsNatKeepaliveListener
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   SipXKeepaliveEventListener(SIPX_INST pInst);

   virtual ~SipXKeepaliveEventListener();

   /* ============================ MANIPULATORS ============================== */

   virtual void OnKeepaliveStart(const OsNatKeepaliveEvent& event);

   virtual void OnKeepaliveStop(const OsNatKeepaliveEvent& event);

   virtual void OnKeepaliveFeedback(const OsNatKeepaliveEvent& event);

   virtual void OnKeepaliveFailure(const OsNatKeepaliveEvent& event);

   virtual UtlBoolean handleMessage(OsMsg& rRawMsg);

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   SipXKeepaliveEventListener(const SipXKeepaliveEventListener& rhs);

   SipXKeepaliveEventListener& operator=(const SipXKeepaliveEventListener& rhs);

   void handleKeepaliveEvent(SIPX_KEEPALIVE_EVENT event,
                             SIPX_KEEPALIVE_CAUSE cause,
                             SIPX_KEEPALIVE_TYPE type,
                             const char* szRemoteAddress,
                             int remotePort,
                             int keepAliveSecs,
                             const char* szFeedbackAddress,
                             int feedbackPort);

   SIPX_INST m_pInst;
};

#endif // SipXKeepaliveEventListener_h__
