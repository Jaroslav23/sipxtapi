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

#ifndef SecurityEventMsg_h__
#define SecurityEventMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <net/SipSecurityEventListener.h>
#include <tapi/sipXtapiEvents.h>

// DEFINES
#define SECURITY_MSG OsMsg::USER_START + 1

// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * Wrapper class for SipSecurityEvent
 */
class SecurityEventMsg : public OsMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   SecurityEventMsg();

   SecurityEventMsg(SIPX_SECURITY_EVENT event,
      const SipSecurityEvent& eventPayload);

   virtual ~SecurityEventMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   SIPX_SECURITY_EVENT getEvent() const { return m_event; }
   void setEvent(SIPX_SECURITY_EVENT val) { m_event = val; }

   SipSecurityEvent getEventPayload() const { return m_eventPayload; }
   const SipSecurityEvent& getEventPayloadRef() const { return m_eventPayload; }
   void setEventPayload(const SipSecurityEvent& val) { m_eventPayload = val; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   SecurityEventMsg(const SecurityEventMsg& rhs);

   SecurityEventMsg& operator=(const SecurityEventMsg& rhs);

   SIPX_SECURITY_EVENT m_event;
   SipSecurityEvent m_eventPayload;
};

#endif // SecurityEventMsg_h__
