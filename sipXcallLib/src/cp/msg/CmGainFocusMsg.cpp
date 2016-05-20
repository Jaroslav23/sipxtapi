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
#include <cp/msg/CmGainFocusMsg.h>

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

CmGainFocusMsg::CmGainFocusMsg(const UtlString& sAbstractCallId, UtlBoolean bGainOnlyIfNoFocusedCall)
: CmCommandMsg(CM_GAIN_FOCUS)
, m_sAbstractCallId(sAbstractCallId)
, m_bGainOnlyIfNoFocusedCall(bGainOnlyIfNoFocusedCall)
{

}

CmGainFocusMsg::~CmGainFocusMsg()
{

}

OsMsg* CmGainFocusMsg::createCopy(void) const
{
   return new CmGainFocusMsg(m_sAbstractCallId, m_bGainOnlyIfNoFocusedCall);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

