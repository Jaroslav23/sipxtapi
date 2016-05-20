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
#include <cp/msg/AcRenegotiateCodecsAllMsg.h>

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

AcRenegotiateCodecsAllMsg::AcRenegotiateCodecsAllMsg(const UtlString& sAudioCodecs,
                                                     const UtlString& sVideoCodecs)
: AcCommandMsg(AC_RENEGOTIATE_CODECS_ALL)
, m_sAudioCodecs(sAudioCodecs)
, m_sVideoCodecs(sVideoCodecs)
{

}

AcRenegotiateCodecsAllMsg::~AcRenegotiateCodecsAllMsg()
{

}

OsMsg* AcRenegotiateCodecsAllMsg::createCopy(void) const
{
   return new AcRenegotiateCodecsAllMsg(m_sAudioCodecs, m_sVideoCodecs);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

