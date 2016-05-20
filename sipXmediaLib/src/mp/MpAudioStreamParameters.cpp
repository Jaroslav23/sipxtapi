//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpAudioStreamParameters.h"

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

MpAudioStreamParameters::MpAudioStreamParameters() : m_deviceIndex(0)
, m_channelCount(0)
, m_sampleFormat(0)
, m_suggestedLatency(0.0)
{

}

MpAudioStreamParameters::MpAudioStreamParameters(MpAudioDeviceIndex deviceIndex,
                                                 int channelCount,
                                                 MpAudioDriverSampleFormat sampleFormat,
                                                 double suggestedLatency)
: m_deviceIndex(deviceIndex)
, m_channelCount(channelCount)
, m_sampleFormat(sampleFormat)
, m_suggestedLatency(suggestedLatency)
{

}

MpAudioStreamParameters::~MpAudioStreamParameters(void)
{

}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


