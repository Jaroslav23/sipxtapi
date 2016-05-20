//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>


#ifdef __pingtel_on_posix__
#include <sys/types.h>
#include <netinet/in.h>
#endif

#ifdef WIN32 /* [ */
#include <winsock2.h>
#endif /* WIN32 ] */

#include <string.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include <mp/MpTypes.h>
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MprEncode.h"
#include "mp/MprToNet.h"
#include "mp/MpEncoderBase.h"
#include "mp/MpMediaTask.h"
#include "mp/MpCodecFactory.h"
#include <mp/MpResamplerFactory.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// At 10 ms each, 10 seconds.  We will send an RTP packet to each active
// destination at least this often, even when muted.
const int MprEncode::RTP_KEEP_ALIVE_FRAME_INTERVAL = 1000;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprEncode::MprEncode(const UtlString& rName,
                     int samplesPerFrame, int samplesPerSec)
: MpAudioResource(rName, 1, 1, 0, 0, samplesPerFrame, samplesPerSec)
, mpPrimaryCodec(NULL)
, mpPacket1Payload(NULL)
, mPacket1PayloadBytes(0)
, mPayloadBytesUsed(0)
, mSamplesPacked(0)
, mActiveAudio1(FALSE)
, mMarkNext1(FALSE)
, mConsecutiveInactive1(0)
, mConsecutiveActive1(0)
, mConsecutiveUnsentFrames1(0)
, mDoesVad1(FALSE)
, mDisableDTX(TRUE)
, mpDtmfCodec(NULL)
, mpPacket2Payload(NULL)
, mPacket2PayloadBytes(0)
, mCurrentTone(-1)
, mNumToneStops(-1)
, mTotalTime(0)
, mNewTone(0)
, mCurrentTimestamp(0)
, mMaxPacketTime(20)
, mMaxPacketSamples(0)
, mpToNet(NULL)
, mTimestampStep(samplesPerFrame)
, m_pResampler(NULL)
{
   memset(m_tmpBuffer, 0, sizeof(m_tmpBuffer));
}

// Destructor
MprEncode::~MprEncode()
{
   if (NULL != mpPacket1Payload) {
      delete[] mpPacket1Payload;
      mpPacket1Payload = NULL;
   }
   if (NULL != mpPacket2Payload) {
      delete[] mpPacket2Payload;
      mpPacket2Payload = NULL;
   }
   if (NULL != mpPrimaryCodec) {
      delete mpPrimaryCodec;
      mpPrimaryCodec = NULL;
   }
   if (NULL != mpDtmfCodec) {
      delete mpDtmfCodec;
      mpDtmfCodec = NULL;
   }
   mpToNet = NULL;

   destroyResampler();
}

/* ============================ MANIPULATORS ============================== */

void MprEncode::setMyToNet(MprToNet* myToNet)
{
   mpToNet = myToNet;
}

OsStatus MprEncode::startTone(int toneId)
{
   MpFlowGraphMsg msg(START_TONE, this, NULL, NULL, toneId, 0);
   return postMessage(msg);
}

OsStatus MprEncode::stopTone(void)
{
   MpFlowGraphMsg msg(STOP_TONE, this, NULL, NULL, 0, 0);
   return postMessage(msg);
}

