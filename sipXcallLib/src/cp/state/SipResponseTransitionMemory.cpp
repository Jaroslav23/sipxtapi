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
#include <cp/state/SipResponseTransitionMemory.h>

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

SipResponseTransitionMemory::SipResponseTransitionMemory(int sipResponseCode, const UtlString& sipResponseText)
: m_sipResponseCode(sipResponseCode)
, m_sipResponseText(sipResponseText)
{

}

SipResponseTransitionMemory::~SipResponseTransitionMemory()
{

}

/* ============================ MANIPULATORS ============================== */

StateTransitionMemory* SipResponseTransitionMemory::clone() const
{
   return new SipResponseTransitionMemory(m_sipResponseCode, m_sipResponseText);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
