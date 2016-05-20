// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
//
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <math.h>

// APPLICATION INCLUDES
#include <os/OsLock.h>
#include "os/OsIntTypes.h"
#include <os/OsSysLog.h>
#include <mp/MpJitterBufferDefault.h>
#include <mp/MpRtpBuf.h>
#include <mp/MpDspUtils.h>

// DEFINES
//#define PRINT_STATISTICS

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS

#define WRAP_NUMBER(number, range)  \
   {                                \
      if ((number) < 0)             \
      {                             \
         number = (range) - (-(number)%(range));   \
      }                             \
      else                          \
      {                             \
         number = (number)%(range); \
      }                             \
   }


// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/*
static void appendToFile(const char* fileName, const char* text)
{
   FILE* f = fopen(fileName, "a");
   fprintf(f, text);
   fclose(f);
}
*/

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpJitterBufferDefault::MpJitterBufferDefault(const UtlString& name,
                                         int payloadType,
                                         unsigned int samplesPerFrame,
                                         bool bUsePrefetch,
                                         unsigned int initialPrefetchCount,
                                         unsigned int minPrefetchCount,
                                         unsigned int maxPrefetchCount,
                                         bool bDoPLC,
                                         unsigned int maxConcealedFrames,
                                         bool bAutodetectPtime,
                                         unsigned int ptime)
: MpJitterBufferBase(name, payloadType, samplesPerFrame)
, m_bUsePrefetch(bUsePrefetch)
, m_lastSSRC(0)
, m_bFirstFrame(true)
, m_mutex(OsMutex::Q_FIFO)
, m_bufferLength(0)
, m_lastPushed(0)
, m_lastPulled(0)
, m_expectedTimestamp(0)
, m_prefetchCount(min(initialPrefetchCount, MAX_RTP_PACKETS))
, m_bPrefetchMode(true)
, m_internalClock(0)
, m_minPrefetchCount(min(minPrefetchCount, MAX_RTP_PACKETS))
, m_maxPrefetchCount(min(maxPrefetchCount, MAX_RTP_PACKETS))
, m_iPLCCounter(0)
, m_bDoPLC(bDoPLC)
, m_iMaxConcealedFrames(maxConcealedFrames)
, m_pTime(ptime)
, m_initialPTime(ptime)
, m_bAutodetectPtime(bAutodetectPtime)
{
#ifdef PRINT_STATISTICS
   enableConsoleOutput(TRUE);
#endif
   if (m_prefetchCount < m_minPrefetchCount)
   {
      m_prefetchCount = m_minPrefetchCount;
   }
   if (!bDoPLC && bAutodetectPtime)
   {
      // if PLC is not enabled, disable ptime autodetection and assume ptime is equal to
      // sipxmedialib internal audio frame size
      bAutodetectPtime = false;
      m_pTime = samplesPerFrame;
      m_initialPTime = samplesPerFrame;
   }
}

MpJitterBufferDefault::~MpJitterBufferDefault()
{
}

/* ============================ MANIPULATORS ============================== */

void MpJitterBufferDefault::reset()
{
   OsLock lock(m_mutex);

   for (int i = 0; i < MAX_RTP_PACKETS; i++)
   {
      m_pPackets[i].release(); // assign NULL to MpRtpBufPtr by release(), as = NULL doesnt work
   }
   m_lastPushed = 0;
   m_lastPulled = 0;
   m_bufferLength = 0;
   m_expectedTimestamp = 0;
   m_internalClock = 0;
   m_bPrefetchMode = true;

   if (m_bDoPLC)
   {
      m_iPLCCounter = 0;
      m_pPLC.release();
   }

   if (m_bAutodetectPtime)
   {
      // autodetect ptime
      m_pTime = m_initialPTime;
   }
}

void MpJitterBufferDefault::frameIncrement()
{
   if (!m_bPrefetchMode || !m_bUsePrefetch)
   {
      // wrap around is done automatically for unsigned
      m_expectedTimestamp = m_expectedTimestamp + m_samplesPerFrame;
   }
   // if in prefetch mode, we can't increment timestamp, as it could cause us to skip
   // whole prefetch buffer. If prefetch is disabled, always increment

   // always advance internal clock
   m_internalClock = m_internalClock + m_samplesPerFrame;
}

