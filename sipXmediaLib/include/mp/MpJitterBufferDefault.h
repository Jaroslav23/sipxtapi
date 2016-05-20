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

#ifndef MpJitterBufferDefault_h__
#define MpJitterBufferDefault_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsMutex.h>
#include <mp/MpJitterBufferBase.h>
#include <mp/MpRtpBuf.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
// STRUCTS
// TYPEDEFS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/**
* Wrapper for default jitter buffer.
*/
class MpJitterBufferDefault : public MpJitterBufferBase
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum
   {
      MAX_RTP_PACKETS = 32,  ///< MUST BE A POWER OF 2, AND SHOULD BE >3
      MAX_STATISTICS_SAMPLES = 500,
      FEW_STATISTICS_SAMPLES = 10,
      DEFAULT_MIN_PREFETCH_COUNT = 6, // for 20ms frames, 120ms latency
      DEFAULT_MAX_PREFETCH_COUNT = 20 // for 20ms frames, 400ms latency
   };

   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   MpJitterBufferDefault(const UtlString& name,
      int payloadType,// codec payload id used in RTP
      unsigned int samplesPerFrame, // size of our internal frame in samples, 80 by default
      bool bUsePrefetch = true,
      unsigned int initialPrefetchCount = DEFAULT_MIN_PREFETCH_COUNT,
      unsigned int minPrefetchCount = DEFAULT_MIN_PREFETCH_COUNT,
      unsigned int maxPrefetchCount = DEFAULT_MAX_PREFETCH_COUNT,
      bool bDoPLC = false,
      unsigned int maxConcealedFrames = 0,
      bool bAutodetectPtime = true,
      unsigned int ptime = 640); // how much rtp seq of inbound frames will increment

   /**
   * Jitter buffer destructor. Will free internal resources.
   */
   virtual ~MpJitterBufferDefault();

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

   virtual void reset();

   virtual void frameIncrement();

   /**
   * Pulls one frame from jitter buffer and stores it into supplied MpRtpBufPtr.
   *
   * @param pOutRtp MpRtpBufPtr supplied by caller where we store the frame. Only
   *        payload, payload size, payload type and timestamp are initialized.
   */
   virtual JitterBufferResult pull(MpRtpBufPtr &pOutRtp);

   /**
   * Pushes an RTP frame into jitter buffer. Jitter buffer will throw out
   * a frame with incorrect payload type.
   */
   virtual void push(const MpRtpBufPtr &pRtp);

   //@}

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{
   //@}

   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{

      virtual int getBufferLength();

   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   typedef struct JitterBufferStatistics
   {
      unsigned int m_totalHits;
      unsigned int m_total2ndHits;
      unsigned int m_totalPLCHits;
      unsigned int m_totalNormalUnderflows;
      unsigned int m_totalPrefetchUnderflows;
      unsigned int m_totalPulls;

      unsigned int m_totalPushCollisions;
      unsigned int m_totalPushReplacements;
      unsigned int m_totalPushInserts;
      
      int m_frameArrivalDiffs[MAX_STATISTICS_SAMPLES];
      unsigned int m_frameDiffsCount;
      int m_frameDiffsNextIndex;

      JitterBufferStatistics()
      : m_totalPulls(0)
      , m_totalHits(0)
      , m_total2ndHits(0)
      , m_totalPLCHits(0)
      , m_totalNormalUnderflows(0)
      , m_totalPrefetchUnderflows(0)
      , m_totalPushCollisions(0)
      , m_totalPushReplacements(0)
      , m_totalPushInserts(0)
      , m_frameDiffsCount(0)
      , m_frameDiffsNextIndex(0)
      {
         memset(m_frameArrivalDiffs, 0, sizeof(m_frameArrivalDiffs));
      }
   } JitterBufferStatistics;

   void initJitterBuffer(const MpRtpBufPtr &pOutRtp);
   void updateArrivalDiffs(const MpRtpBufPtr &pRtp);
   void setOptimalPrefetchCount();
   int getOptimalPrefetchCount(unsigned int maxAnalyzeFrames);

   RtpSRC m_lastSSRC;   ///< last SSRC, if it changes we need to reset jitter buffer
   bool m_bFirstFrame;  ///< whether we have yet to receive the 1st frame
   int m_bufferLength;  ///< length of jitter buffer
   int m_prefetchCount; ///< how many RTP frames we need to have before we allow pulling
   bool m_bPrefetchMode; ///< whether we allow pulling frames from jitter buffer, if true we don't
   bool m_bUsePrefetch; ///< whether prefetching is enabled. If disabled, we allow pulling at any time, used for DTMF
   int m_minPrefetchCount; ///< minimum value for m_prefetchCount
   int m_maxPrefetchCount; ///< maximum value for m_prefetchCount

   RtpTimestamp m_expectedTimestamp;   ///< expected timestamp for pull
   RtpSeq m_lastSeqNumber; ///< sequence number of last pulled frame
   int m_lastPulled; ///< index of last pulled frame

   int m_lastPushed;   ///< Index of the last inserted packet.

   MpRtpBufPtr m_pPackets[MAX_RTP_PACKETS];   ///< Buffer for incoming RTP packets
   bool m_bAutodetectPtime; // whether ptime should be automatically detected
   bool m_bDoPLC; ///< whether PLC is enabled, must be disabled for RFC 2833
   MpRtpBufPtr m_pPLC; ///< 1 RTP frame for PLC purpose
   unsigned int m_pTime; ///< ptime parameter - by how much timestamp will be incremented in inbound frame
   unsigned int m_initialPTime;
   unsigned int m_iPLCCounter; ///< how many times PLC frame was used
   unsigned int m_iMaxConcealedFrames; ///< maximum number of successive lost frames to conceal
   JitterBufferStatistics m_statistics; ///< jitter buffer statistics
   RtpTimestamp m_internalClock;
   OsMutex m_mutex;
};

#endif // MpJitterBufferDefault_h__
