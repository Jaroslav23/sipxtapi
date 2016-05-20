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

#ifndef InfoStatusEventMsg_h__
#define InfoStatusEventMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <net/SipInfoStatusEventListener.h>
#include <tapi/sipXtapiEvents.h>

// DEFINES
#define INFOSTATUS_MSG OsMsg::USER_START + 1

// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * Wrapper class for SipInfoStatusEvent
 */
class InfoStatusEventMsg : public OsMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   InfoStatusEventMsg();

   InfoStatusEventMsg(SIPX_INFOSTATUS_EVENT event,
                      const SipInfoStatusEvent& eventPayload);

   virtual ~InfoStatusEventMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   SIPX_INFOSTATUS_EVENT getEvent() const { return m_event; }
   void setEvent(SIPX_INFOSTATUS_EVENT val) { m_event = val; }

   SipInfoStatusEvent getEventPayload() const { return m_eventPayload; }
   const SipInfoStatusEvent& getEventPayloadRef() const { return m_eventPayload; }
   void setEventPayload(const SipInfoStatusEvent& val) { m_eventPayload = val; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   InfoStatusEventMsg(const InfoStatusEventMsg& rhs);

   InfoStatusEventMsg& operator=(const InfoStatusEventMsg& rhs);

   SIPX_INFOSTATUS_EVENT m_event;
   SipInfoStatusEvent m_eventPayload;
};

#endif // InfoStatusEventMsg_h__