JitterBufferResult MpJitterBufferDefault::pull(MpRtpBufPtr &pOutRtp)
{
   OsLock lock(m_mutex);

   if (!m_bFirstFrame)
   {
      // only increment pulls since 1st frame was received
      m_statistics.m_totalPulls++;
   }
   
#ifdef PRINT_STATISTICS
   if (m_statistics.m_totalPulls % 100 == 1)
   {
      // print them
      OsSysLog::add(FAC_AUDIO, PRI_DEBUG,"---- Jitter Buffer Statistics for %s ----\n", m_name.data());
      OsSysLog::add(FAC_AUDIO, PRI_DEBUG,"m_totalHits: %u\n", m_statistics.m_totalHits);
      OsSysLog::add(FAC_AUDIO, PRI_DEBUG,"m_total2ndHits: %u\n", m_statistics.m_total2ndHits);
      OsSysLog::add(FAC_AUDIO, PRI_DEBUG,"m_totalPLCHits: %u\n", m_statistics.m_totalPLCHits);
      OsSysLog::add(FAC_AUDIO, PRI_DEBUG,"m_totalNormalUnderflows: %u\n", m_statistics.m_totalNormalUnderflows);
      OsSysLog::add(FAC_AUDIO, PRI_DEBUG,"m_totalPrefetchUnderflows: %u\n", m_statistics.m_totalPrefetchUnderflows);
      OsSysLog::add(FAC_AUDIO, PRI_DEBUG,"m_totalPulls: %u\n", m_statistics.m_totalPulls);
      OsSysLog::add(FAC_AUDIO, PRI_DEBUG,"m_totalPushCollisions: %u\n", m_statistics.m_totalPushCollisions);
      OsSysLog::add(FAC_AUDIO, PRI_DEBUG,"m_totalPushReplacements: %u\n", m_statistics.m_totalPushReplacements);
      OsSysLog::add(FAC_AUDIO, PRI_DEBUG,"m_totalPushInserts: %u\n", m_statistics.m_totalPushInserts);
      OsSysLog::add(FAC_AUDIO, PRI_DEBUG,"m_bufferLength: %u\n", m_bufferLength);      
      OsSysLog::add(FAC_AUDIO, PRI_DEBUG,"m_lastSeqNumber: %u\n", m_lastSeqNumber);      
      OsSysLog::add(FAC_AUDIO, PRI_DEBUG,"-------------------------------------------\n");
   }
#endif

   if (m_bUsePrefetch && m_bPrefetchMode)
   {
      m_statistics.m_totalPrefetchUnderflows++;
      // we are in prefetch mode - waiting to fill buffer
      return MP_JITTER_BUFFER_MISSING;
   }
   
   if (m_bufferLength == 0)
   {
      if (m_bUsePrefetch)
      {
         setOptimalPrefetchCount();
      }
      m_bPrefetchMode = true;
      // jitter buffer is empty
      m_statistics.m_totalNormalUnderflows++;
      return MP_JITTER_BUFFER_MISSING;
   }
   
   // we will start pulling just after the last pull
   int iNextPull = (m_lastPulled + 1) % MAX_RTP_PACKETS;
   int secondCandidate = -1;
   RtpTimestamp frameSizeHalf = m_samplesPerFrame / 2;

   for (int i = 0; i < MAX_RTP_PACKETS; i++)
   {
      if (m_pPackets[iNextPull].isValid())
      {
         // buffer is valid, check seq and timestamp
         RtpSeq iSeqNo = m_pPackets[iNextPull]->getRtpSequenceNumber();
         RtpTimestamp iTimestamp = m_pPackets[iNextPull]->getRtpTimestamp();

         if (MpDspUtils::compareSerials(iSeqNo, m_lastSeqNumber) < 0)
         {
            // we found old frame that we don't want to play, discard it
            m_pPackets[iNextPull].release();
            m_bufferLength--;
            continue;
         }

         // sequence number is ok, now check timestamp
         if (secondCandidate == -1)
         {
            // this candidate will be used if we don't find frame for matching timestamp
            secondCandidate = iNextPull;
         }

         // wrapping around max and 0 is done automatically for unsigned
         RtpTimestamp upperBound = m_expectedTimestamp + frameSizeHalf;
         RtpTimestamp lowerBound = m_expectedTimestamp - frameSizeHalf;

         if (MpDspUtils::compareSerials(iTimestamp, upperBound) < 0
            && MpDspUtils::compareSerials(iTimestamp, lowerBound) > 0)
         {
            if (m_bAutodetectPtime && m_bDoPLC && m_pPLC.isValid())
            {
               // autodetect ptime, we can currently do it only if PLC is also enabled
               m_pTime = min(m_pTime, iTimestamp - m_pPLC->getRtpTimestamp());
            }
            
            // we found the expected frame, use it
            pOutRtp.swap(m_pPackets[iNextPull]);
            // Make sure we does not have copy of this buffer left in other threads.
            pOutRtp.requestWrite();

            if (m_bDoPLC)
            {
               m_pPLC = pOutRtp.clone(); // clone the frame for PLC purpose
               m_iPLCCounter = 0;
            }
            assert(!m_pPackets[iNextPull].isValid());

            m_lastSeqNumber = iSeqNo;
            m_lastPulled = iNextPull;
            m_expectedTimestamp = iTimestamp; // next time it will be incremented in frameIncrement
            m_statistics.m_totalHits++;

            m_bufferLength--;
            return MP_JITTER_BUFFER_OK;
         }
      }

      // Wrap iNextPull counter if we reach end of buffer
      iNextPull = (iNextPull + 1) % MAX_RTP_PACKETS;
   }

   // we didn't find frame with expected timestamp, use the 2nd candidate if available
   if (secondCandidate != -1)
   {
      assert(m_pPackets[secondCandidate].isValid());
      RtpSeq secCandSeq = m_pPackets[secondCandidate]->getRtpSequenceNumber();

      if (m_bDoPLC)
      {
         // PLC is enabled
         RtpSeq seqDiff = abs(m_lastSeqNumber - secCandSeq);

         if (seqDiff > 1 && m_pPLC.isValid())
         {
            // packet loss detected, play back last frame if available
            if (m_iPLCCounter < m_iMaxConcealedFrames)
            {
               // PLC frame is valid, use it
               m_lastSeqNumber = m_pPLC->getRtpSequenceNumber() + (m_iPLCCounter + 1);
               m_lastPulled = (m_lastPulled + 1) % MAX_RTP_PACKETS;
               // estimate timestamp of the "lost" frame
               m_expectedTimestamp = m_pPLC->getRtpTimestamp() + m_pTime*(m_iPLCCounter + 1);
               m_statistics.m_totalPLCHits++;
               m_iPLCCounter++;

               pOutRtp = m_pPLC.clone();
               // Make sure we does not have copy of this buffer left in other threads.
               pOutRtp.requestWrite();
               return MP_JITTER_BUFFER_PLC;
            }
            else
            {
               // discard PLC frame
               m_pPLC.release();
            }
         }
         // we probably experienced packet loss, but can't conceal it, so we will actually skip a few frames
      }      
      
      m_lastSeqNumber = secCandSeq;
      m_lastPulled = secondCandidate;
      m_expectedTimestamp = m_pPackets[secondCandidate]->getRtpTimestamp(); // next time it will be incremented in frameIncrement
      m_statistics.m_total2ndHits++;

      pOutRtp.swap(m_pPackets[secondCandidate]);
      // Make sure we does not have copy of this buffer left in other threads.
      pOutRtp.requestWrite();

      m_bufferLength--;
      return MP_JITTER_BUFFER_FRAME_SKIP;
   }
   
   // we didn't find any suitable frame :(
   m_bPrefetchMode = true;
   setOptimalPrefetchCount();
   m_statistics.m_totalNormalUnderflows++;
   return MP_JITTER_BUFFER_MISSING;
}

