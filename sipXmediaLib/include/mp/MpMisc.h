//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _INCLUDED_MPMISC_H /* [ */
#define _INCLUDED_MPMISC_H

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsMsgQ.h"
#include "mp/MpTypes.h"
#include "mp/MpBufPool.h"
#include "mp/MpAudioBuf.h"
#include "mp/MpAudioDriverDefs.h"

// DEFINES

/* miscellaneous debugging support: */
#define Zprintf printf
#undef Zprintf
#define Zprintf(fmt, a, b, c, d, e, f)
#define Nprintf(fmt, a, b, c, d, e, f)
#define Lprintf(fmt, a, b, c, d, e, f)

/**
 * mpStartUp initializes the MpMisc struct and other MP data, for
 * example, the buffer pools and tables. It also initializes MpAudioDriverManager,
 * MpMediaTask, NetInTask. Sampling rate is fixed, and cannot be changed.
 */
OsStatus mpStartUp();

/**
 * tears down whatever was created in mpStartUp
 */
OsStatus mpShutdown();

/// This structure contain all static variables
struct MpGlobals
{
   OsMsgQ* m_pEchoQ;         ///< Message queue for echo cancelation data
                           ///<  (it is copy of speaker data).
   int m_audioSamplesPerFrame;       ///< Number of samples in one audio frame
   int m_audioSamplesPerSec;        ///< Sample rate per sec

   MpBufPool *m_pRawAudioPool;     ///< Memory pool for raw audio data buffers
   MpBufPool *m_pAudioHeadersPool;
   MpBufPool *m_pRtpPool;          ///< Memory pool for RTP data buffers
   MpBufPool *m_pRtcpPool;         ///< Memory pool for RTCP data buffers
   MpBufPool *m_pRtpHeadersPool;   ///< Memory pool for headers of RTP and
   MpBufPool *m_pUdpPool;          ///< Memory pool for raw UDP packets
   MpBufPool *m_pUdpHeadersPool;   ///< Memory pool for headers of UDP packets
                                   ///<  buffers
   MpAudioBufPtr m_fgSilence;   ///< Buffer filled with silence. Used for
                                ///<  muting and as default output. You
                                ///<  should not modify this buffer, cause
                                ///<  it is used many times.
};

extern struct MpGlobals MpMisc;

#endif /* _INCLUDED_MPMISC_H ] */
