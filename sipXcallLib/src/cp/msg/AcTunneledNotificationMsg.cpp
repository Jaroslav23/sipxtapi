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

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <cp/msg/AcTunneledNotificationMsg.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

AcTunneledNotificationMsg::AcTunneledNotificationMsg(const OsMsg& msg, const SipDialog& sipDialog)
: AcNotificationMsg(AcNotificationMsg::ACN_TUNNELED,  sipDialog)
, m_pMsg(NULL)
{
   m_pMsg = msg.createCopy();
}

AcTunneledNotificationMsg::AcTunneledNotificationMsg(const AcTunneledNotificationMsg& rhs)
: AcNotificationMsg(rhs)
, m_pMsg(NULL)
{
   if (rhs.m_pMsg)
   {
      m_pMsg = rhs.m_pMsg->createCopy();
   }
}

AcTunneledNotificationMsg::~AcTunneledNotificationMsg()
{
   delete m_pMsg;
   m_pMsg = NULL;
}

OsMsg* AcTunneledNotificationMsg::createCopy(void) const
{
   return new AcTunneledNotificationMsg(*this);
}

/* ============================ MANIPULATORS ============================== */

AcTunneledNotificationMsg& AcTunneledNotificationMsg::operator=(const AcTunneledNotificationMsg& rhs)
{
   if (this == &rhs)
   {
      return *this;
   }

   AcNotificationMsg::operator=(rhs); // assign fields for parent class

   if (m_pMsg)
   {
      delete m_pMsg;
      m_pMsg = NULL;
   }

   if (rhs.m_pMsg)
   {
      m_pMsg = rhs.m_pMsg->createCopy();
   }

   return *this;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

