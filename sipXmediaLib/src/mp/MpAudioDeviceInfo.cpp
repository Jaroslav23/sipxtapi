//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpAudioDeviceInfo.h"

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

MpAudioDeviceInfo::MpAudioDeviceInfo() : m_name(NULL)
, m_hostApi(0)
, m_hostApiName(0)
, m_maxInputChannels(0)
, m_maxOutputChannels(0)
, m_defaultLowInputLatency(0.0)
, m_defaultLowOutputLatency(0.0)
, m_defaultHighInputLatency(0.0)
, m_defaultHighOutputLatency(0.0)
, m_defaultSampleRate(0)
{

}

MpAudioDeviceInfo::~MpAudioDeviceInfo(void)
{

}

MpAudioDeviceInfo::MpAudioDeviceInfo(const MpAudioDeviceInfo& rMpAudioDeviceInfo)
{
   *this = rMpAudioDeviceInfo;
}

MpAudioDeviceInfo& MpAudioDeviceInfo::operator=(const MpAudioDeviceInfo& rhs)
{
   if (&rhs == this)
   {
      return *this;
   }

   m_name = rhs.getName();
   m_hostApi = rhs.getHostApi();
   m_hostApiName = rhs.getHostApiName();
   m_maxInputChannels = rhs.getMaxInputChannels();
   m_maxOutputChannels = rhs.getMaxOutputChannels();
   m_defaultLowInputLatency = rhs.getDefaultLowInputLatency();
   m_defaultLowOutputLatency = rhs.getDefaultLowOutputLatency();
   m_defaultHighInputLatency = rhs.getDefaultHighInputLatency();
   m_defaultHighOutputLatency = rhs.getDefaultHighOutputLatency();
   m_defaultSampleRate = rhs.getDefaultSampleRate();

   return *this;
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

