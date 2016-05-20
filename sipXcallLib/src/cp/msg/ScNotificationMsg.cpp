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
#include <cp/msg/ScNotificationMsg.h>

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

ScNotificationMsg::ScNotificationMsg(SubTypesEnum subType, const SipDialog& sipDialog)
: OsMsg(CpMessageTypes::SC_NOFITICATION, (unsigned char)subType)
, m_sipDialog(sipDialog)
{

}

ScNotificationMsg::ScNotificationMsg(const ScNotificationMsg& rMsg)
: OsMsg(rMsg)
, m_sipDialog(rMsg.m_sipDialog)
{

}

ScNotificationMsg::~ScNotificationMsg()
{

}

OsMsg* ScNotificationMsg::createCopy(void) const
{
   return new ScNotificationMsg(*this);
}

/* ============================ MANIPULATORS ============================== */

ScNotificationMsg& ScNotificationMsg::operator=(const ScNotificationMsg& rhs)
{
   if (this == &rhs)
   {
      return *this;
   }

   OsMsg::operator=(rhs); // assign fields for parent class

   m_sipDialog = rhs.m_sipDialog;

   return *this;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

