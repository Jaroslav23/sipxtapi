//
// Copyright (C) 2007 Jaroslav Libak
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsIntTypes.h> 
#include <os/OsSysLog.h>
#include <mp/MpPortAudioStreamBase.h>
#include "mp/MpVolumeMeter.h"
#include "portaudio.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType MpPortAudioStreamBase::TYPE = "MpPortAudioStreamBase";

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpPortAudioStreamBase::MpPortAudioStreamBase(MpAudioStreamId streamId,
                                             int outputChannelCount,
                                             int inputChannelCount,
                                             MpAudioDriverSampleFormat outputSampleFormat,
                                             MpAudioDriverSampleFormat inputSampleFormat,
                                             double sampleRate,
                                             unsigned long framesPerBuffer)
: m_streamId(streamId)
, m_outputChannelCount(outputChannelCount)
, m_inputChannelCount(inputChannelCount)
, m_outputSampleFormat(outputSampleFormat)
, m_inputSampleFormat(inputSampleFormat)
, m_sampleRate(sampleRate)
, m_framesPerBuffer(framesPerBuffer)
, m_inputSampleSize(0)
, m_outputSampleSize(0)
, m_inputVolumeMeter(NULL)
, m_outputVolumeMeter(NULL)
, m_inputPortStreamLatency(0.0)
, m_outputPortStreamLatency(0.0)
{
   switch(m_inputSampleFormat & 0x3f)
   {
   case MP_AUDIO_FORMAT_FLOAT32:
      m_inputSampleSize = sizeof(float);
      m_inputVolumeMeter = NULL;
      break;
   case MP_AUDIO_FORMAT_INT32:
      m_inputSampleSize = sizeof(int32_t);
      m_inputVolumeMeter = new MpVolumeMeter<int32_t, INT32_MAX>(inputChannelCount, sampleRate);
      break;
   case MP_AUDIO_FORMAT_INT24:
      m_inputSampleSize = sizeof(char)*3;
      m_inputVolumeMeter = NULL;
      OsSysLog::add(FAC_AUDIO, PRI_WARNING, "Dangerous input sample format selected, check MpPortAudioStreamBase.cpp\n");
      break;
   case MP_AUDIO_FORMAT_INT16:
      m_inputSampleSize = sizeof(int16_t);
      m_inputVolumeMeter = new MpVolumeMeter<int16_t, INT16_MAX>(inputChannelCount, sampleRate);
      break;
   case MP_AUDIO_FORMAT_INT8:
      m_inputSampleSize = sizeof(int8_t);
      m_inputVolumeMeter = new MpVolumeMeter<int8_t, INT8_MAX>(inputChannelCount, sampleRate);
      break;
   case MP_AUDIO_FORMAT_UINT8:
      m_inputSampleSize = sizeof(uint8_t);
      m_inputVolumeMeter = NULL;
      break;
   default:
      // don't create buffers, unsupported sample format
      break;
   }

   switch(m_outputSampleFormat & 0x3f)
   {
   case MP_AUDIO_FORMAT_FLOAT32:
      m_outputSampleSize = sizeof(float);
      m_outputVolumeMeter = NULL;
      break;
   case MP_AUDIO_FORMAT_INT32:
      m_outputSampleSize = sizeof(int32_t);
      m_outputVolumeMeter = new MpVolumeMeter<int32_t, INT32_MAX>(outputChannelCount, sampleRate);
      break;
   case MP_AUDIO_FORMAT_INT24:
      m_outputSampleSize = sizeof(char)*3;
      m_outputVolumeMeter = NULL;
      OsSysLog::add(FAC_AUDIO, PRI_WARNING, "Dangerous output sample format selected, check MpPortAudioStreamBase.cpp\n");
      break;
   case MP_AUDIO_FORMAT_INT16:
      m_outputSampleSize = sizeof(int16_t);
      m_outputVolumeMeter = new MpVolumeMeter<int16_t, INT16_MAX>(outputChannelCount, sampleRate);
      break;
   case MP_AUDIO_FORMAT_INT8:
      m_outputSampleSize = sizeof(int8_t);
      m_outputVolumeMeter = new MpVolumeMeter<int8_t, INT8_MAX>(outputChannelCount, sampleRate);
      break;
   case MP_AUDIO_FORMAT_UINT8:
      m_outputSampleSize = sizeof(uint8_t);
      m_outputVolumeMeter = NULL;
      break;
   default:
      // don't create buffers, unsupported sample format
      break;
   }

   if (m_sampleRate < MIN_PORT_SAMPLE_RATE)
   {
      // we need a minimum due to division
      m_sampleRate = MIN_PORT_SAMPLE_RATE;
   }
}

MpPortAudioStreamBase::~MpPortAudioStreamBase()
{
   // delete volume meters if they exist
   delete m_inputVolumeMeter;
   m_inputVolumeMeter = NULL;
   delete m_outputVolumeMeter;
   m_outputVolumeMeter = NULL;
}

/* ============================ MANIPULATORS ============================== */

void MpPortAudioStreamBase::resetStream()
{
   if (m_inputVolumeMeter)
   {
      m_inputVolumeMeter->resetMeter();
   }

   if (m_outputVolumeMeter)
   {
      m_outputVolumeMeter->resetMeter();
   }
}

/* ============================ ACCESSORS ================================= */

unsigned MpPortAudioStreamBase::hash() const
{
   return (unsigned)this;
}

UtlContainableType MpPortAudioStreamBase::getContainableType() const
{
   return MpPortAudioStreamBase::TYPE;
}


double MpPortAudioStreamBase::getSampleRate() const
{
   return m_sampleRate;
}

/* ============================ INQUIRY =================================== */

int MpPortAudioStreamBase::compareTo(UtlContainable const* inVal) const
{
   int result;

   if (inVal->isInstanceOf(MpPortAudioStreamBase::TYPE))
   {
      result = hash() - inVal->hash();
   }
   else
   {
      result = -1; 
   }

   return result;
}

double MpPortAudioStreamBase::getInputStreamVolume(MP_VOLUME_METER_TYPE type) const
{
   if (m_inputVolumeMeter)
   {
      switch(type)
      {
      case MP_VOLUME_METER_VU:
         return m_inputVolumeMeter->getVUVolume();
      case MP_VOLUME_METER_PPM:
         return m_inputVolumeMeter->getPPMVolume();
      default:
         return 0;
      }
   }

   return 0;
}

double MpPortAudioStreamBase::getOutputStreamVolume(MP_VOLUME_METER_TYPE type) const
{
   if (m_outputVolumeMeter)
   {
      switch(type)
      {
      case MP_VOLUME_METER_VU:
         return m_outputVolumeMeter->getVUVolume();
      case MP_VOLUME_METER_PPM:
         return m_outputVolumeMeter->getPPMVolume();
      default:
         return 0;
      }
   }

   return 0;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


