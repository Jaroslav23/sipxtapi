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
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsTimerTaskCommandMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsTimerTaskCommandMsg::OsTimerTaskCommandMsg(const unsigned char subType,
                       OsTimer* pTimer,
                       OsEvent* pEvent)
: OsRpcMsg(OsMsg::OS_TIMERTASK_COMMAND, subType, *pEvent),
  mpTimer(pTimer)
{
   init();
}

// Copy constructor
OsTimerTaskCommandMsg::OsTimerTaskCommandMsg(const OsTimerTaskCommandMsg& rOsTimerMsg)
: OsRpcMsg(rOsTimerMsg)
{
   mpTimer = rOsTimerMsg.mpTimer;
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* OsTimerTaskCommandMsg::createCopy(void) const
{
   return new OsTimerTaskCommandMsg(*this);
}

// Destructor
OsTimerTaskCommandMsg::~OsTimerTaskCommandMsg()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsTimerTaskCommandMsg& 
OsTimerTaskCommandMsg::operator=(const OsTimerTaskCommandMsg& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   OsMsg::operator=(rhs);       // assign fields for parent class

   mpTimer = rhs.mpTimer;

   return *this;
}

/* ============================ ACCESSORS ================================= */

// Return the size of the message in bytes.
// This is a virtual method so that it will return the accurate size for
// the message object even if that object has been upcast to the type of
// an ancestor class.
int OsTimerTaskCommandMsg::getMsgSize(void) const
{
   return sizeof(*this);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Initialization common to all constructors
void OsTimerTaskCommandMsg::init(void)
{
}

/* ============================ FUNCTIONS ================================= */
