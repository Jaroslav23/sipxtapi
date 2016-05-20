//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// Copyright (C) 2006-2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2008-2009 Jaroslav Libak.  All rights reserved.
// Licensed under the LGPL license.
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifdef HAVE_SPAN_DSP /* [ */

// APPLICATION INCLUDES
#include "mp/MpdSipxG726.h"

const MpCodecInfo MpdSipxG726::ms_codecInfo16(
   SdpCodec::SDP_CODEC_G726_16,    // codecType
   "G726-16",                 // codecVersion
   8000,                       // samplingRate
   16,                          // numBitsPerSample
   1,                          // numChannels
   16000,                      // bitRate. It doesn't matter right now.
   40*8,                       // minPacketBits
   40*8,                       // maxPacketBits
   160);                       // numSamplesPerFrame - 20ms frame

const MpCodecInfo MpdSipxG726::ms_codecInfo24(
   SdpCodec::SDP_CODEC_G726_24,    // codecType
   "G726-24",                 // codecVersion
   8000,                       // samplingRate
   16,                          // numBitsPerSample
   1,                          // numChannels
   24000,                      // bitRate. It doesn't matter right now.
   60*8,                       // minPacketBits
   60*8,                       // maxPacketBits
   160);                       // numSamplesPerFrame - 20ms frame

const MpCodecInfo MpdSipxG726::ms_codecInfo32(
   SdpCodec::SDP_CODEC_G726_32,    // codecType
   "G726-32",                 // codecVersion
   8000,                       // samplingRate
   16,                          // numBitsPerSample
   1,                          // numChannels
   32000,                      // bitRate. It doesn't matter right now.
   80*8,                       // minPacketBits
   80*8,                       // maxPacketBits
   160);                       // numSamplesPerFrame - 20ms frame

const MpCodecInfo MpdSipxG726::ms_codecInfo40(
   SdpCodec::SDP_CODEC_G726_40,    // codecType
   "G726-40",                 // codecVersion
   8000,                       // samplingRate
   16,                          // numBitsPerSample
   1,                          // numChannels
   40000,                      // bitRate. It doesn't matter right now.
   100*8,                       // minPacketBits
   100*8,                       // maxPacketBits
   160);                       // numSamplesPerFrame - 20ms frame

MpdSipxG726::MpdSipxG726(int payloadType, G726_BITRATE bitRate)
: MpDecoderBase(payloadType, getCodecInfo(bitRate))
, m_pG726state(NULL)
{
}

MpdSipxG726::~MpdSipxG726()
{
   freeDecode();
}

OsStatus MpdSipxG726::initDecode()
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

OsStatus MpdSipxG726::freeDecode(void)
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

int MpdSipxG726::decode(const MpRtpBufPtr &pPacket,
                        unsigned decodedBufferLength,
                        MpAudioSample *samplesBuffer,
                        UtlBoolean bIsPLCFrame)
{
   if (!pPacket.isValid())
      return 0;

   unsigned payloadSize = pPacket->getPayloadSize();
   unsigned maxPayloadSize = getInfo()->getMaxPacketBits()/8;
   // do not accept frames longer than 20ms from RTP to protect against buffer overflow
   assert(payloadSize <= maxPayloadSize);
   if (payloadSize > maxPayloadSize || payloadSize <= 1)
   {
      return 0;
   }

   int samplesDecoded = g726_decode(m_pG726state, samplesBuffer, (const uint8_t*)pPacket->getDataPtr(), (int)pPacket->getPayloadSize());

   return samplesDecoded;
}

const MpCodecInfo* MpdSipxG726::getCodecInfo(G726_BITRATE bitRate)
{
   const MpCodecInfo* pCodecInfo = &ms_codecInfo40;
   switch(bitRate)
   {
   case MpdSipxG726::BITRATE_16:
      pCodecInfo = &ms_codecInfo16;
      break;
   case MpdSipxG726::BITRATE_24:
      pCodecInfo = &ms_codecInfo24;
      break;
   case MpdSipxG726::BITRATE_32:
      pCodecInfo = &ms_codecInfo32;
      break;
   case MpdSipxG726::BITRATE_40:
      pCodecInfo = &ms_codecInfo40;
      break;
   default:
      ;
   }

   return pCodecInfo;
}

#endif /* HAVE_SPAN_DSP ] */
