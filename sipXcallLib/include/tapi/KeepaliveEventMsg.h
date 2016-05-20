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

#ifndef KeepaliveEventMsg_h__
#define KeepaliveEventMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <os/OsNatKeepaliveListener.h>
#include <tapi/sipXtapiEvents.h>

// DEFINES
#define KEEPALIVE_MSG OsMsg::USER_START + 1

// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * Wrapper class for OsNatKeepaliveEvent
 */
class KeepaliveEventMsg : public OsMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   KeepaliveEventMsg();

   KeepaliveEventMsg(SIPX_KEEPALIVE_EVENT event,
                     SIPX_KEEPALIVE_CAUSE eventCause,
                     const OsNatKeepaliveEvent& eventPayload);

   virtual ~KeepaliveEventMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   SIPX_KEEPALIVE_EVENT getEvent() const { return m_event; }
   void setEvent(SIPX_KEEPALIVE_EVENT val) { m_event = val; }

   SIPX_KEEPALIVE_CAUSE getEventCause() const { return m_eventCause; }
   void setEventCause(SIPX_KEEPALIVE_CAUSE val) { m_eventCause = val; }

   OsNatKeepaliveEvent getEventPayload() const { return m_eventPayload; }
   const OsNatKeepaliveEvent& getEventPayloadRef() const { return m_eventPayload; }
   void setEventPayload(const OsNatKeepaliveEvent& val) { m_eventPayload = val; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   KeepaliveEventMsg(const KeepaliveEventMsg& rhs);

   KeepaliveEventMsg& operator=(const KeepaliveEventMsg& rhs);

   SIPX_KEEPALIVE_EVENT m_event;
   SIPX_KEEPALIVE_CAUSE m_eventCause;
   OsNatKeepaliveEvent m_eventPayload;
};

#endif // KeepaliveEventMsg_h__
