//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mi/CpAudioDeviceInfo.h"

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

CpAudioDeviceInfo::CpAudioDeviceInfo()
: m_deviceName(0)
, m_driverName(0)
, m_maxChannels(0)
, m_defaultSampleRate(0.0)
, m_bIsInput(FALSE)
{

}

CpAudioDeviceInfo::~CpAudioDeviceInfo(void)
{

}

CpAudioDeviceInfo::CpAudioDeviceInfo(const CpAudioDeviceInfo& rCpAudioDeviceInfo)
{
   *this = rCpAudioDeviceInfo;
}

CpAudioDeviceInfo& CpAudioDeviceInfo::operator=(const CpAudioDeviceInfo& rhs)
{
   if (&rhs == this)
   {
      return *this;
   }

   m_deviceName = rhs.m_deviceName;
   m_driverName = rhs.m_driverName;
   m_maxChannels = rhs.m_maxChannels;
   m_defaultSampleRate = rhs.m_defaultSampleRate;
   m_bIsInput = rhs.m_bIsInput;

   return *this;
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

