//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <net/SipMessageEvent.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipMessageEvent::SipMessageEvent(SipMessage* message, int status /*= APPLICATION*/)
: OsMsg(OsMsg::PHONE_APP, SipMessage::NET_SIP_MESSAGE)
, m_messageStatus(status)
, m_pSipMessage(message) // use supplied message
{
}

SipMessageEvent::SipMessageEvent(const SipMessage& rSipMessage, int status /*= APPLICATION*/)
: OsMsg(OsMsg::PHONE_APP, SipMessage::NET_SIP_MESSAGE)
, m_messageStatus(status)
, m_pSipMessage(new SipMessage(rSipMessage)) // use a copy
{
}

// Destructor
SipMessageEvent::~SipMessageEvent()
{
   if(m_pSipMessage)
   {
      delete m_pSipMessage;
      m_pSipMessage = NULL;
   }
}

OsMsg* SipMessageEvent::createCopy() const
{
   // Inefficient but easy coding way to copy message
   SipMessage* sipMsg = NULL;

   if(m_pSipMessage)
   {
      sipMsg = new SipMessage(*m_pSipMessage);
   }

   return new SipMessageEvent(sipMsg, m_messageStatus);
}
/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipMessageEvent& SipMessageEvent::operator=(const SipMessageEvent& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);

   m_messageStatus = rhs.m_messageStatus;
   
   if(m_pSipMessage)
   {
      delete m_pSipMessage;
      m_pSipMessage = NULL;
   }

   if(rhs.m_pSipMessage)
   {
      m_pSipMessage = new SipMessage(*(rhs.m_pSipMessage));
   }

   return *this;
}

/* ============================ ACCESSORS ================================= */

const SipMessage* SipMessageEvent::getMessage() const
{
   return(m_pSipMessage);
}

void SipMessageEvent::setMessageStatus(int status)
{
   m_messageStatus = status;
}

int SipMessageEvent::getMessageStatus() const
{
   return(m_messageStatus);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
