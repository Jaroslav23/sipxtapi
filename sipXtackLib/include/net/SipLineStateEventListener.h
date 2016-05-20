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

#ifndef SipLineStateEventListener_h__
#define SipLineStateEventListener_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
// STRUCTS
// TYPEDEFS

typedef enum SIPXTACK_LINESTATE_EVENT
{
    SIPXTACK_LINESTATE_UNKNOWN         = 0,
    SIPXTACK_LINESTATE_REGISTERING     = 20000,
    SIPXTACK_LINESTATE_REGISTERED      = 21000,
    SIPXTACK_LINESTATE_UNREGISTERING   = 22000,
    SIPXTACK_LINESTATE_UNREGISTERED    = 23000,
    SIPXTACK_LINESTATE_REGISTER_FAILED = 24000,
    SIPXTACK_LINESTATE_UNREGISTER_FAILED  = 25000,
    SIPXTACK_LINESTATE_PROVISIONED      = 26000,
} SIPXTACK_LINESTATE_EVENT;  

typedef enum SIPXTACK_LINESTATE_CAUSE
{
   SIPXTACK_LINESTATE_CAUSE_UNKNOWN                           = 0,
   SIPXTACK_LINESTATE_REGISTERING_NORMAL                      = SIPXTACK_LINESTATE_REGISTERING + 1,
   SIPXTACK_LINESTATE_REGISTERED_NORMAL                       = SIPXTACK_LINESTATE_REGISTERED + 1,
   SIPXTACK_LINESTATE_UNREGISTERING_NORMAL                    = SIPXTACK_LINESTATE_UNREGISTERING + 1,
   SIPXTACK_LINESTATE_UNREGISTERED_NORMAL                     = SIPXTACK_LINESTATE_UNREGISTERED + 1,
   SIPXTACK_LINESTATE_REGISTER_FAILED_COULD_NOT_CONNECT       = SIPXTACK_LINESTATE_REGISTER_FAILED + 1,
   SIPXTACK_LINESTATE_REGISTER_FAILED_NOT_AUTHORIZED          = SIPXTACK_LINESTATE_REGISTER_FAILED + 2,
   SIPXTACK_LINESTATE_REGISTER_FAILED_TIMEOUT                 = SIPXTACK_LINESTATE_REGISTER_FAILED + 3,
   SIPXTACK_LINESTATE_UNREGISTER_FAILED_COULD_NOT_CONNECT     = SIPXTACK_LINESTATE_UNREGISTER_FAILED + 1,
   SIPXTACK_LINESTATE_UNREGISTER_FAILED_NOT_AUTHORIZED        = SIPXTACK_LINESTATE_UNREGISTER_FAILED + 2,
   SIPXTACK_LINESTATE_UNREGISTER_FAILED_TIMEOUT               = SIPXTACK_LINESTATE_UNREGISTER_FAILED + 3,
   SIPXTACK_LINESTATE_PROVISIONED_NORMAL                      = SIPXTACK_LINESTATE_PROVISIONED + 1
} SIPXTACK_LINESTATE_CAUSE;

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

class SipLineStateEvent
{
public:

   UtlString m_sLineId;
   SIPXTACK_LINESTATE_CAUSE m_cause;
   int m_responseCode;
   UtlString m_sResponseText;

   SipLineStateEvent() : m_sLineId()
      , m_cause(SIPXTACK_LINESTATE_CAUSE_UNKNOWN)
      , m_responseCode(0)
      , m_sResponseText()
   {

   }

   SipLineStateEvent(const UtlString& lineId, SIPXTACK_LINESTATE_CAUSE cause, int responseCode = 0, const UtlString& sResponseText = NULL)
      : m_sLineId(lineId),
        m_cause(cause),
        m_responseCode(responseCode),
        m_sResponseText(sResponseText)
   {

   }

   ~SipLineStateEvent()
   {

   }

   SipLineStateEvent(const SipLineStateEvent& event)
   {
      *this = event;
   }

   SipLineStateEvent& operator=(const SipLineStateEvent& event)
   {
      if (&event == this)
      {
         return *this;
      }

      m_sLineId = event.m_sLineId;
      m_cause = event.m_cause;
      m_responseCode = event.m_responseCode;
      m_sResponseText = event.m_sResponseText;

      return *this;
   }
};


/**
* Listener for Line state events
*/
class SipLineStateEventListener
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */

   /* ============================ CREATORS ================================== */
public:
   SipLineStateEventListener() {}
   virtual ~SipLineStateEventListener() {}

   /* ============================ MANIPULATORS ============================== */

   virtual void OnLineRegistering(const SipLineStateEvent& event) = 0;

   virtual void OnLineRegistered(const SipLineStateEvent& event) = 0;

   virtual void OnLineUnregistering(const SipLineStateEvent& event) = 0;

   virtual void OnLineUnregistered(const SipLineStateEvent& event) = 0;

   virtual void OnLineRegisterFailed(const SipLineStateEvent& event) = 0;

   virtual void OnLineUnregisterFailed(const SipLineStateEvent& event) = 0;

   virtual void OnLineProvisioned(const SipLineStateEvent& event) = 0;

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // SipLineStateEventListener_h__
