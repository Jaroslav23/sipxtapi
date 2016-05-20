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


// APPLICATION INCLUDES
#ifdef __pingtel_on_posix__ /* [ */
#include <sys/types.h>
#include <netinet/in.h>
#endif /* __pingtel_on_posix__ ] */
#include "mp/MpdPtAVT.h"
#include "mp/MpResNotification.h"
#include "os/OsSysLog.h"
#ifdef _VXWORKS /* [ */
#include <inetlib.h>
#endif /* _VXWORKS ] */

struct AvtPacket {
   uint8_t  key;
   uint8_t  dB;
   short    samplesSwapped;
};

static int debugCtr = 0;

const MpCodecInfo MpdPtAVT::smCodecInfo(
         SdpCodec::SDP_CODEC_TONES,// codecType
         "Pingtel_1.0",// codecVersion
         8000,// samplingRate
         0,// numBitsPerSample
         1,// numChannels
         6400,// bitRate. It doesn't matter right now.
         128,// minPacketBits
         128,// maxPacketBits
         0,// numSamplesPerFrame
         0,// requested length of jitter buffer
         TRUE);// signalingCodec

MpdPtAVT::MpdPtAVT(int payloadType)
   : MpDecoderBase(payloadType, &smCodecInfo),
     mCurrentToneKey(-1),
     mPrevToneSignature(0),
     mCurrentToneSignature(0),
     mToneDuration(0)
{
   OsSysLog::add(FAC_MP, PRI_INFO, "MpdPtAVT(0x%X)::MpdPtAVT(%d)\n",
      (int) this, payloadType);
}

MpdPtAVT::~MpdPtAVT()
{
   freeDecode();
}

OsStatus MpdPtAVT::initDecode()
{
   int res = 0;
   debugCtr = 0;

   return (0 == res) ? OS_SUCCESS : OS_UNSPECIFIED;
}

OsStatus MpdPtAVT::freeDecode(void)
{
   return OS_SUCCESS;
}

void dumpRawAvtPacket(const MpRtpBufPtr &pRtp, int pThis)
{
   uint8_t vpxcc;
   uint8_t mpt;
   RtpSeq seq;
   RtpTimestamp timestamp;
   RtpSRC ssrc;

   uint8_t  key;
   uint8_t  dB;
   uint16_t duration;

   AvtPacket *pAvt;

   vpxcc = pRtp->getRtpHeader().vpxcc;
   mpt = pRtp->getRtpHeader().mpt;
   seq = pRtp->getRtpSequenceNumber();
   timestamp = pRtp->getRtpTimestamp();
   ssrc = pRtp->getRtpSSRC();

   pAvt = (AvtPacket*)pRtp->getDataPtr();
   key = pAvt->key;
   dB  = pAvt->dB;
   duration = ntohs(pAvt->samplesSwapped);

   OsSysLog::add(FAC_MP, PRI_INFO,
      " MpdPtAvt(0x%x): Raw packet: %02x %02x %6d %08x %08x %2d %02x %5d\n",
      pThis, vpxcc, mpt, seq, timestamp, ssrc, key, dB, duration);
}