void MpJitterBufferDefault::push(const MpRtpBufPtr &pRtp)
{  
   if (pRtp.isValid() && pRtp->getRtpPayloadType() == m_payloadType)
   {
      OsLock lock(m_mutex);

      RtpSeq inRtpSeq = pRtp->getRtpSequenceNumber();
      // for first frame, setup SSRC
      if (m_bFirstFrame)
      {
         if (!m_bUsePrefetch)
         {
            // init jitter buffer variables
            // so that it can catch up at the correct index during pull
            initJitterBuffer(pRtp);
         }
         m_bFirstFrame = false;
      }
      else
      if (m_lastSSRC != pRtp->getRtpSSRC())
      {
         // SSRC changed, reset jitter buffer
         reset();
         if (!m_bUsePrefetch)
         {
            // init jitter buffer variables
            // so that it can catch up at the correct index during pull
            initJitterBuffer(pRtp);
         }
      }

      // Find place for incoming packet
      int index = (inRtpSeq % MAX_RTP_PACKETS);

      // Place packet to the buffer
      if (m_pPackets[index].isValid())
      {
         m_statistics.m_totalPushCollisions++;
         // Check for packets already in the buffer. Overwrite them if 
         // the just-arriving packet is newer than the existing packet
         // Don't overwrite if the just-arriving packet is older
         RtpSeq iBufSeqNo = m_pPackets[index]->getRtpSequenceNumber();

         if (MpDspUtils::compareSerials(inRtpSeq, iBufSeqNo) > 0) 
         {
            m_statistics.m_totalPushReplacements++;
            // Insert the new packet over the old packet
            m_pPackets[index] = pRtp;
            m_lastPushed = index;  
            // m_bufferLength remains unchanged, since we discarded a packet, and added one

            if (m_bUsePrefetch)
            {
               // add sample to statistics
               updateArrivalDiffs(pRtp);
            }
         }
         else
         {
            // Don't insert the new packet - it is a old delayed packet
            return;
         }
      }
      else
      {
         m_statistics.m_totalPushInserts++;
         m_lastPushed = index;
         m_pPackets[index] = pRtp;
         m_bufferLength++;

         if (m_bUsePrefetch)
         {
            if (m_bPrefetchMode)
            {
               if (m_bufferLength == 1)
               {
                  // first frame in prefetch mode was received, init jitter buffer variables
                  // so that it can catch up at the correct index during pull
                  initJitterBuffer(pRtp);
               }
               else
               if (m_bufferLength >= m_prefetchCount)
               {
                  m_bPrefetchMode = false; // turn off prefetch mode, so that we can start pulling
               }
            }

            // add sample to statistics
            updateArrivalDiffs(pRtp);
         }
      }
   }
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

int MpJitterBufferDefault::getBufferLength()
{
   return m_bufferLength;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void MpJitterBufferDefault::setOptimalPrefetchCount()
{
   if (m_statistics.m_frameDiffsCount > 0)
   {
      int shortPrefetch = getOptimalPrefetchCount(FEW_STATISTICS_SAMPLES);
      int longPrefetch = getOptimalPrefetchCount(MAX_STATISTICS_SAMPLES);

      m_prefetchCount = max(shortPrefetch, longPrefetch);
#ifdef PRINT_STATISTICS
      osPrintf("Changing m_prefetchCount to %u\n", m_prefetchCount);
#endif
   }  
}

int MpJitterBufferDefault::getOptimalPrefetchCount(unsigned int maxAnalyzeFrames)
{
   unsigned int framesToAnalyze = min(m_statistics.m_frameDiffsCount, maxAnalyzeFrames);

   if (framesToAnalyze > 0)
   {
      // calculate optimal prefetch count by chebyshev's inequality
      double fEX = 0.0f;

      int startIndex = m_statistics.m_frameDiffsNextIndex - framesToAnalyze;
      WRAP_NUMBER(startIndex, MAX_STATISTICS_SAMPLES);
      // now startIndex points to the first sample

      int index = startIndex;
      // compute sum of all arrival diffs
      for (unsigned int i = 0; i < framesToAnalyze; i++)
      {
         fEX += m_statistics.m_frameArrivalDiffs[index];
         index = (index + 1) % MAX_STATISTICS_SAMPLES;
      }
      // now compute EX
      fEX = fEX / framesToAnalyze;
      int iEX = (int)fEX;

      double varX = 0.0f;
      int tmp;
      index = startIndex; // set index to start index again
      // compute (X - EX)^2 values
      for (unsigned int i = 0; i < framesToAnalyze; i++)
      {
         tmp = m_statistics.m_frameArrivalDiffs[index] - iEX;
         varX += (tmp * tmp);
         index = (index + 1) % MAX_STATISTICS_SAMPLES;
      }
      varX = varX / framesToAnalyze;

      double maxProbability = 0.09f;
      int epsilon = (int)ceil(sqrt(varX / maxProbability) / m_samplesPerFrame); // round up

      // now probability that we need higher prefetch count than epsilon is lower than maxProbability
      return min(max(epsilon, m_minPrefetchCount), m_maxPrefetchCount);
   }
   else
   return m_minPrefetchCount;
}

void MpJitterBufferDefault::updateArrivalDiffs(const MpRtpBufPtr &pRtp)
{
   int diff = abs((int)(pRtp->getRtpTimestamp() - m_internalClock));
   m_statistics.m_frameArrivalDiffs[m_statistics.m_frameDiffsNextIndex] = diff;
   m_statistics.m_frameDiffsNextIndex = (m_statistics.m_frameDiffsNextIndex + 1) % MAX_STATISTICS_SAMPLES;
   if (m_statistics.m_frameDiffsCount < MAX_STATISTICS_SAMPLES)
   {
      m_statistics.m_frameDiffsCount++;
   }   
}

void MpJitterBufferDefault::initJitterBuffer(const MpRtpBufPtr &pRtp)
{
   int index = pRtp->getRtpSequenceNumber() % MAX_RTP_PACKETS;

   m_lastSSRC = pRtp->getRtpSSRC();

   // now init data for pulling correct frames
   m_lastPulled = index - 1;
   WRAP_NUMBER(m_lastPulled, MAX_RTP_PACKETS);
   // these 2 wrap around correctly automatically, since they are unsigned
   m_lastSeqNumber = pRtp->getRtpSequenceNumber() - 1;
   m_expectedTimestamp = pRtp->getRtpTimestamp() - m_samplesPerFrame;
   m_internalClock = pRtp->getRtpTimestamp();
}

/* ============================ FUNCTIONS ================================= */

