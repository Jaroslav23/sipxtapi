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
#include <cp/msg/AcNotificationMsg.h>

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

AcNotificationMsg::AcNotificationMsg(SubTypesEnum subType, const SipDialog& sipDialog)
: OsMsg(CpMessageTypes::AC_NOTIFICATION, (unsigned char)subType)
, m_sipDialog(sipDialog)
{

}

AcNotificationMsg::AcNotificationMsg(const AcNotificationMsg& rhs)
: OsMsg(rhs)
, m_sipDialog(rhs.m_sipDialog)
{

}

AcNotificationMsg::~AcNotificationMsg()
{

}

OsMsg* AcNotificationMsg::createCopy(void) const
{
   return new AcNotificationMsg(*this);
}

/* ============================ MANIPULATORS ============================== */

AcNotificationMsg& AcNotificationMsg::operator=(const AcNotificationMsg& rhs)
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

