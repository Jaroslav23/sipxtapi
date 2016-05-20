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

#ifndef SipXRtpRedirectEventListener_h__
#define SipXRtpRedirectEventListener_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsSharedServerTask.h>
#include <cp/CpRtpRedirectEventListener.h>
#include "tapi/sipXtapi.h"
#include <tapi/sipXtapiEvents.h>
#include <tapi/SipXCall.h>

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
* Listener for RTP redirect events
*/
class SipXRtpRedirectEventListener : public OsSharedServerTask, public CpRtpRedirectEventListener
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */

   /* ============================ CREATORS ================================== */
public:
   SipXRtpRedirectEventListener(SIPX_INST pInst);
   virtual ~SipXRtpRedirectEventListener();

   /* ============================ MANIPULATORS ============================== */

   virtual void OnRtpRedirectRequested(const CpRtpRedirectEvent& event);

   virtual void OnRtpRedirectActive(const CpRtpRedirectEvent& event);

   virtual void OnRtpRedirectError(const CpRtpRedirectEvent& event);

   virtual void OnRtpRedirectStop(const CpRtpRedirectEvent& event);

   virtual UtlBoolean handleMessage(OsMsg& rRawMsg);

   /** Fire even manually. Avoid using if possible. Let events work automatically. */
   void sipxFireRtpRedirectEvent(const UtlString& sAbstractCallId,
                                 SIPX_RTP_REDIRECT_EVENT event,
                                 SIPX_RTP_REDIRECT_CAUSE cause);

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   SipXRtpRedirectEventListener(const SipXRtpRedirectEventListener& rhs);

   SipXRtpRedirectEventListener& operator=(const SipXRtpRedirectEventListener& rhs);

   void handleRtpRedirectEvent(const UtlString& sAbstractCallId, 
                               SIPX_RTP_REDIRECT_EVENT event, 
                               SIPX_RTP_REDIRECT_CAUSE cause);

   /**
    * Gets RTP redirect state from event.
    */
   RTP_REDIRECT_STATE getRtpRedirectState(SIPX_RTP_REDIRECT_EVENT event);

   SIPX_INST m_pInst;
};

#endif // SipXRtpRedirectEventListener_h__
