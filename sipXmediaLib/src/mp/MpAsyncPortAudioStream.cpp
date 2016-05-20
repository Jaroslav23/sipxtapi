//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef DISABLE_LOCAL_AUDIO

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <assert.h>
#include <os/OsIntTypes.h> 
#include <os/OsSysLog.h>
#include <os/OsDateTime.h>
#include "mp/MpAsyncPortAudioStream.h"
#include "mp/MpVolumeMeter.h"

// DEFINES
//#define DEBUG_AUDIO_STREAM
#define INPUT_PREFETCH_BUFFERS_COUNT 8
#define OUTPUT_PREFETCH_BUFFERS_COUNT 8

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpAsyncPortAudioStream::MpAsyncPortAudioStream(MpAudioStreamId streamId,
                                               int outputChannelCount,
                                               int inputChannelCount,
                                               MpAudioDriverSampleFormat outputSampleFormat,
                                               MpAudioDriverSampleFormat inputSampleFormat,
                                               double sampleRate,
                                               unsigned long framesPerBuffer)
: MpPortAudioStreamBase(streamId, outputChannelCount, inputChannelCount,
                        outputSampleFormat, inputSampleFormat,
                        sampleRate, framesPerBuffer)
, m_virtualFramesPerBuffer(0)
, m_pInputBuffer(NULL)
, m_pOutputBuffer(NULL)
, m_inputBufferSize(0)
, m_outputBufferSize(0)
, m_inputWritePos(0)
, m_inputReadPos(0)
, m_outputWritePos(0)
, m_outputReadPos(0)
, m_outputBufferOverflow(0)
, m_outputBufferUnderflow(0)
, m_inputBufferOverflow(0)
, m_inputBufferUnderflow(0)
, m_inputBufferPrefetchMode(true)
, m_outputBufferPrefetchMode(true)
, m_inputPrefetchCount(0)
, m_outputPrefetchCount(0)
, m_bFrameRecorded(false)
, m_bFramePushed(false)
, m_streamReadWriteCount(0)
, m_callbackCallCount(0)
{
   m_virtualFramesPerBuffer = m_framesPerBuffer;

   if (m_virtualFramesPerBuffer == 0)
   {
      // callback will accept frames of any size anyway...
      // use 160 for now, we need to create buffers of certain size, but will accept any number of frames
      m_virtualFramesPerBuffer = 160;
   }
   
   // allocate input buffer
   if (m_inputSampleFormat > 0 && m_inputChannelCount > 0)
   {
      unsigned int inputFrameSize = m_inputSampleSize * m_inputChannelCount;
      unsigned int inputBufferReserve = ((unsigned int)m_sampleRate / MIN_PORT_SAMPLE_RATE + 5);
      m_inputBufferSize = inputFrameSize * m_virtualFramesPerBuffer * inputBufferReserve;
      m_inputPrefetchCount = min(INPUT_PREFETCH_BUFFERS_COUNT, inputBufferReserve) * m_virtualFramesPerBuffer;
      m_pInputBuffer = malloc(m_inputBufferSize);

      if (!m_pInputBuffer)
      {
         // allocation error, don't use buffer
         m_inputBufferSize = 0;
      }
      else
      {
         // zero buffer
         memset(m_pInputBuffer, 0, m_inputBufferSize);
         // start writing at the next sample position
         m_inputWritePos = inputFrameSize;
      }
   }
   
   // allocate output buffer
   if (m_outputSampleSize > 0 && m_outputChannelCount > 0)
   {
      unsigned int outputFrameSize = m_outputSampleSize * m_outputChannelCount;
      unsigned int outputBufferReserve = ((unsigned int)m_sampleRate / MIN_PORT_SAMPLE_RATE + 5);
      m_outputBufferSize = outputFrameSize * m_virtualFramesPerBuffer * outputBufferReserve;
      m_outputPrefetchCount = min(OUTPUT_PREFETCH_BUFFERS_COUNT, outputBufferReserve) * m_virtualFramesPerBuffer;
      m_pOutputBuffer = malloc(m_outputBufferSize);

      if (!m_pOutputBuffer)
      {
         // allocation error, don't use buffer
         m_outputBufferSize = 0;
      }      
      else
      {
         // zero buffer
         memset(m_pOutputBuffer, 0, m_outputBufferSize);
         // start writing at the next sample position
         m_outputWritePos = outputFrameSize;
      }
   }
}

