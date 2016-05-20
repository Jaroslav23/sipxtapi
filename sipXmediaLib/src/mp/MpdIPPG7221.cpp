//
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2008-2009 Jaroslav Libak.  All rights reserved.
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_INTEL_IPP /* [ */

#ifdef WIN32 // [
#   pragma comment(lib, "ipps.lib")
#   pragma comment(lib, "ippsc.lib")
#   pragma comment(lib, "ippcore.lib")
#   pragma comment(lib, "ippsr.lib")
#   pragma comment(lib, "libircmt.lib")
#endif // WIN32 ]

// APPLICATION INCLUDES
#include "mp/MpdIPPG7221.h"

extern "C" {
#include "ippcore.h"
#include "ipps.h"
#include "usccodec.h"
}

MpdIPPG7221::MpdIPPG7221(int payloadType, int bitrate)
: MpDecoderBase(payloadType, getCodecInfo(bitrate))
{
   m_pCodec = (LoadedCodec*)malloc(sizeof(LoadedCodec));
   memset(m_pCodec, 0, sizeof(LoadedCodec));
}

MpdIPPG7221::~MpdIPPG7221()
{
   freeDecode();
   free(m_pCodec);
}

OsStatus MpdIPPG7221::initDecode()
{
   int lCallResult;

   ippStaticInit();
   strcpy((char*)m_pCodec->codecName, "IPP_G722.1");

   // Load codec by name
   lCallResult = LoadUSCCodecByName(m_pCodec, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   // Get USC codec params
   lCallResult = USCCodecAllocInfo(&m_pCodec->uscParams, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   lCallResult = USCCodecGetInfo(&m_pCodec->uscParams, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   // Get its supported format details
   lCallResult = GetUSCCodecParamsByFormat(m_pCodec, BY_NAME, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   // Set params for decode
   USC_PCMType streamType;
   streamType.bitPerSample = getInfo()->getNumBitsPerSample();
   streamType.nChannels = getInfo()->getNumChannels();
   streamType.sample_frequency = getInfo()->getSamplingRate();

   // decoder doesn't need to know PCM type
   lCallResult = SetUSCDecoderPCMType(&m_pCodec->uscParams, -1, &streamType, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   // instead of SetUSCDecoderParams(...)
   m_pCodec->uscParams.pInfo->params.direction = USC_DECODE;
   m_pCodec->uscParams.pInfo->params.law = 0;
   m_pCodec->uscParams.pInfo->params.modes.bitrate = getInfo()->getBitRate();
   m_pCodec->uscParams.pInfo->params.modes.vad = 0;

   // Prepare input buffer parameters
   Bitstream.frametype = 0;

   // Alloc memory for the codec
   lCallResult = USCCodecAlloc(&m_pCodec->uscParams, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   // Init decoder
   lCallResult = USCDecoderInit(&m_pCodec->uscParams, NULL, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED; 
   }

   return OS_SUCCESS;
}

OsStatus MpdIPPG7221::freeDecode(void)
{
   // Free codec memory
   USCFree(&m_pCodec->uscParams);

   return OS_SUCCESS;
}

int MpdIPPG7221::decode(const MpRtpBufPtr &rtpPacket,
                       unsigned decodedBufferLength,
                       MpAudioSample *samplesBuffer,
                       UtlBoolean bIsPLCFrame) 
{
   if (!rtpPacket.isValid())
      return 0;

   unsigned payloadSize = rtpPacket->getPayloadSize();
   unsigned maxPayloadSize = getInfo()->getMaxPacketBits()/8;

   assert(payloadSize <= maxPayloadSize);
   if (payloadSize > maxPayloadSize || payloadSize <= 1)
   {
      return 0;
   }

   unsigned int decodedSamples = 0;

   configureBitStream();
   // decode whole 20ms frame

   // Setup input and output pointers
   Bitstream.pBuffer = const_cast<char*>(rtpPacket->getDataPtr());
   PCMStream.pBuffer = reinterpret_cast<char*>(samplesBuffer);
   // zero the buffer in case we decode less than 640 bytes
   memset(PCMStream.pBuffer, 0, 640); // 20ms 16bit 16Khz = 640 bytes

   // Decode frame
   USC_Status uscStatus = m_pCodec->uscParams.USC_Fns->Decode(m_pCodec->uscParams.uCodec.hUSCCodec,
      &Bitstream, &PCMStream); // G.722.1 has no internal PLC support
   assert(uscStatus == USC_NoError);

   if (uscStatus != USC_NoError)
   {
      return 0;
   }

   decodedSamples = PCMStream.nbytes / sizeof(MpAudioSample);

   // Return number of decoded samples
   return decodedSamples;
}

void MpdIPPG7221::configureBitStream()
{
   Bitstream.bitrate = getInfo()->getBitRate();
   Bitstream.nbytes = getInfo()->getMaxPacketBits() / 8; // for 20ms frames
}

const MpCodecInfo* MpdIPPG7221::getCodecInfo(int bitrate)
{
   const MpCodecInfo* pCodecInfo = &ms_codecInfo32000;
   switch(bitrate)
   {
   case 16000:
      pCodecInfo = &ms_codecInfo16000;
      break;
   case 24000:
      pCodecInfo = &ms_codecInfo24000;
      break;
   case 32000:
      pCodecInfo = &ms_codecInfo32000;
      break;
   default:
      ;
   }

   return pCodecInfo;
}

const MpCodecInfo MpdIPPG7221::ms_codecInfo16000(
   SdpCodec::SDP_CODEC_G7221_16,    // codecType
   "Intel IPP 6.0",             // codecVersion
   16000,                        // samplingRate
   16,                          // numBitsPerSample (not used)
   1,                           // numChannels
   16000,                        // bitRate
   40*8,                        // minPacketBits
   40*8,                        // maxPacketBits
   320);                        // numSamplesPerFrame - 20ms frame

const MpCodecInfo MpdIPPG7221::ms_codecInfo24000(
   SdpCodec::SDP_CODEC_G7221_24,    // codecType
   "Intel IPP 6.0",             // codecVersion
   16000,                        // samplingRate
   16,                          // numBitsPerSample (not used)
   1,                           // numChannels
   24000,                        // bitRate
   60*8,                        // minPacketBits
   60*8,                        // maxPacketBits
   320);                        // numSamplesPerFrame - 20ms frame

const MpCodecInfo MpdIPPG7221::ms_codecInfo32000(
   SdpCodec::SDP_CODEC_G7221_32,    // codecType
   "Intel IPP 6.0",             // codecVersion
   16000,                        // samplingRate
   16,                          // numBitsPerSample (not used)
   1,                           // numChannels
   32000,                        // bitRate
   80*8,                        // minPacketBits
   80*8,                        // maxPacketBits
   320);                        // numSamplesPerFrame - 20ms frame

#endif /* !HAVE_INTEL_IPP ] */
