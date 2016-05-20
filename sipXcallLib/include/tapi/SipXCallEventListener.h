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

#ifndef SipXCallEventListener_h__
#define SipXCallEventListener_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsSharedServerTask.h>
#include <cp/CpCallStateEventListener.h>
#include "tapi/sipXtapi.h"
#include <tapi/sipXtapiEvents.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
class SipDialog;

// STRUCTS
// TYPEDEFS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/**
* Listener for Call state events
*/
class SipXCallEventListener : public OsSharedServerTask, public CpCallStateEventListener
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */

   /* ============================ CREATORS ================================== */
public:
   SipXCallEventListener(SIPX_INST pInst);
   virtual ~SipXCallEventListener();

   /* ============================ MANIPULATORS ============================== */

   virtual void OnNewCall(const CpCallStateEvent& event);

   virtual void OnDialTone(const CpCallStateEvent& event);

   virtual void OnRemoteOffering(const CpCallStateEvent& event);

   virtual void OnRemoteAlerting(const CpCallStateEvent& event);

   virtual void OnConnected(const CpCallStateEvent& event);

   virtual void OnBridged(const CpCallStateEvent& event);

   virtual void OnHeld(const CpCallStateEvent& event);

   virtual void OnRemoteHeld(const CpCallStateEvent& event);

   virtual void OnDisconnected(const CpCallStateEvent& event);

   virtual void OnOffering(const CpCallStateEvent& event);

   virtual void OnAlerting(const CpCallStateEvent& event);

   virtual void OnDestroyed(const CpCallStateEvent& event);

   virtual void OnTransferEvent(const CpCallStateEvent& event);

   virtual UtlBoolean handleMessage(OsMsg& rRawMsg);

   /** Fire call even manually. Avoid using if possible. Let events work automatically. */
   void sipxFireCallEvent(const UtlString& sCallId,
                          const SipDialog* pSipDialog,
                          SIPX_CALLSTATE_EVENT event,
                          SIPX_CALLSTATE_CAUSE cause,
                          const UtlString& sOriginalSessionCallId = NULL,
                          int sipResponseCode = 0,
                          const UtlString& sResponseText = NULL);

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   SipXCallEventListener(const SipXCallEventListener& rhs);

   SipXCallEventListener& operator=(const SipXCallEventListener& rhs);

   void handleCallEvent(const UtlString& sCallId, 
                        const SipDialog* pSipDialog, 
                        SIPX_CALLSTATE_EVENT event, 
                        SIPX_CALLSTATE_CAUSE cause, 
                        const UtlString& sOriginalSessionCallId,
                        int sipResponseCode,
                        const UtlString& sResponseText,
                        const UtlString& sReferredBy = NULL,
                        const UtlString& sReferTo = NULL);

   SIPX_INST m_pInst;
};

#endif // SipXCallEventListener_h__