MpAsyncPortAudioStream::~MpAsyncPortAudioStream(void)
{
   if (m_pInputBuffer)
   {
      free(m_pInputBuffer);
      m_pInputBuffer = NULL;
   }
   
   if (m_pOutputBuffer)
   {
      free(m_pOutputBuffer);
      m_pOutputBuffer = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

int MpAsyncPortAudioStream::streamCallback(const void *input,
                                      void *output,
                                      unsigned long frameCount,
                                      const PaStreamCallbackTimeInfo* timeInfo,
                                      PaStreamCallbackFlags statusFlags,
                                      void *userData)
{
   if (userData)
   {
      MpAsyncPortAudioStream* instance = (MpAsyncPortAudioStream*)userData;

      return instance->instanceStreamCallback(input, output, frameCount, timeInfo, statusFlags);
   }

   return paContinue;
}

OsStatus MpAsyncPortAudioStream::readStream(void *buffer,
                                            unsigned long frames)
{
   OsStatus status = OS_FAILED;
  
#ifdef DEBUG_AUDIO_STREAM
   if (m_bFrameRecorded && m_streamReadWriteCount++ % 200 == 0)
   {
      this->printStatistics();
   }
#endif
   
   if (frames > 0 && m_inputSampleSize > 0)
   {
      if ((m_framesPerBuffer == 0) || (m_framesPerBuffer == frames))
      {
         // count number of required bytes in buffer
         unsigned int bytesRequired = m_inputSampleSize * m_inputChannelCount * frames;

         if (m_inputBufferPrefetchMode)
         {
            // input is in prefetch mode, send zeroes only
            memset(buffer, 0, bytesRequired);
            status = OS_PREFETCH;
            return status;
         }         

         unsigned int copyable = getCopyableBytes(m_inputReadPos, m_inputWritePos, m_inputBufferSize);
         unsigned int bytesToCopy = min(bytesRequired, copyable);
         unsigned int framesToCopy = bytesToCopy / (m_inputSampleSize * m_inputChannelCount);

         if (framesToCopy < frames)
         {
            // we are out of buffer
            if (m_bFrameRecorded)
            {
               // record underflows only since the 1st frame was recorded
               m_inputBufferUnderflow++;
            }
            status = OS_UNDERFLOW;
         }
         else if (framesToCopy == frames)
         {
            status = OS_SUCCESS;
         }

         if (framesToCopy > 0)
         {
            unsigned int realBytesToCopy = framesToCopy * m_inputSampleSize * m_inputChannelCount;

            // count1 is number of bytes to copy before wrapping occurs
            int count1 = min(realBytesToCopy, m_inputBufferSize - m_inputReadPos);
            // count 2 is number of bytes to copy after wrapping
            int count2 = realBytesToCopy - count1;
            // now copy some frames
            memcpy(buffer, (char*)m_pInputBuffer + m_inputReadPos, count1);
            if (count2 > 0)
            {
               // handle wrap around
               memcpy((char*)buffer + count1, m_pInputBuffer, count2);
            }
            m_inputReadPos = (m_inputReadPos + realBytesToCopy) % m_inputBufferSize;
         }
      }         
   }

   return status;
}

OsStatus MpAsyncPortAudioStream::writeStream(const void *buffer,
                                             unsigned long frames)
{
   OsStatus status = OS_FAILED;

#ifdef DEBUG_AUDIO_STREAM
   if (m_bFramePushed && m_streamReadWriteCount++ % 200 == 0)
   {
      this->printStatistics();
   }
#endif

   if (frames > 0 && m_outputSampleSize > 0)
   {
      if ((m_framesPerBuffer == 0) || (m_framesPerBuffer == frames))
      {
         // count number of required bytes in buffer
         unsigned int bytesRequired = m_outputSampleSize * m_outputChannelCount * frames;

         unsigned int copyable = getCopyableBytes(m_outputWritePos, m_outputReadPos, m_outputBufferSize);
         unsigned int bytesToCopy = min(bytesRequired, copyable);
         unsigned int framesToCopy = bytesToCopy / (m_outputSampleSize * m_outputChannelCount);

         if (framesToCopy < frames)
         {
            // we are out of buffer
            OsSysLog::add(FAC_AUDIO, PRI_WARNING, "Output stream buffer overflow!\n");
            m_outputBufferOverflow++;
            status = OS_OVERFLOW;
         }
         else if (framesToCopy == frames)
         {
            status = OS_SUCCESS;
         }

         if (framesToCopy > 0)
         {
            m_bFramePushed = true;
            unsigned int realBytesToCopy = framesToCopy * m_outputSampleSize * m_outputChannelCount;

            // count1 is number of bytes to copy before wrapping occurs
            int count1 = min(realBytesToCopy, m_outputBufferSize - m_outputWritePos);
            // count 2 is number of bytes to copy after wrapping
            int count2 = realBytesToCopy - count1;
            // now copy some frames
            memcpy((char*)m_pOutputBuffer + m_outputWritePos, buffer, count1);
            if (count2 > 0)
            {
               // handle wrap around
               memcpy(m_pOutputBuffer, (char*)buffer + count1, count2);
            }
            m_outputWritePos = (m_outputWritePos + realBytesToCopy) % m_outputBufferSize;
         }
      }         
   }
   
   return status;
}

void MpAsyncPortAudioStream::printStatistics()
{
   OsSysLog::add(FAC_AUDIO, PRI_DEBUG,"--------- MpPortAudioStream::printStatistics ---------\n");
   OsSysLog::add(FAC_AUDIO, PRI_DEBUG,"m_outputBufferOverflow = %d\n", m_outputBufferOverflow);
   OsSysLog::add(FAC_AUDIO, PRI_DEBUG,"m_outputBufferUnderflow = %d\n", m_outputBufferUnderflow);
   OsSysLog::add(FAC_AUDIO, PRI_DEBUG,"m_inputBufferOverflow = %d\n", m_inputBufferOverflow);
   OsSysLog::add(FAC_AUDIO, PRI_DEBUG,"m_inputBufferUnderflow = %d\n", m_inputBufferUnderflow);
   OsSysLog::add(FAC_AUDIO, PRI_DEBUG,"m_streamReadWriteCount = %d\n", m_streamReadWriteCount);
   OsSysLog::add(FAC_AUDIO, PRI_DEBUG,"m_callbackCallCount = %d\n", m_callbackCallCount);
   OsSysLog::add(FAC_AUDIO, PRI_DEBUG,"------------------------------------------------------\n");
}

void MpAsyncPortAudioStream::resetStream()
{
   MpPortAudioStreamBase::resetStream();

   m_inputWritePos = 0;
   m_inputReadPos = 0;
   m_outputWritePos = 0;
   m_outputReadPos = 0;

   // zero buffers
   if (m_pInputBuffer)
   {
      memset(m_pInputBuffer, 0, m_inputBufferSize);
      m_inputWritePos = m_inputSampleSize * m_inputChannelCount;
   }

   if (m_pOutputBuffer)
   {
      memset(m_pOutputBuffer, 0, m_outputBufferSize);
      m_outputWritePos = m_outputSampleSize * m_outputChannelCount;
   }

   // reset statistics
   m_outputBufferOverflow = 0;
   m_outputBufferUnderflow = 0;
   m_inputBufferOverflow = 0;
   m_inputBufferUnderflow = 0;
   m_bFrameRecorded = false;
   m_bFramePushed = false;
   m_inputBufferPrefetchMode = true;
   m_outputBufferPrefetchMode = true;
}

/* ============================ ACCESSORS ================================= */

double MpAsyncPortAudioStream::getInputLatency() const
{
   if (m_inputSampleSize != 0 && m_inputChannelCount != 0)
   {
      return m_inputPortStreamLatency + (double)m_inputPrefetchCount / (m_sampleRate * m_inputSampleSize * m_inputChannelCount);
   }
   else
   {
      return 0.0;
   }
}

double MpAsyncPortAudioStream::getOutputLatency() const
{
   if (m_outputSampleSize != 0 && m_outputChannelCount != 0)
   {
      return m_outputPortStreamLatency + (double)m_outputPrefetchCount / (m_sampleRate * m_outputSampleSize * m_outputChannelCount);
   }
   else
   {
      return 0.0;
   }
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

int MpAsyncPortAudioStream::instanceStreamCallback(const void *input,
                                                   void *output,
                                                   unsigned long frameCount,
                                                   const PaStreamCallbackTimeInfo* timeInfo,
                                                   PaStreamCallbackFlags statusFlags)
{
   // portaudio supplied us pointers to its input and output buffers

#ifdef DEBUG_AUDIO_STREAM
   if (m_bFrameRecorded || m_bFramePushed)
   {
      m_callbackCallCount++;
   }   
#endif

   if (frameCount > 0)
   {
      // handle output frames
      if (m_outputChannelCount > 0 && output && m_outputSampleSize > 0)
      {
         // copy output frames to meter
         if (m_outputVolumeMeter)
         {
            m_outputVolumeMeter->pushBuffer(output, frameCount);
         }

         unsigned int outputFrameCount = getOutputBufferFrameCount();

         if (m_outputBufferPrefetchMode)
         {
            // prefetch is on
            if (outputFrameCount > m_outputPrefetchCount)
            {
               // we have enough frames, disable prefetch mode
               m_outputBufferPrefetchMode = false;
            }
         }
         else
         {
            // prefetch is off
            if (outputFrameCount <= 0)
            {
               // we are out of buffer
               if (m_bFramePushed)
               {
                  // record only underflows since the first push
                  m_outputBufferUnderflow++;
               }
               // buffer is empty, enable prefetch
               m_outputBufferPrefetchMode = true;
            }
         }

         // count number of required bytes in buffer
         unsigned int bytesRequired = m_outputSampleSize * m_outputChannelCount * frameCount;

         unsigned int copyable = 0;
         if (!m_outputBufferPrefetchMode)
         {
            copyable = getCopyableBytes(m_outputReadPos, m_outputWritePos, m_outputBufferSize);
         }
         unsigned int bytesToCopy = min(bytesRequired, copyable);
         unsigned int framesToCopy = bytesToCopy / (m_outputSampleSize * m_outputChannelCount);
         unsigned int zeroFrames = 0;

         if (framesToCopy < frameCount && !m_outputBufferPrefetchMode)
         {
            // we will have to copy zeroFrames filled with 0s, as we don't have enough data
            zeroFrames = frameCount - framesToCopy;
         }

         if (framesToCopy > 0)
         {
            unsigned int realBytesToCopy = framesToCopy * m_outputSampleSize * m_outputChannelCount;

            // count1 is number of bytes to copy before wrapping occurs
            int count1 = min(realBytesToCopy, m_outputBufferSize - m_outputReadPos);
            // count 2 is number of bytes to copy after wrapping
            int count2 = realBytesToCopy - count1;
            // now copy some frames
            memcpy(output, (char*)m_pOutputBuffer + m_outputReadPos, count1);
            if (count2 > 0)
            {
               // handle wrap around
               memcpy((char*)output + count1, m_pOutputBuffer, count2);
            }

            // fill zeroFrames with 0s
            if (zeroFrames > 0)
            {
               char* outputBuff = (char*)output + count1 + count2;
               unsigned int zeroBytesToCopy = zeroFrames * m_outputSampleSize * m_outputChannelCount;
               for (unsigned int i = 0; i < zeroBytesToCopy; i++)
               {
                  *outputBuff++ = 0;
               }
            }

            m_outputReadPos = (m_outputReadPos + realBytesToCopy) % m_outputBufferSize;
         }
         else
         {
            // prefetch is on, or buffer is empty
            // output zeros
            memset(output, 0, bytesRequired);
         }
      }

      // handle input frames
      if (m_inputChannelCount > 0 && input && m_inputSampleSize > 0)
      {
         // copy input frames to meter
         if (m_inputVolumeMeter)
         {
            m_inputVolumeMeter->pushBuffer(input, frameCount);
         }

         unsigned int inputFrameCount = getInputBufferFrameCount();

         if (m_inputBufferPrefetchMode)
         {
            // prefetch is on
            if (inputFrameCount > m_inputPrefetchCount)
            {
               // we have enough frames, disable prefetch mode
               m_inputBufferPrefetchMode = false;
            }
         }
         else
         {
            // prefetch is off
            if (inputFrameCount <= 0)
            {
               // buffer is empty, enable prefetch
               m_inputBufferPrefetchMode = true;
            }
         }

         // count number of required bytes in buffer
         unsigned int bytesRequired = m_inputSampleSize * m_inputChannelCount * frameCount;

         unsigned int copyable = getCopyableBytes(m_inputWritePos, m_inputReadPos, m_inputBufferSize);
         unsigned int bytesToCopy = min(bytesRequired, copyable);
         unsigned int framesToCopy = bytesToCopy / (m_inputSampleSize * m_inputChannelCount);

         if (framesToCopy < frameCount)
         {
            // we are out of buffer
            m_inputBufferOverflow++;
         }

         if (framesToCopy > 0)
         {
            m_bFrameRecorded = true;
            unsigned int realBytesToCopy = framesToCopy * m_inputSampleSize * m_inputChannelCount;

            // count1 is number of bytes to copy before wrapping occurs
            int count1 = min(realBytesToCopy, m_inputBufferSize - m_inputWritePos);
            // count 2 is number of bytes to copy after wrapping
            int count2 = realBytesToCopy - count1;
            // now copy some frames
            memcpy((char*)m_pInputBuffer + m_inputWritePos, input, count1);
            if (count2 > 0)
            {
               // handle wrap around
               memcpy(m_pInputBuffer, (char*)input + count1, count2);
            }

            // advance m_inputWritePos by realBytesToCopy
            m_inputWritePos = (m_inputWritePos + realBytesToCopy) % m_inputBufferSize;
         }
      }
   }   
   
   return paContinue;
}

int MpAsyncPortAudioStream::getCopyableBytes(unsigned int inputPos,
                                             unsigned int outputPos,
                                             unsigned int maxPos) const
{
   if (inputPos < outputPos)
   {
      if (inputPos < maxPos && outputPos < maxPos)
      {
         return outputPos - inputPos - 1;
      }      
   }
   else if (inputPos > outputPos)
   {
      if (outputPos < maxPos)
      {
         return maxPos - inputPos + outputPos - 1;
      }      
   }
   
   return 0;
}

int MpAsyncPortAudioStream::getInputBufferFrameCount()
{
   return getCopyableBytes(m_inputReadPos, m_inputWritePos, m_inputBufferSize) / (m_inputSampleSize * m_inputChannelCount);
}

int MpAsyncPortAudioStream::getOutputBufferFrameCount()
{
   return getCopyableBytes(m_outputReadPos, m_outputWritePos, m_outputBufferSize) / (m_outputSampleSize * m_outputChannelCount);
}

/* ============================ FUNCTIONS ================================= */


#endif // DISABLE_LOCAL_AUDIO
