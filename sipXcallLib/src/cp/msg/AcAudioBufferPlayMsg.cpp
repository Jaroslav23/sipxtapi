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
#include <cp/msg/AcAudioBufferPlayMsg.h>

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

AcAudioBufferPlayMsg::AcAudioBufferPlayMsg(const void* pAudiobuf,
                                           size_t iBufSize,
                                           int iType,
                                           UtlBoolean bRepeat,
                                           UtlBoolean bLocal,
                                           UtlBoolean bRemote,
                                           UtlBoolean bMixWithMic,
                                           int iDownScaling,
                                           void* pCookie)
: AcCommandMsg(AC_AUDIO_BUFFER_PLAY)
, m_pAudiobuf(NULL)
, m_iBufSize(iBufSize)
, m_iType(iType)
, m_bRepeat(bRepeat)
, m_bLocal(bLocal)
, m_bRemote(bRemote)
, m_bMixWithMic(bMixWithMic)
, m_iDownScaling(iDownScaling)
, m_pCookie(pCookie)
{
   if (m_iBufSize > 0 && pAudiobuf)
   {
      m_pAudiobuf = malloc(sizeof(char) * m_iBufSize);
      if (m_pAudiobuf)
      {
         memcpy(m_pAudiobuf, pAudiobuf, m_iBufSize);
      }
   }
}

AcAudioBufferPlayMsg::~AcAudioBufferPlayMsg()
{
   if (m_pAudiobuf)
   {
      free(m_pAudiobuf);
      m_pAudiobuf = NULL;
   }
}

OsMsg* AcAudioBufferPlayMsg::createCopy(void) const
{
   return new AcAudioBufferPlayMsg(m_pAudiobuf, m_iBufSize, m_iType, m_bRepeat, m_bLocal, m_bRemote,
      m_bMixWithMic, m_iDownScaling, m_pCookie);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

