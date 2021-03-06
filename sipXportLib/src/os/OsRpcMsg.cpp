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
#include "os/OsRpcMsg.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsRpcMsg::OsRpcMsg(const unsigned char msgType,
                   const unsigned char msgSubType,
                   const OsEvent& rEvent)
:  
   OsMsg(msgType, msgSubType),
   mpEvent((OsEvent*) &rEvent)
{
   // all of the necessary work is done by the initializers
}

// Copy constructor
OsRpcMsg::OsRpcMsg(const OsRpcMsg& rOsRpcMsg)
:  OsMsg(rOsRpcMsg)
{
   mpEvent = rOsRpcMsg.mpEvent;
}

// Create a copy of this msg object (which may be of a derived type)
OsMsg* OsRpcMsg::createCopy(void) const
{
   return new OsRpcMsg(*this);
}

// Destructor
OsRpcMsg::~OsRpcMsg()
{
   // no work required
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsRpcMsg& 
OsRpcMsg::operator=(const OsRpcMsg& rhs)
{
   if (this != &rhs)            // handle the assignment to self case
   {
      OsMsg::operator=(rhs);
      mpEvent = rhs.mpEvent;
   }
   
   return *this;
}

/* ============================ ACCESSORS ================================= */

// Return the size of the message in bytes.
// This is a virtual method so that it will return the accurate size for
// the message object even if that object has been upcast to the type of
// an ancestor class.
int OsRpcMsg::getMsgSize(void) const
{
   return sizeof(*this);
}

// Return the pointer to the OsEvent object used to signal completion
OsEvent* OsRpcMsg::getEvent(void) const
{
   return mpEvent;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


