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
#include <cp/state/GeneralTransitionMemory.h>

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

GeneralTransitionMemory::GeneralTransitionMemory(CP_CALLSTATE_CAUSE cause,
                                                 int sipResponseCode,
                                                 const UtlString& sipResponseText,
                                                 const UtlString& originalSessionCallId)
: m_cause(cause)
, m_sipResponseCode(sipResponseCode)
, m_sipResponseText(sipResponseText)
, m_originalSessionCallId(originalSessionCallId)
{

}

GeneralTransitionMemory::~GeneralTransitionMemory()
{

}

/* ============================ MANIPULATORS ============================== */

StateTransitionMemory* GeneralTransitionMemory::clone() const
{
   return new GeneralTransitionMemory(m_cause, m_sipResponseCode, m_sipResponseText, m_originalSessionCallId);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
