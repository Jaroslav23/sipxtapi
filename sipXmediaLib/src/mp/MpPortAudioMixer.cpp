//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef DISABLE_LOCAL_AUDIO

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlDefs.h>
#include <os/OsLock.h>
#include "mp/MpPortAudioMixer.h"
#include "mp/MpPortAudioDriver.h"

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

MpPortAudioMixer::~MpPortAudioMixer(void)
{
   // we share lock with MpPortAudioDriver
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      Px_CloseMixer(m_pxMixer);
   }
}

MpPortAudioMixer* MpPortAudioMixer::createMixer(MpAudioStreamId stream,
                                                int mixerIndex)
{
   // only MpPortAudioDriver can call this, and it already has a lock

   PxMixer* pxMixer = Px_OpenMixer(stream, mixerIndex);

   if (pxMixer)
   {
      MpPortAudioMixer* pMixer = new MpPortAudioMixer();
      pMixer->m_pxMixer = pxMixer;
      return pMixer;
   }

   return NULL;
}

/* ============================ MANIPULATORS ============================== */

int MpPortAudioMixer::getNumMixers() const
{
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      return Px_GetNumMixers(m_pxMixer);
   }
   
   return 0;
}

void MpPortAudioMixer::getMixerName(UtlString& name, int i) const
{
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      name = Px_GetMixerName(m_pxMixer, i);
   }
   else
   {
      name = NULL;
   }
}

MpAudioVolume MpPortAudioMixer::getMasterVolume() const
{
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      return Px_GetMasterVolume(m_pxMixer);
   }

   return 0.0;
}

void MpPortAudioMixer::setMasterVolume(MpAudioVolume volume)
{
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      Px_SetMasterVolume(m_pxMixer, volume);
   }
}

MpAudioVolume MpPortAudioMixer::getPCMOutputVolume() const
{
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      return Px_GetPCMOutputVolume(m_pxMixer);
   }

   return 0.0;
}

void MpPortAudioMixer::setPCMOutputVolume(MpAudioVolume volume)
{
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      Px_SetPCMOutputVolume(m_pxMixer, volume);
   }
}

UtlBoolean MpPortAudioMixer::supportsPCMOutputVolume() const
{
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      return Px_SupportsPCMOutputVolume(m_pxMixer);
   }

   return FALSE;
}

int MpPortAudioMixer::getNumOutputVolumes() const
{
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      return Px_GetNumOutputVolumes(m_pxMixer);
   }

   return 0;
}

void MpPortAudioMixer::getOutputVolumeName(UtlString& name, int i) const
{
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      name = Px_GetOutputVolumeName(m_pxMixer, i);
   }
   else
   {
      name = NULL;
   }
}

MpAudioVolume MpPortAudioMixer::getOutputVolume(int i) const
{
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      return Px_GetOutputVolume(m_pxMixer, i);
   }

   return 0.0;
}

void MpPortAudioMixer::setOutputVolume(int i, MpAudioVolume volume)
{
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      Px_SetOutputVolume(m_pxMixer, i, volume);
   }
}

int MpPortAudioMixer::getNumInputSources() const
{
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      return Px_GetNumInputSources(m_pxMixer);
   }

   return 0;
}

void MpPortAudioMixer::getInputSourceName(UtlString& name, int i) const
{
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      name = Px_GetInputSourceName(m_pxMixer, i);
   }
   else
   {
      name = NULL;

   }
}

int MpPortAudioMixer::getCurrentInputSource() const
{
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      return Px_GetCurrentInputSource(m_pxMixer);
   }

   return 0;
}

void MpPortAudioMixer::setCurrentInputSource(int i)
{
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      Px_SetCurrentInputSource(m_pxMixer, i);
   }
}

MpAudioVolume MpPortAudioMixer::getInputVolume() const
{
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      return Px_GetInputVolume(m_pxMixer);
   }

   return 0.0;
}

void MpPortAudioMixer::setInputVolume(MpAudioVolume volume)
{
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      Px_SetInputVolume(m_pxMixer, volume);
   }
}

UtlBoolean MpPortAudioMixer::supportsOutputBalance() const
{
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      return Px_SupportsOutputBalance(m_pxMixer);
   }

   return FALSE;
}

MpAudioBalance MpPortAudioMixer::getOutputBalance() const
{
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      return Px_GetOutputBalance(m_pxMixer);
   }

   return 0.0;
}

void MpPortAudioMixer::setOutputBalance(MpAudioBalance balance)
{
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      Px_SetOutputBalance(m_pxMixer, balance);
   }
}

UtlBoolean MpPortAudioMixer::supportsPlaythrough() const
{
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      return Px_SupportsPlaythrough(m_pxMixer);
   }

   return FALSE;
}

MpAudioVolume MpPortAudioMixer::getPlaythrough() const
{
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      return Px_GetPlaythrough(m_pxMixer);
   }

   return 0.0;
}

void MpPortAudioMixer::setPlaythrough(MpAudioVolume volume)
{
   OsLock lock(MpPortAudioDriver::ms_driverMutex);

   if (m_pxMixer)
   {
      Px_SetPlaythrough(m_pxMixer, volume);
   }
}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

MpPortAudioMixer::MpPortAudioMixer()
: m_pxMixer(NULL)
{

}

/* ============================ FUNCTIONS ================================= */

#endif // DISABLE_LOCAL_AUDIO
