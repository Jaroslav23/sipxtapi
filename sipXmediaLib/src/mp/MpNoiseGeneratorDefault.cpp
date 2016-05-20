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
#include <memory.h>

// APPLICATION INCLUDES
#include <mp/MpNoiseGeneratorDefault.h>

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

MpNoiseGeneratorDefault::MpNoiseGeneratorDefault()
{

}

/* ============================ MANIPULATORS ============================== */

void MpNoiseGeneratorDefault::generateComfortNoise(MpAudioSample* pSamplesBuffer, unsigned sampleCount)
{
   if (pSamplesBuffer && sampleCount >= 0)
   {
      memset(pSamplesBuffer, 0, sampleCount * sizeof(MpAudioSample));
   }
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
