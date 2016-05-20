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

#ifndef ConferenceEventMsg_h__
#define ConferenceEventMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <cp/CpConferenceEventListener.h>
#include <tapi/sipXtapiEvents.h>

// DEFINES
#define CONFERENCEEVENT_MSG OsMsg::USER_START + 1

// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
* Wrapper class for CpConferenceEvent
*/
class ConferenceEventMsg : public OsMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   ConferenceEventMsg();

   ConferenceEventMsg(SIPX_CONFERENCE_EVENT event,
                      const CpConferenceEvent& eventPayload);

   virtual ~ConferenceEventMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   SIPX_CONFERENCE_EVENT getEvent() const { return m_event; }
   void setEvent(SIPX_CONFERENCE_EVENT val) { m_event = val; }

   CpConferenceEvent getEventPayload() const { return m_eventPayload; }
   const CpConferenceEvent& getEventPayloadRef() const { return m_eventPayload; }
   void setEventPayload(const CpConferenceEvent& val) { m_eventPayload = val; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   ConferenceEventMsg(const ConferenceEventMsg& rhs);

   ConferenceEventMsg& operator=(const ConferenceEventMsg& rhs);

   SIPX_CONFERENCE_EVENT m_event;
   CpConferenceEvent m_eventPayload;
};

#endif // ConferenceEventMsg_h__
