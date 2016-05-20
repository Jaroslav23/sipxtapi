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
#include <cp/state/SipConnectionStateTransition.h>
#include <cp/state/StateTransitionMemory.h>

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

SipConnectionStateTransition::SipConnectionStateTransition(const BaseSipConnectionState* pSource,
                                                           BaseSipConnectionState* pDestination,
                                                           StateTransitionMemory* pMemory)
: m_pSource(pSource)
, m_pDestination(pDestination)
, m_pMemory(pMemory)
{

}

SipConnectionStateTransition::~SipConnectionStateTransition()
{
   if (m_pMemory)
   {
      delete m_pMemory;
      m_pMemory = NULL;
   }
   m_pSource = NULL;
   m_pDestination = NULL;
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
