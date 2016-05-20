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

#ifndef RtpRedirectEventMsg_h__
#define RtpRedirectEventMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <cp/CpRtpRedirectEventListener.h>
#include <tapi/sipXtapiEvents.h>

// DEFINES
#define RTPREDIRECTEVENT_MSG OsMsg::USER_START + 1

// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
* Wrapper class for CpRtpRedirectEvent
*/
class RtpRedirectEventMsg : public OsMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   RtpRedirectEventMsg();

   RtpRedirectEventMsg(SIPX_RTP_REDIRECT_EVENT event,
                       const CpRtpRedirectEvent& eventPayload);

   virtual ~RtpRedirectEventMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   SIPX_RTP_REDIRECT_EVENT getEvent() const { return m_event; }
   void setEvent(SIPX_RTP_REDIRECT_EVENT val) { m_event = val; }

   CpRtpRedirectEvent getEventPayload() const { return m_eventPayload; }
   const CpRtpRedirectEvent& getEventPayloadRef() const { return m_eventPayload; }
   void setEventPayload(const CpRtpRedirectEvent& val) { m_eventPayload = val; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   RtpRedirectEventMsg(const RtpRedirectEventMsg& rhs);

   RtpRedirectEventMsg& operator=(const RtpRedirectEventMsg& rhs);

   SIPX_RTP_REDIRECT_EVENT m_event;
   CpRtpRedirectEvent m_eventPayload;
};

#endif // RtpRedirectEventMsg_h__
