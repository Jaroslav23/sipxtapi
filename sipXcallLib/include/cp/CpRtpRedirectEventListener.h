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

#ifndef CpRtpRedirectEventListener_h__
#define CpRtpRedirectEventListener_h__

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

class CpRtpRedirectEvent
{
public:
   UtlString m_sCallId; // id of abstract call
   CP_RTP_REDIRECT_CAUSE m_cause;

   CpRtpRedirectEvent(const UtlString& sCallId,
                      CP_RTP_REDIRECT_CAUSE cause)
      : m_sCallId(sCallId)
      , m_cause(cause)
   {
   }

   CpRtpRedirectEvent()
   {
      m_cause = CP_RTP_REDIRECT_CAUSE_NORMAL;
   }

   ~CpRtpRedirectEvent()
   {
   }

   CpRtpRedirectEvent(const CpRtpRedirectEvent& event)
   {
      *this = event;
   }

   CpRtpRedirectEvent& operator=(const CpRtpRedirectEvent& event)
   {
      if (&event == this)
      {
         return *this;
      }

      m_sCallId = event.m_sCallId;
      m_cause = event.m_cause;
      return *this;
   }
};


/**
* Listener for RTP redirect events
*/
class CpRtpRedirectEventListener
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */

   /* ============================ CREATORS ================================== */
public:
   CpRtpRedirectEventListener() {}
   virtual ~CpRtpRedirectEventListener() {}

   /* ============================ MANIPULATORS ============================== */

   virtual void OnRtpRedirectRequested(const CpRtpRedirectEvent& event) = 0;

   virtual void OnRtpRedirectActive(const CpRtpRedirectEvent& event) = 0;

   virtual void OnRtpRedirectError(const CpRtpRedirectEvent& event) = 0;

   virtual void OnRtpRedirectStop(const CpRtpRedirectEvent& event) = 0;

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // CpRtpRedirectEventListener_h__
