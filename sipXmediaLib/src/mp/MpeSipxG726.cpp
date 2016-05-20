//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2008-2009 Jaroslav Libak.  All rights reserved.
// Licensed under the LGPL license.
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifdef HAVE_SPAN_DSP /* [ */

#include "assert.h"
// APPLICATION INCLUDES
#include "mp/MpeSipxG726.h"
#include "mp/NetInTask.h"  

const MpCodecInfo MpeSipxG726::ms_codecInfo16(
   SdpCodec::SDP_CODEC_G726_16,    // codecType
   "G726-16",                 // codecVersion
   8000,                       // samplingRate
   16,                          // numBitsPerSample
   1,                          // numChannels
   16000,                      // bitRate. It doesn't matter right now.
   40*8,                       // minPacketBits
   40*8,                       // maxPacketBits
   160);                       // numSamplesPerFrame - 20 ms frame

const MpCodecInfo MpeSipxG726::ms_codecInfo24(
   SdpCodec::SDP_CODEC_G726_24,    // codecType
   "G726-24",                 // codecVersion
   8000,                       // samplingRate
   16,                          // numBitsPerSample
   1,                          // numChannels
   24000,                      // bitRate. It doesn't matter right now.
   60*8,                       // minPacketBits
   60*8,                       // maxPacketBits
   160);                       // numSamplesPerFrame - 20 ms frame

const MpCodecInfo MpeSipxG726::ms_codecInfo32(
   SdpCodec::SDP_CODEC_G726_32,    // codecType
   "G726-32",                 // codecVersion
   8000,                       // samplingRate
   16,                          // numBitsPerSample
   1,                          // numChannels
   32000,                      // bitRate. It doesn't matter right now.
   80*8,                       // minPacketBits
   80*8,                       // maxPacketBits
   160);                       // numSamplesPerFrame - 20 ms frame

const MpCodecInfo MpeSipxG726::ms_codecInfo40(
   SdpCodec::SDP_CODEC_G726_40,    // codecType
   "G726-40",                 // codecVersion
   8000,                       // samplingRate
   16,                          // numBitsPerSample
   1,                          // numChannels
   40000,                      // bitRate. It doesn't matter right now.
   100*8,                       // minPacketBits
   100*8,                       // maxPacketBits
   160);                       // numSamplesPerFrame - 20 ms frame

MpeSipxG726::MpeSipxG726(int payloadType, G726_BITRATE bitRate)
: MpEncoderBase(payloadType, getCodecInfo(bitRate))
, m_pG726state(NULL)
{
}

MpeSipxG726::~MpeSipxG726()
{
   freeEncode();
}

OsStatus MpeSipxG726::initEncode(void)
{
   m_pG726state = g726_init(NULL, getInfo()->getBitRate(), G726_ENCODING_LINEAR, G726_PACKING_LEFT);

   if (m_pG726state)
   {
      return OS_SUCCESS;
   }
   else
   {
      return OS_FAILED;
   }
}

OsStatus MpeSipxG726::freeEncode(void)
{
   int res = 0;
   
   if (m_pG726state)
   {
      res = g726_release(m_pG726state);
      m_pG726state = NULL;
   }

   if (res == 0)
   {
      return OS_SUCCESS;
   }
   else
   {
      return OS_FAILED;
   }
}

OsStatus MpeSipxG726::encode(const MpAudioSample* pAudioSamples,
                            const int numSamples,
                            int& rSamplesConsumed,
                            unsigned char* pCodeBuf,
                            const int bytesLeft,
                            int& rSizeInBytes,
                            UtlBoolean& sendNow,
                            MpSpeechType& speechType)
{
   assert(numSamples == 80); // we expect 10ms frames

   if (speechType == MP_SPEECH_SILENT && ms_bEnableVAD)
   {
      // VAD must be enabled, do DTX
      rSamplesConsumed = numSamples;
      rSizeInBytes = 0;
      sendNow = TRUE; // sends any unsent frames now
      return OS_SUCCESS;
   }

   rSizeInBytes = g726_encode(m_pG726state, pCodeBuf, pAudioSamples, numSamples);

   rSamplesConsumed = numSamples;

   return OS_SUCCESS;
}

const MpCodecInfo* MpeSipxG726::getCodecInfo(G726_BITRATE bitRate)
{
   const MpCodecInfo* pCodecInfo = &ms_codecInfo40;
   switch(bitRate)
   {
   case MpeSipxG726::BITRATE_16:
      pCodecInfo = &ms_codecInfo16;
      break;
   case MpeSipxG726::BITRATE_24:
      pCodecInfo = &ms_codecInfo24;
      break;
   case MpeSipxG726::BITRATE_32:
      pCodecInfo = &ms_codecInfo32;
      break;
   case MpeSipxG726::BITRATE_40:
      pCodecInfo = &ms_codecInfo40;
      break;
   default:
      ;
   }

   return pCodecInfo;
}

#endif /* HAVE_SPAN_DSP ] */
