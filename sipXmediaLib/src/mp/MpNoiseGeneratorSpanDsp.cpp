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

#ifdef HAVE_SPAN_DSP

// SYSTEM INCLUDES
#include <stdlib.h>

// APPLICATION INCLUDES
#include <mp/MpNoiseGeneratorSpanDsp.h>

// WIN32: Add libspandsp to linker input.
#ifdef WIN32 // [
#   ifdef _DEBUG // [
#      pragma comment(lib, "libspandspd.lib")
#   else // _DEBUG ][
#      pragma comment(lib, "libspandsp.lib")
#   endif // _DEBUG ]
#endif // WIN32 ]

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

MpNoiseGeneratorSpanDsp::MpNoiseGeneratorSpanDsp(int seed, float noiseLevel, int noiseQuality)
: m_pNoiseState(NULL)
{
   m_pNoiseState = noise_init_dbm0(NULL, seed, noiseLevel, NOISE_CLASS_HOTH, noiseQuality);
}

MpNoiseGeneratorSpanDsp::~MpNoiseGeneratorSpanDsp()
{
   if (m_pNoiseState)
   {
      free((void*)m_pNoiseState);
      m_pNoiseState = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

void MpNoiseGeneratorSpanDsp::generateComfortNoise(MpAudioSample* pSamplesBuffer, unsigned sampleCount)
{
   if (m_pNoiseState && pSamplesBuffer && sampleCount >= 0)
   {
      for (int i = 0; i < sampleCount; i++)
      {
         pSamplesBuffer[i] = noise(m_pNoiseState); // generate 1 sample of noise
      }
   }
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

#endif // HAVE_SPAN_DSP ]
