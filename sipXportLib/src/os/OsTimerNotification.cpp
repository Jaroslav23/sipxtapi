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
#include <os/OsTimerNotification.h>
#include <os/OsDateTime.h>
#include <os/OsTime.h>
#include <os/OsTimerMsg.h>

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

OsTimerNotification::OsTimerNotification(OsMsgQ& rMsgQ, const OsTimerMsg& rMsg)
: OsQueuedNotification(rMsgQ, rMsg)
{

}

OsTimerNotification::~OsTimerNotification()
{
}

/* ============================ MANIPULATORS ============================== */

OsStatus OsTimerNotification::signal(const intptr_t eventData /*= 0*/)
{
   if (m_pOsMsg)
   {
      // add timestamp to the message
      OsTimerMsg* pOsTimerMsg = (OsTimerMsg*)m_pOsMsg;
      OsTime timestamp;
      OsDateTime::getCurTimeSinceBoot(timestamp);
      pOsTimerMsg->setTimestamp(timestamp);
      // send message
      return m_rMsgQ.send(*m_pOsMsg); // makes a copy of message including timestamp
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

