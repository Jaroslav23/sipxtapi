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

#ifndef InfoEventMsg_h__
#define InfoEventMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <net/SipInfoEventListener.h>

// DEFINES
#define INFO_MSG OsMsg::USER_START + 1

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
class InfoEventMsg : public OsMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   InfoEventMsg();

   InfoEventMsg(const SipInfoEvent& eventPayload);

   virtual ~InfoEventMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   SipInfoEvent getEventPayload() const { return m_eventPayload; }
   const SipInfoEvent& getEventPayloadRef() const { return m_eventPayload; }
   void setEventPayload(const SipInfoEvent& val) { m_eventPayload = val; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   InfoEventMsg(const InfoEventMsg& rhs);

   InfoEventMsg& operator=(const InfoEventMsg& rhs);

   SipInfoEvent m_eventPayload;
};

#endif // InfoEventMsg_h__
