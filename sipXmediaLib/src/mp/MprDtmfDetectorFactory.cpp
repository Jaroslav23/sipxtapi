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
#include <mp/MprDtmfDetectorFactory.h>
#include <mp/MprSimpleDtmfDetector.h>
#ifdef HAVE_SPAN_DSP /* [ */
#include <mp/MprSpanDspDtmfDetector.h>
#endif /* HAVE_SPAN_DSP ] */

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

/* ============================ MANIPULATORS ============================== */

MprDtmfDetectorBase* MprDtmfDetectorFactory::createDtmfDetector(const UtlString& rName,
                                                                int samplesPerFrame,
                                                                int samplesPerSec)
{
#if defined(HAVE_SPAN_DSP) && defined(USE_SPAN_DSP_DTMF) && !defined(ENABLE_WIDEBAND_AUDIO)
   // Span DSP doesn't support more than 8Khz DTMF detection
   return new MprSpanDspDtmfDetector(rName, samplesPerFrame, samplesPerSec);
#else
   return new MprSimpleDtmfDetector(rName, samplesPerFrame, samplesPerSec);
#endif // defined(HAVE_SPAN_DSP) && defined(USE_SPAN_DSP_DTMF)

}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
