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

#ifndef MediaEventMsg_h__
#define MediaEventMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <tapi/SipXMediaEventListener.h>
#include <tapi/sipXtapiEvents.h>

// DEFINES
#define MEDIAEVENT_MSG OsMsg::USER_START + 1

// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
* Wrapper class for SipXMediaEvent
*/
class MediaEventMsg : public OsMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   MediaEventMsg();

   MediaEventMsg(const SipXMediaEvent& eventPayload);

   virtual ~MediaEventMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   SipXMediaEvent getEventPayload() const { return m_eventPayload; }
   const SipXMediaEvent& getEventPayloadRef() const { return m_eventPayload; }
   void setEventPayload(const SipXMediaEvent& val) { m_eventPayload = val; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   MediaEventMsg(const MediaEventMsg& rhs);

   MediaEventMsg& operator=(const MediaEventMsg& rhs);

   SipXMediaEvent m_eventPayload;
};

#endif // MediaEventMsg_h__
