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
#include <os/OsQueuedNotification.h>

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

OsQueuedNotification::OsQueuedNotification(OsMsgQ& rMsgQ, const OsMsg& rOsMsg)
: OsNotification()
, m_rMsgQ(rMsgQ)
, m_pOsMsg(rOsMsg.createCopy())
{

}

OsQueuedNotification::~OsQueuedNotification()
{
   if (m_pOsMsg)
   {
      delete m_pOsMsg;
      m_pOsMsg = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

OsStatus OsQueuedNotification::signal(const intptr_t eventData /*= 0*/)
{
   if (m_pOsMsg)
   {
      return m_rMsgQ.send(*m_pOsMsg);
   }
   else
   {
      return OS_INVALID_STATE;
   }
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

