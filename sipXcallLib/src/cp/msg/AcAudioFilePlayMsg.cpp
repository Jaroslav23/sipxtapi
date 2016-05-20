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
#include <cp/msg/AcAudioFilePlayMsg.h>

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

AcAudioFilePlayMsg::AcAudioFilePlayMsg(const UtlString& sAudioFile,
                                       UtlBoolean bRepeat,
                                       UtlBoolean bLocal,
                                       UtlBoolean bRemote,
                                       UtlBoolean bMixWithMic,
                                       int iDownScaling,
                                       void* pCookie)
: AcCommandMsg(AC_AUDIO_FILE_PLAY)
, m_sAudioFile(sAudioFile)
, m_bRepeat(bRepeat)
, m_bLocal(bLocal)
, m_bRemote(bRemote)
, m_bMixWithMic(bMixWithMic)
, m_iDownScaling(iDownScaling)
, m_pCookie(pCookie)
{

}

AcAudioFilePlayMsg::~AcAudioFilePlayMsg()
{

}

OsMsg* AcAudioFilePlayMsg::createCopy(void) const
{
   return new AcAudioFilePlayMsg(m_sAudioFile, m_bRepeat, m_bLocal, m_bRemote,
      m_bMixWithMic, m_iDownScaling, m_pCookie);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

