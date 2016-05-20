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

// APPLICATION INCLUDES
#include "mp/MpdSipxL16.h"

MpdSipxL16::MpdSipxL16(int payloadType, int samplesPerSec)
: MpDecoderBase(payloadType, getCodecInfo(samplesPerSec))
{
}

MpdSipxL16::~MpdSipxL16()
{
}

OsStatus MpdSipxL16::initDecode()
{
   return OS_SUCCESS;
}

OsStatus MpdSipxL16::freeDecode(void)
{
   return OS_SUCCESS;
}

int MpdSipxL16::decode(const MpRtpBufPtr &pPacket,
                       unsigned decodedBufferLength,
                       MpAudioSample *samplesBuffer,
                       UtlBoolean bIsPLCFrame)
{
   if (!pPacket.isValid() || decodedBufferLength <= 0)
   {
      return 0;
   }

   const MpAudioSample* inputPtr = (const MpAudioSample*)pPacket->getDataPtr();
   int inputPayloadSize = (int)pPacket->getPayloadSize() / sizeof(MpAudioSample); // count of samples in RTP
   int samplesToDecode = min(inputPayloadSize, (int)decodedBufferLength); // don't overflow output buffer

   for (int i = 0; i < samplesToDecode; i++)
   {
      samplesBuffer[i] = ntohs(inputPtr[i]);
   }

   return samplesToDecode;
}

const MpCodecInfo* MpdSipxL16::getCodecInfo(int samplesPerSec)
{
   const MpCodecInfo* pCodecInfo = &ms_codecInfo8000;
   switch(samplesPerSec)
   {
   case 8000:
      pCodecInfo = &ms_codecInfo8000;
      break;
   case 11025:
      pCodecInfo = &ms_codecInfo11025;
      break;
   case 16000:
      pCodecInfo = &ms_codecInfo16000;
      break;
   case 22050:
      pCodecInfo = &ms_codecInfo22050;
      break;
   case 24000:
      pCodecInfo = &ms_codecInfo24000;
      break;
   case 32000:
      pCodecInfo = &ms_codecInfo32000;
      break;
   case 44100:
      pCodecInfo = &ms_codecInfo44100;
      break;
   case 48000:
      pCodecInfo = &ms_codecInfo48000;
      break;
   default:
      ;
   }

   return pCodecInfo;
}

const MpCodecInfo MpdSipxL16::ms_codecInfo8000(
   SdpCodec::SDP_CODEC_L16_8000_MONO,    // codecType
   "",                         // codecVersion
   8000,                       // samplingRate
   16,                          // numBitsPerSample
   1,                          // numChannels
   8000*2*8,                   // bitRate. It doesn't matter right now.
   160*2*8,                       // minPacketBits
   160*2*8,                       // maxPacketBits
   160);                       // numSamplesPerFrame - 20ms frame

const MpCodecInfo MpdSipxL16::ms_codecInfo11025(
   SdpCodec::SDP_CODEC_L16_11025_MONO,    // codecType
   "",                         // codecVersion
   11025,                       // samplingRate
   16,                          // numBitsPerSample
   1,                          // numChannels
   11025*2*8,                   // bitRate. It doesn't matter right now.
   220*2*8,                       // minPacketBits
   222*2*8,                       // maxPacketBits
   220);                       // numSamplesPerFrame - 20ms frame

const MpCodecInfo MpdSipxL16::ms_codecInfo16000(
   SdpCodec::SDP_CODEC_L16_16000_MONO,    // codecType
   "",                         // codecVersion
   16000,                       // samplingRate
   16,                          // numBitsPerSample
   1,                          // numChannels
   16000*2*8,                   // bitRate. It doesn't matter right now.
   320*2*8,                       // minPacketBits
   320*2*8,                       // maxPacketBits
   320);                       // numSamplesPerFrame - 20ms frame

const MpCodecInfo MpdSipxL16::ms_codecInfo22050(
   SdpCodec::SDP_CODEC_L16_22050_MONO,    // codecType
   "",                         // codecVersion
   22050,                       // samplingRate
   16,                          // numBitsPerSample
   1,                          // numChannels
   22050*2*8,                   // bitRate. It doesn't matter right now.
   440*2*8,                       // minPacketBits
   442*2*8,                       // maxPacketBits
   440);                       // numSamplesPerFrame - 20ms frame

const MpCodecInfo MpdSipxL16::ms_codecInfo24000(
   SdpCodec::SDP_CODEC_L16_24000_MONO,    // codecType
   "",                         // codecVersion
   24000,                       // samplingRate
   16,                          // numBitsPerSample
   1,                          // numChannels
   24000*2*8,                   // bitRate. It doesn't matter right now.
   480*2*8,                       // minPacketBits
   480*2*8,                       // maxPacketBits
   480);                       // numSamplesPerFrame - 20ms frame

const MpCodecInfo MpdSipxL16::ms_codecInfo32000(
   SdpCodec::SDP_CODEC_L16_32000_MONO,    // codecType
   "",                         // codecVersion
   32000,                       // samplingRate
   16,                          // numBitsPerSample
   1,                          // numChannels
   32000*2*8,                   // bitRate. It doesn't matter right now.
   640*2*8,                       // minPacketBits
   640*2*8,                       // maxPacketBits
   640);                       // numSamplesPerFrame - 20ms frame

const MpCodecInfo MpdSipxL16::ms_codecInfo44100(
   SdpCodec::SDP_CODEC_L16_44100_MONO,    // codecType
   "",                         // codecVersion
   44100,                       // samplingRate
   16,                          // numBitsPerSample
   1,                          // numChannels
   44100*2*8,                   // bitRate. It doesn't matter right now.
   441*2*8,                       // minPacketBits
   441*2*8,                       // maxPacketBits
   441);                       // numSamplesPerFrame - 10ms frame because 20ms can't fit in RTP

const MpCodecInfo MpdSipxL16::ms_codecInfo48000(
   SdpCodec::SDP_CODEC_L16_48000_MONO,    // codecType
   "",                         // codecVersion
   48000,                       // samplingRate
   16,                          // numBitsPerSample
   1,                          // numChannels
   48000*2*8,                   // bitRate. It doesn't matter right now.
   480*2*8,                       // minPacketBits
   480*2*8,                       // maxPacketBits
   480);                       // numSamplesPerFrame - 10ms frame because 20ms can't fit in RTP