OsStatus MprEncode::enableDTX(UtlBoolean dtx)
{
   MpFlowGraphMsg msg(ENABLE_DTX, this, NULL, NULL, dtx, 0);
   return postMessage(msg);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

OsStatus MprEncode::deselectCodecs(void)
{
   MpFlowGraphMsg msg(DESELECT_CODECS, this, NULL, NULL, 0, 0);

   return postMessage(msg);
}

OsStatus MprEncode::selectCodecs(SdpCodec* pPrimary, SdpCodec* pDtmf)
{
   OsStatus res = OS_SUCCESS;
   MpFlowGraphMsg msg(SELECT_CODECS, this, NULL, NULL, 2, 0);
   SdpCodec** newCodecs;

   newCodecs = new SdpCodec*[2];
   newCodecs[0] = newCodecs[1] = NULL;
   if (NULL != pPrimary)   newCodecs[0] = new SdpCodec(*pPrimary);
   if (NULL != pDtmf)      newCodecs[1] = new SdpCodec(*pDtmf);
   msg.setPtr1(newCodecs);
   res = postMessage(msg);

   return res;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

OsStatus MprEncode::allocPacketBuffer(MpEncoderBase& rEncoder,
                                      unsigned char*& rpPacketPayload,
                                      int& rPacketPayloadBytes)
{
   OsStatus ret = OS_SUCCESS;

   // Set packet size to maximum possible size. Probably we could guess better,
   // but to do so we need:
   // 1) to know how much audio data we want to pack (10ms, 20ms, or more);
   // 2) somehow negotiate packet size for codecs that require special
   //    processing to pack several frames into one packet (like AMR, Speex, etc).
   // One of possible solution would be to implement function to ask for
   // packet size (not frame size!) for codecs. That is pass to it number of
   // audio samples we want to pack and get packet size back from it, like this:
   //    int get_packet_size(int numSamples, int* packetSize) const;
   rPacketPayloadBytes = RTP_MTU;

   // Allocate buffer for RTP packet data
   rpPacketPayload = new unsigned char[rPacketPayloadBytes];
   if (rpPacketPayload == NULL )
   {
      // No free memory. Return error.
      ret = OS_NO_MEMORY;
      rpPacketPayload = NULL;
   }

   return ret;
}

void MprEncode::handleDeselectCodecs(void)
{
   if (mpPrimaryCodec) {
      delete mpPrimaryCodec;
      mpPrimaryCodec = NULL;
      if (NULL != mpPacket1Payload)
      {
         delete[] mpPacket1Payload;
         mpPacket1Payload = NULL;
         mPacket1PayloadBytes = 0;
         mPayloadBytesUsed = 0;
         mSamplesPacked = 0;
      }
      destroyResampler();
   }
   if (NULL != mpDtmfCodec) {
      delete mpDtmfCodec;
      mpDtmfCodec = NULL;
      if (NULL != mpPacket2Payload) {
         delete[] mpPacket2Payload;
         mpPacket2Payload = NULL;
         mPacket2PayloadBytes = 0;
      }
   }
}

void MprEncode::handleSelectCodecs(MpFlowGraphMsg& rMsg)
{
   SdpCodec** newCodecs;
   SdpCodec* pPrimary;
   SdpCodec* pDtmf;
   MpEncoderBase* pNewEncoder;
   MpCodecFactory* pFactory = MpCodecFactory::getMpCodecFactory();
   SdpCodec::SdpCodecTypes ourCodec;
   OsStatus ret;
   int payload;

   newCodecs = (SdpCodec**) rMsg.getPtr1();
   pPrimary = newCodecs[0];
   pDtmf = newCodecs[1];

   handleDeselectCodecs();  // cleanup the old ones, if any

   if (OsSysLog::willLog(FAC_MP, PRI_DEBUG))
   {
      if (NULL != pPrimary) {
         OsSysLog::add(FAC_MP, PRI_DEBUG,
                       "MprEncode::handleSelectCodecs "
                       "pPrimary->getCodecType() = %d, "
                       "pPrimary->getCodecPayloadFormat() = %d",
                       pPrimary->getCodecType(),
                       pPrimary->getCodecPayloadId());
      } else {
         OsSysLog::add(FAC_MP, PRI_DEBUG,
                       "MprEncode::handleSelectCodecs "
                       "pPrimary == NULL");
      }
      if (NULL != pDtmf) {
         OsSysLog::add(FAC_MP, PRI_DEBUG,
                       "MprEncode::handleSelectCodecs "
                       "pDtmf->getCodecType() = %d, "
                       "pDtmf->getCodecPayloadFormat() = %d",
                       pDtmf->getCodecType(),
                       pDtmf->getCodecPayloadId());
      }
   }

   if (pPrimary)
   {
      ourCodec = pPrimary->getCodecType();
      payload = pPrimary->getCodecPayloadId();
      ret = pFactory->createEncoder(*pPrimary, pNewEncoder);
      assert(OS_SUCCESS == ret);
      assert(NULL != pNewEncoder);
      pNewEncoder->initEncode();
      mpPrimaryCodec = pNewEncoder;
      mDoesVad1 = (pNewEncoder->getInfo())->doesVadCng();
      allocPacketBuffer(*mpPrimaryCodec, mpPacket1Payload, mPacket1PayloadBytes);
      mPayloadBytesUsed = 0;
      mSamplesPacked = 0;
      // adjust timestamp step by sampling rate difference of flowgraph and primary codec
      // when codec uses 8000, but flowgraph 16000, then samples we get will be downsampled to 1/2 of samples
      // and thus timestamp must be advanced by lower value
      unsigned int encoderSamplingRate = pNewEncoder->getInfo()->getSamplingRate();
      unsigned int flowgraphSamplingRate = (unsigned int)getSamplesPerSec();
      mTimestampStep = (unsigned int)(((double)encoderSamplingRate / flowgraphSamplingRate) * getSamplesPerFrame());
      mMaxPacketTime = pPrimary->getPacketLength() / 1000; // update RTP payload size in ms
      mMaxPacketSamples = mMaxPacketTime * encoderSamplingRate / 1000;
      if (pNewEncoder->getInfo()->getCodecType() == SdpCodec::SDP_CODEC_G722)
      {
         // Workaround RFC bug with G.722 samplerate.
         // Read RFC 3551 Section 4.5.2 "G722" for details.
         mTimestampStep /= 2;
      }
      destroyResampler();
      if (encoderSamplingRate != flowgraphSamplingRate)
      {
         // setup resampler
         m_pResampler = MpResamplerFactory::createResampler(flowgraphSamplingRate, encoderSamplingRate);
      }
   }

   if (pDtmf)
   {
      ret = pFactory->createEncoder(*pDtmf, pNewEncoder);
      assert(OS_SUCCESS == ret);
      assert(NULL != pNewEncoder);
      pNewEncoder->initEncode();
      mpDtmfCodec = pNewEncoder;
      allocPacketBuffer(*mpDtmfCodec, mpPacket2Payload, mPacket2PayloadBytes);
   }

   // delete any SdpCodec objects that we did not keep pointers to.
   if (NULL != pPrimary)   delete pPrimary;
   if (NULL != pDtmf)      delete pDtmf;

   // free the array we were sent
   delete[] newCodecs;
}

void MprEncode::handleStartTone(int toneId)
{
   if (NULL == mpDtmfCodec) return;
   if ((mCurrentTone == -1) && (mNumToneStops < 1)) {
      mCurrentTone = lookupTone(toneId);
      if (mCurrentTone != -1) {
         mNewTone = 1;
      }
   }
}

void MprEncode::handleStopTone(void)
{
   if ((mCurrentTone > -1) && (mNumToneStops < 1)) {
      mNumToneStops = TONE_STOP_PACKETS; // send TONE_STOP_PACKETS end packets
   }
}

void MprEncode::handleEnableDTX(UtlBoolean dtx)
{
   mDisableDTX = !dtx;
}

// Handle messages for this resource.
UtlBoolean MprEncode::handleMessage(MpFlowGraphMsg& rMsg)
{
   if (rMsg.getMsg() == SELECT_CODECS)
   {
      handleSelectCodecs(rMsg);
      return TRUE;
   } else if (rMsg.getMsg() == DESELECT_CODECS) {
      handleDeselectCodecs();
      return TRUE;
   } else if (rMsg.getMsg() == START_TONE) {
      handleStartTone(rMsg.getInt1());
      return TRUE;
   } else if (rMsg.getMsg() == STOP_TONE) {
      handleStopTone();
      return TRUE;
   } else if (rMsg.getMsg() == ENABLE_DTX) {
      handleEnableDTX(rMsg.getInt1());
      return TRUE;
   }
   else
      return MpAudioResource::handleMessage(rMsg);
}

// Translate our tone ID into RFC2833 values.
int MprEncode::lookupTone(int toneId)
{
   int ret = -1;

   switch (toneId) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
         ret = toneId - '0';
         break;
      case  0 :
      case  1 :
      case  2 :
      case  3 :
      case  4 :
      case  5 :
      case  6 :
      case  7 :
      case  8 :
      case  9 :
      case  10 :
      case  11 :
      case  12 :
      case  13 :
      case  14 :
      case  15 :
      case  16 :
         ret = toneId;
         break;
      case 'a': case 'A':
      case 'b': case 'B':
      case 'c': case 'C':
      case 'd': case 'D':
         ret = ((toneId | ('a' ^ 'A')) - ('a' | ('a' ^ 'A'))) + 12;
         break;
      case '*':
         ret = 10;
         break;
      case '#':
         ret = 11;
         break;
   }
   return ret;
}

void MprEncode::doPrimaryCodec(MpAudioBufPtr in, unsigned int startTs)
{
   int numSamplesIn;
   int numSamplesOut;
   int payloadBytesLeft;
   unsigned char* pDest;
   int bytesAdded; //$$$
   MpSpeechType speechType;
   OsStatus ret;
   UtlBoolean isPacketReady = FALSE;

   if (mpPrimaryCodec == NULL)
      return;

   if (!in.isValid())
      return;

   const MpAudioSample* pSamplesIn = NULL;

   if (mpPrimaryCodec->getInfo()->getSamplingRate() != getSamplesPerSec() && m_pResampler)
   {
      uint32_t inResampleSamplesCount = in->getSamplesNumber();
      uint32_t outResampleSamplesCount = SAMPLES_PER_FRAME;
      // we need to resample
      OsStatus res = m_pResampler->resample(in->getSamplesPtr(), inResampleSamplesCount, inResampleSamplesCount,
         m_tmpBuffer, outResampleSamplesCount, outResampleSamplesCount);
      assert(res == OS_SUCCESS);
      numSamplesIn = outResampleSamplesCount;
      pSamplesIn = m_tmpBuffer;
   }
   else
   {
      // no need to resample
      numSamplesIn = in->getSamplesNumber();
      pSamplesIn = in->getSamplesPtr();
   }

   while (numSamplesIn > 0)
   {
      if (mPayloadBytesUsed == 0)
      {
         mStartTimestamp1 = startTs;
         mActiveAudio1 = mDoesVad1 || mDisableDTX;
      }

      speechType = in->getSpeechType();
      mActiveAudio1 = mActiveAudio1 || isActiveAudio(speechType);

      payloadBytesLeft = mPacket1PayloadBytes - mPayloadBytesUsed;

      pDest = mpPacket1Payload + mPayloadBytesUsed;

      bytesAdded = 0;
      ret = mpPrimaryCodec->encode(pSamplesIn, numSamplesIn, numSamplesOut,
                                   pDest, payloadBytesLeft, bytesAdded,
                                   isPacketReady, speechType);
      mPayloadBytesUsed += bytesAdded;
      assert (mPacket1PayloadBytes >= mPayloadBytesUsed);

      // In case the encoder does silence suppression (e.g. G.729 Annex B)
      mMarkNext1 = mMarkNext1 | (0 == bytesAdded);

      mSamplesPacked += numSamplesOut;
      pSamplesIn += numSamplesOut;
      numSamplesIn -= numSamplesOut;
      startTs += numSamplesOut;

      if (speechType == MP_SPEECH_ACTIVE)
      {
         mActiveAudio1 = TRUE;
      }

      if ((mPayloadBytesUsed > 0) &&
         (isPacketReady || mSamplesPacked  >= mMaxPacketSamples))
      {
         if (mActiveAudio1)
         {
            mConsecutiveInactive1 = 0;
         } else {
            mConsecutiveInactive1++;
         }
         if ((mConsecutiveInactive1 < HANGOVER_PACKETS) ||
             (mConsecutiveUnsentFrames1 >= RTP_KEEP_ALIVE_FRAME_INTERVAL))
         {
            mpToNet->writeRtp(mpPrimaryCodec->getPayloadType(),
                              mMarkNext1,
                              mpPacket1Payload,
                              mPayloadBytesUsed,
                              mStartTimestamp1,
                              NULL);
            mMarkNext1 = FALSE;
            mConsecutiveUnsentFrames1 = 0;
         } else {
            mMarkNext1 = TRUE;
         }
         mPayloadBytesUsed = 0;
         mSamplesPacked = 0;
      }
   }
}

void MprEncode::doDtmfCodec(unsigned int startTs, int samplesPerFrame,
                            int samplesPerSecond)
{
   int numSampleTimes;
#ifdef DEBUG_DTMF_SEND /* [ */
   int skipped;
#endif /* DEBUG_DTMF_SEND ] */

   if (mCurrentTone == -1)
      return;

   if (mpDtmfCodec == NULL)
      return;

   if (mNewTone) {
      mStartTimestamp2 = startTs;
      mDtmfSampleInterval = samplesPerFrame * 2;
      mNumToneStops = -1;
   }

   if (TONE_STOP_PACKETS == mNumToneStops) {
      mTotalTime = startTs - mStartTimestamp2;
   }

   if (mNumToneStops-- < 0) {
      if (mNewTone ||
          ((mLastDtmfSendTimestamp + mDtmfSampleInterval) <= startTs)) {

         numSampleTimes = (startTs + samplesPerFrame) - mStartTimestamp2;
         if (numSampleTimes > ((1<<16) - 1)) numSampleTimes = ((1<<16) - 1);

         mpPacket2Payload[0] = mCurrentTone;
         mpPacket2Payload[1] = 10; // -10 dBm0
         mpPacket2Payload[2] = (numSampleTimes >> 8) & 0xff; // Big Endian
         mpPacket2Payload[3] = numSampleTimes & 0xff; // Big Endian
         mpToNet->writeRtp(mpDtmfCodec->getPayloadType(),
                           (0 != mNewTone),  // set marker on first packet
                           mpPacket2Payload,
                           4,
                           mStartTimestamp2,
                           NULL);
         mLastDtmfSendTimestamp = startTs;
         mNewTone = 0;
#ifdef DEBUG_DTMF_SEND /* [ */
         skipped = 0;
      } else {
         skipped = 1;
      }
      if (mNumToneStops > -20) {
         osPrintf("doDtmfCodec: %d + %d = %d, %d -- %s\n",
            mLastDtmfSendTimestamp, mDtmfSampleInterval,
            (mLastDtmfSendTimestamp + mDtmfSampleInterval),
            startTs, (skipped ? "skipped" : "sent"));
#endif /* DEBUG_DTMF_SEND ] */
      }
   } else {
      numSampleTimes = mTotalTime;
      if (numSampleTimes > ((1<<16) - 1)) numSampleTimes = ((1<<16) - 1);

      mpPacket2Payload[0] = mCurrentTone;
      mpPacket2Payload[1] = (1<<7) + 10; // -10 dBm0, with E bit
      mpPacket2Payload[2] = (numSampleTimes >> 8) & 0xff; // Big Endian
      mpPacket2Payload[3] = numSampleTimes & 0xff; // Big Endian
      mpToNet->writeRtp(mpDtmfCodec->getPayloadType(),
                        FALSE,
                        mpPacket2Payload,
                        4,
                        mStartTimestamp2,
                        NULL);
      mLastDtmfSendTimestamp = startTs;
      if (mNumToneStops < 1) { // all done, ready to start next tone.
         mCurrentTone = -1;
         mNumToneStops = -1;
         mTotalTime = 0;
      }
   }
}

UtlBoolean MprEncode::doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame,
                                     int samplesPerSecond)
{
   MpBufPtr in;

   mConsecutiveUnsentFrames1++;

   if (inBufsSize == 0)
      return FALSE;

   if (!isEnabled)
      return TRUE;

   in = inBufs[0];

   mCurrentTimestamp += mTimestampStep;

   if (NULL != mpPrimaryCodec) {
      doPrimaryCodec(in, mCurrentTimestamp);
   }

   if (NULL != mpDtmfCodec) {
      doDtmfCodec(mCurrentTimestamp, samplesPerFrame, samplesPerSecond);
   }

   // mLastTimestamp = startTs;  // Unused?

   return TRUE;
}

void MprEncode::destroyResampler()
{
   delete m_pResampler;
   m_pResampler = NULL;
}