int MpdPtAVT::decode(const MpRtpBufPtr &pPacket,
                      unsigned decodedBufferLength,
                      MpAudioSample *samplesBuffer,
                      UtlBoolean bIsPLCFrame
                     )
{
   const struct AvtPacket* pAvt;
   unsigned int samples;
   unsigned int ts;

   pAvt = (const AvtPacket*) pPacket->getDataPtr();

   // dumpRawAvtPacket(pPacket, (int)this);

   ts = pPacket->getRtpTimestamp();

   if (mCurrentToneKey != -1) { // if previous tone still active
      if (mCurrentToneSignature != ts) { // and we have not seen this
         if (0 != mToneDuration) { // and its duration > 0
            OsSysLog::add(FAC_MP, PRI_INFO,
               "++++ MpdPtAvt(0x%X) SYNTHESIZING KEYUP for old key (%d)"
               " duration=%d ++++\n", (int) this, 
               mCurrentToneKey, mToneDuration);
            signalKeyUp(pPacket);
         }
      }
   }

   // Key Down (start of tone)
   if (pPacket->isRtpMarker() && (mCurrentToneSignature != ts) && (ts != mPrevToneSignature)) {
      // start bit marked
      OsSysLog::add(FAC_MP, PRI_INFO, "++++ MpdPtAvt(0x%X) RECEIVED KEYDOWN"
         " (marker bit set), duration=%d, TSs: old=0x%08x, new=0x%08x,"
         " delta=%d; mCurrentToneKey=%d ++++",
         (int) this, mToneDuration, mPrevToneSignature, ts,
         ts - mPrevToneSignature, mCurrentToneKey);
      signalKeyDown(pPacket);
      samples = pAvt->samplesSwapped;
      mToneDuration = (ntohs(samples) & 0xffff);
   } else if ((mPrevToneSignature != ts) && (-1 == mCurrentToneKey)) {
      // key up interpreted as key down if no previous start tone received
      OsSysLog::add(FAC_MP, PRI_INFO, "++++ MpdPtAvt(0x%X) RECEIVED KEYDOWN"
         " (lost packets?) duration=%d; TSs: old=0x%08x, new=0x%08x,"
         " delta=%d; ++++\n",
         (int) this, mToneDuration, mPrevToneSignature, ts,
         ts - mPrevToneSignature);
      signalKeyDown(pPacket);
      samples = pAvt->samplesSwapped;
      mToneDuration = (ntohs(samples) & 0xffff);
   }
   else
   {
      samples = pAvt->samplesSwapped;
      mToneDuration = (ntohs(samples) & 0xffff);
      if (mToneDuration && (0x80 != (0x80 & (pAvt->dB))))
      {
         OsSysLog::add(FAC_MP, PRI_INFO, "++++ MpdPtAvt(0x%X) RECEIVED packet, not KEYDOWN, set duration to zero"
            " duration=%d; TSs: old=0x%08x, new=0x%08x,"
            " delta=%d; ++++\n",
            (int) this, mToneDuration, mPrevToneSignature, ts,
            ts - mPrevToneSignature);
         mToneDuration = 0;
      }
   }

   // Key Up (end of tone)
   if (0x80 == (0x80 & (pAvt->dB))) {
      OsSysLog::add(FAC_MP, PRI_INFO, "++++ MpdPtAvt(0x%X) RECEIVED KEYUP"
         " duration=%d, TS=0x%08x ++++\n", (int) this, mToneDuration, ts);
      signalKeyUp(pPacket);
   }

   return 0;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void MpdPtAVT::signalKeyDown(const MpRtpBufPtr &pPacket)
{
   const struct AvtPacket* pAvt;
   unsigned int ts;

   pAvt = (const struct AvtPacket*) pPacket->getDataPtr();

   ts = pPacket->getRtpTimestamp();
   if (mCurrentToneSignature != ts)
   {
      // must have missed a KeyUp
      if (mCurrentToneKey != -1)
      {
         OsSysLog::add(FAC_MP, PRI_INFO, "++++ MpdPtAvt(0x%X) SYNTHESIZING KEYUP for old key (%d)"
            " duration=%d ++++\n", (int) this, mCurrentToneKey, mToneDuration);
         signalKeyUp(pPacket);
      }
   }
   OsSysLog::add(FAC_MP, PRI_INFO, "MpdPtAvt(0x%X) Start Rcv Tone key=%d"
      " dB=%d TS=0x%08x\n", (int) this, pAvt->key, pAvt->dB, ts);
   
   notify(MP_RES_DTMF_2833_NOTIFICATION, pAvt->key);

   mCurrentToneKey = pAvt->key;
   mCurrentToneSignature = ts;
   mToneDuration = 0;
}


void MpdPtAVT::signalKeyUp(const MpRtpBufPtr &pPacket)
{
   const struct AvtPacket* pAvt;
   unsigned int samples;
   unsigned int ts;

   pAvt = (const struct AvtPacket*) pPacket->getDataPtr();
   ts = pPacket->getRtpTimestamp();
   samples = pAvt->samplesSwapped;
   samples = ntohs(samples);

   if ((-1) != mCurrentToneKey) {
      OsSysLog::add(FAC_MP, PRI_INFO, "MpdPtAvt(0x%X) Stop Rcv Tone key=%d"
         " dB=%d TS=0x%08x+%d last key=%d\n", (int) this, pAvt->key, pAvt->dB,
         mCurrentToneSignature, mToneDuration, mCurrentToneKey);
      mPrevToneSignature = mCurrentToneSignature;
   }
   mCurrentToneKey = -1;
   mCurrentToneSignature = 0;
   mToneDuration = 0;
}
