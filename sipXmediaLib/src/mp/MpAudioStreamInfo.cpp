//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpAudioStreamInfo.h"

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

MpAudioStreamInfo::MpAudioStreamInfo()
: m_inputLatency(0.0)
, m_outputLatency(0.0)
, m_sampleRate(0.0)
{

}

MpAudioStreamInfo::MpAudioStreamInfo(double inputLatency,
                                     double outputLatency,
                                     double sampleRate)
: m_inputLatency(inputLatency)
, m_outputLatency(outputLatency)
, m_sampleRate(sampleRate)
{

}

MpAudioStreamInfo::~MpAudioStreamInfo(void)
{

}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


