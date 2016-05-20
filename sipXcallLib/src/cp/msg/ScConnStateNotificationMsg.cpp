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
#include <cp/msg/ScConnStateNotificationMsg.h>

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

ScConnStateNotificationMsg::ScConnStateNotificationMsg(ISipConnectionState::StateEnum state,
                                                       const SipDialog& targetSipDialog,
                                                       const SipDialog& sourceSipDialog)
: ScNotificationMsg(ScNotificationMsg::SCN_CONNECTION_STATE, targetSipDialog)
, m_state(state)
, m_sourceSipDialog(sourceSipDialog)
{

}

ScConnStateNotificationMsg::ScConnStateNotificationMsg(const ScConnStateNotificationMsg& rMsg)
: ScNotificationMsg(rMsg)
, m_state(rMsg.m_state)
, m_sourceSipDialog(rMsg.m_sourceSipDialog)
{

}

ScConnStateNotificationMsg::~ScConnStateNotificationMsg()
{

}

OsMsg* ScConnStateNotificationMsg::createCopy(void) const
{
   return new ScConnStateNotificationMsg(*this);
}

/* ============================ MANIPULATORS ============================== */

ScConnStateNotificationMsg& ScConnStateNotificationMsg::operator=(const ScConnStateNotificationMsg& rhs)
{
   if (this == &rhs)
   {
      return *this;
   }

   ScNotificationMsg::operator=(rhs); // assign fields for parent class

   m_state = rhs.m_state;
   m_sourceSipDialog = rhs.m_sourceSipDialog;

   return *this;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

