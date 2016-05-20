//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpHostAudioApiInfo.h"

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

MpHostAudioApiInfo::MpHostAudioApiInfo() : m_typeId(MP_AUDIOAPI_INDEVELOPMENT)
, m_name(NULL)
, m_deviceCount(0)
, m_defaultInputDevice(0)
, m_defaultOutputDevice(0)
{

}

MpHostAudioApiInfo::MpHostAudioApiInfo(MpHostAudioApiTypeId typeId,
                                       const UtlString& name,
                                       int deviceCount,
                                       MpAudioDeviceIndex defaultInputDevice,
                                       MpAudioDeviceIndex defaultOutputDevice)
: m_typeId(typeId)
, m_name(name)
, m_deviceCount(deviceCount)
, m_defaultInputDevice(defaultInputDevice)
, m_defaultOutputDevice(defaultOutputDevice)
{

}

MpHostAudioApiInfo::~MpHostAudioApiInfo(void)
{

}

MpHostAudioApiInfo::MpHostAudioApiInfo(const MpHostAudioApiInfo& rMpHostAudioApiInfo)
{
   *this = rMpHostAudioApiInfo;
}

MpHostAudioApiInfo& MpHostAudioApiInfo::operator=(const MpHostAudioApiInfo& rhs)
{
   if (&rhs == this)
   {
      return *this;
   }
   
   m_typeId = rhs.getTypeId();
   m_name = rhs.getName();
   m_deviceCount = rhs.getDeviceCount();
   m_defaultInputDevice = rhs.getDefaultInputDevice();
   m_defaultOutputDevice = rhs.getDefaultOutputDevice();

   return *this;
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


