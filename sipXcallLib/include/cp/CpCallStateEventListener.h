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

#ifndef CpCallStateEventListener_h__
#define CpCallStateEventListener_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <net/SipDialog.h>
#include <cp/CpDefs.h>

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

class CpCallStateEvent
{
public:
   UtlString m_sCallId; // id of abstract call
   SipDialog* m_pSipDialog;
   CP_CALLSTATE_CAUSE m_cause;
   UtlString m_sOriginalSessionCallId; // callId supplied sometimes for transfer events
   int m_sipResponseCode;
   UtlString m_sResponseText;
   UtlString m_sReferredBy;
   UtlString m_sReferTo;

   CpCallStateEvent(const UtlString& sCallId,
      const SipDialog* pSipDialog,
      CP_CALLSTATE_CAUSE cause,
      const UtlString& sOriginalSessionCallId = NULL,
      int sipResponseCode = 0,
      const UtlString& sResponseText = NULL,
      const UtlString& sReferredBy = NULL,
      const UtlString& sReferTo = NULL)
      : m_sCallId(sCallId),
      m_pSipDialog(NULL),
      m_cause(cause),
      m_sOriginalSessionCallId(sOriginalSessionCallId),
      m_sipResponseCode(sipResponseCode),
      m_sResponseText(sResponseText),
      m_sReferredBy(sReferredBy),
      m_sReferTo(sReferTo)
   {
      if (pSipDialog)
      {
         // create copy of sip dialog
         m_pSipDialog = new SipDialog(*pSipDialog);
      }
   }

   CpCallStateEvent()
      : m_pSipDialog(NULL)
      , m_sipResponseCode(0)
      , m_cause(CP_CALLSTATE_CAUSE_UNKNOWN)
   {
   }

   ~CpCallStateEvent()
   {
      if (m_pSipDialog)
      {
         delete m_pSipDialog;
         m_pSipDialog = NULL;
      }
   }

   CpCallStateEvent(const CpCallStateEvent& event)
      : m_pSipDialog(NULL)
   {
      *this = event;
   }

   CpCallStateEvent& operator=(const CpCallStateEvent& event)
   {
      if (&event == this)
      {
         return *this;
      }

      m_sCallId = event.m_sCallId;
      // get rid of old sip dialog if it exists
      if (m_pSipDialog)
      {
         delete m_pSipDialog;
         m_pSipDialog = NULL;
      }
      if (event.m_pSipDialog)
      {
         // copy new sip dialog
         m_pSipDialog = new SipDialog(*(event.m_pSipDialog));
      }
      m_cause = event.m_cause;
      m_sOriginalSessionCallId = event.m_sOriginalSessionCallId;
      m_sipResponseCode = event.m_sipResponseCode;
      m_sResponseText = event.m_sResponseText;
      m_sReferredBy = event.m_sReferredBy;
      m_sReferTo = event.m_sReferTo;

      return *this;
   }
};


/**
* Listener for Call state events
*/
class CpCallStateEventListener
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */

   /* ============================ CREATORS ================================== */
public:
   CpCallStateEventListener() {}
   virtual ~CpCallStateEventListener() {}

   /* ============================ MANIPULATORS ============================== */

   virtual void OnNewCall(const CpCallStateEvent& event) = 0;

   virtual void OnDialTone(const CpCallStateEvent& event) = 0;

   virtual void OnRemoteOffering(const CpCallStateEvent& event) = 0;

   virtual void OnRemoteAlerting(const CpCallStateEvent& event) = 0;

   virtual void OnConnected(const CpCallStateEvent& event) = 0;

   virtual void OnBridged(const CpCallStateEvent& event) = 0;

   virtual void OnHeld(const CpCallStateEvent& event) = 0;

   virtual void OnRemoteHeld(const CpCallStateEvent& event) = 0;

   virtual void OnDisconnected(const CpCallStateEvent& event) = 0;

   virtual void OnOffering(const CpCallStateEvent& event) = 0;

   virtual void OnAlerting(const CpCallStateEvent& event) = 0;

   virtual void OnDestroyed(const CpCallStateEvent& event) = 0;

   virtual void OnTransferEvent(const CpCallStateEvent& event) = 0;

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // CpCallStateEventListener_h__
