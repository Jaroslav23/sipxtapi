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

#ifndef CallStateEventMsg_h__
#define CallStateEventMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <cp/CpCallStateEventListener.h>
#include <tapi/sipXtapiEvents.h>

// DEFINES
#define CALLSTATEEVENT_MSG OsMsg::USER_START + 1

// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
* Wrapper class for CpCallStateEvent
*/
class CallStateEventMsg : public OsMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   CallStateEventMsg();

   CallStateEventMsg(SIPX_CALLSTATE_EVENT event,
                     const CpCallStateEvent& eventPayload);

   virtual ~CallStateEventMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   SIPX_CALLSTATE_EVENT getEvent() const { return m_event; }
   void setEvent(SIPX_CALLSTATE_EVENT val) { m_event = val; }

   CpCallStateEvent getEventPayload() const { return m_eventPayload; }
   const CpCallStateEvent& getEventPayloadRef() const { return m_eventPayload; }
   void setEventPayload(const CpCallStateEvent& val) { m_eventPayload = val; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   CallStateEventMsg(const CallStateEventMsg& rhs);

   CallStateEventMsg& operator=(const CallStateEventMsg& rhs);

   SIPX_CALLSTATE_EVENT m_event;
   CpCallStateEvent m_eventPayload;
};

#endif // CallStateEventMsg_h__
