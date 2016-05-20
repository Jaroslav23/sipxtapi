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
#include <mp/MpSyncPortAudioStream.h>
#include <mp/MpVolumeMeterBase.h>
#include "portaudio.h"

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

MpSyncPortAudioStream::MpSyncPortAudioStream(MpAudioStreamId streamId,
                                             int outputChannelCount,
                                             int inputChannelCount,
                                             MpAudioDriverSampleFormat outputSampleFormat,
                                             MpAudioDriverSampleFormat inputSampleFormat,
                                             double sampleRate,
                                             unsigned long framesPerBuffer)
: MpPortAudioStreamBase(streamId, outputChannelCount, inputChannelCount,
                        outputSampleFormat, inputSampleFormat,
                        sampleRate, framesPerBuffer)
{

}

MpSyncPortAudioStream::~MpSyncPortAudioStream()
{

}

/* ============================ MANIPULATORS ============================== */

OsStatus MpSyncPortAudioStream::readStream(void *buffer, unsigned long frames)
{
   OsStatus status = OS_FAILED;

   PaError paError = Pa_ReadStream(m_streamId, buffer, frames);

   if (paError == paNoError)
   {
      // copy input frames to meter
      if (m_inputVolumeMeter)
      {
         m_inputVolumeMeter->pushBuffer(buffer, (unsigned int)frames);
      }

      status = OS_SUCCESS;
   }
   else if (paError == paInputOverflowed)
   {
      status = OS_OVERFLOW;
   }

   return status;
}

OsStatus MpSyncPortAudioStream::writeStream(const void *buffer, unsigned long frames)
{
   OsStatus status = OS_FAILED;

   // copy output frames to meter
   if (m_outputVolumeMeter)
   {
      m_outputVolumeMeter->pushBuffer(buffer, (unsigned int)frames);
   }

   PaError paError = Pa_WriteStream(m_streamId, buffer, frames);

   if (paError == paNoError)
   {
      status = OS_SUCCESS;
   }
   else if (paError == paOutputUnderflowed)
   {
      status = OS_OVERFLOW;
   }

   return status;
}

/* ============================ ACCESSORS ================================= */

double MpSyncPortAudioStream::getInputLatency() const
{
   return m_inputPortStreamLatency;
}

double MpSyncPortAudioStream::getOutputLatency() const
{
   return m_outputPortStreamLatency;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

