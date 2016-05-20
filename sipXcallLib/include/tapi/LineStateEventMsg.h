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

#ifndef LineStateEventMsg_h__
#define LineStateEventMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <net/SipLineStateEventListener.h>
#include <tapi/sipXtapiEvents.h>

// DEFINES
#define LINESTATEEVENT_MSG OsMsg::USER_START + 1

// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * Wrapper class for SipLineStateEvent
 */
class LineStateEventMsg : public OsMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   LineStateEventMsg();

   LineStateEventMsg(SIPX_LINESTATE_EVENT event,
      const SipLineStateEvent& eventPayload);

   virtual ~LineStateEventMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   SIPX_LINESTATE_EVENT getEvent() const { return m_event; }
   void setEvent(SIPX_LINESTATE_EVENT val) { m_event = val; }

   SipLineStateEvent getEventPayload() const { return m_eventPayload; }
   const SipLineStateEvent& getEventPayloadRef() const { return m_eventPayload; }
   void setEventPayload(const SipLineStateEvent& val) { m_eventPayload = val; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   LineStateEventMsg(const LineStateEventMsg& rhs);

   LineStateEventMsg& operator=(const LineStateEventMsg& rhs);

   SIPX_LINESTATE_EVENT m_event;
   SipLineStateEvent m_eventPayload;
};

#endif // LineStateEventMsg_h__
