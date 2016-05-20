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
#include "mp/MpdIPPG728.h"

extern "C" {
#include "ippcore.h"
#include "ipps.h"
#include "usccodec.h"
}

MpdIPPG728::MpdIPPG728(int payloadType)
: MpDecoderBase(payloadType, getCodecInfo(16000))
{
   m_pCodec = (LoadedCodec*)malloc(sizeof(LoadedCodec));
   memset(m_pCodec, 0, sizeof(LoadedCodec));
}

MpdIPPG728::~MpdIPPG728()
{
   freeDecode();
   free(m_pCodec);
}

OsStatus MpdIPPG728::initDecode()
{
   int lCallResult;

   ippStaticInit();
   strcpy((char*)m_pCodec->codecName, "IPP_G728");

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

OsStatus MpdIPPG728::freeDecode(void)
{
   // Free codec memory
   USCFree(&m_pCodec->uscParams);

   return OS_SUCCESS;
}

int MpdIPPG728::decode(const MpRtpBufPtr &rtpPacket,
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

   configureBitStream(payloadSize);
   if (Bitstream.nbytes % 12 == 0)
   {
      return 0; // decoding 9.6 kbit/s seems to be broken in Intel IPP 6.0 samples. Decoder always returns -4.
   }
   // decode whole 20ms frame

   int frames = 0;
   frames = payloadSize / Bitstream.nbytes;

   // Setup input and output pointers
   Bitstream.pBuffer = const_cast<char*>(rtpPacket->getDataPtr());
   PCMStream.pBuffer = reinterpret_cast<char*>(samplesBuffer);
   // zero the buffer in case we decode less than 320 bytes
   // as it happens sometimes
   memset(PCMStream.pBuffer, 0, 320);

   // Decode frames
   for (int i = 0; i < frames; i++)
   {
      // Decode frame
      USC_Status uscStatus = m_pCodec->uscParams.USC_Fns->Decode(m_pCodec->uscParams.uCodec.hUSCCodec,
         bIsPLCFrame ? NULL : &Bitstream,
         &PCMStream);
      assert(uscStatus == USC_NoError);

      if (uscStatus != USC_NoError)
      {
         return 0;
      }

      // move pointers
      Bitstream.pBuffer += Bitstream.nbytes;
      PCMStream.pBuffer += m_pCodec->uscParams.pInfo->params.framesize;
      decodedSamples += PCMStream.nbytes / sizeof(MpAudioSample);
   }

   // Return number of decoded samples
   return decodedSamples;
}

void MpdIPPG728::configureBitStream(int rtpPayloadBytes)
{
   SdpCodec::SdpCodecTypes codecType = getInfo()->getCodecType();

   if (rtpPayloadBytes % 12 == 0)
   {
      Bitstream.bitrate = 8000;
      Bitstream.nbytes = 12; // in 10ms frames
   }
   else if (rtpPayloadBytes % 16 == 0)
   {
      Bitstream.bitrate = 12800;
      Bitstream.nbytes = 16; // in 10ms frames
   }
   else if (rtpPayloadBytes % 20 == 0)
   {
      // 6400 bps, G.729/D
      Bitstream.bitrate = 16000;
      Bitstream.nbytes = 20; // in 10ms frames
   }
   else
   {
      Bitstream.nbytes = 16; // use default
   }
}

const MpCodecInfo* MpdIPPG728::getCodecInfo(int bitRate)
{
   const MpCodecInfo* pCodecInfo = &smCodecInfo12800;
   switch(bitRate)
   {
   case 9600:
      pCodecInfo = &smCodecInfo9600;
      break;
   case 12800:
      pCodecInfo = &smCodecInfo12800;
      break;
   case 16000:
      pCodecInfo = &smCodecInfo16000;
      break;
   default:
      ;
   }

   return pCodecInfo;
}

const MpCodecInfo MpdIPPG728::smCodecInfo9600(
   SdpCodec::SDP_CODEC_G728,    // codecType
   "Intel IPP 6.0",             // codecVersion
   8000,                        // samplingRate
   16,                          // numBitsPerSample (not used)
   1,                           // numChannels
   9600,                        // bitRate
   24*8,                        // minPacketBits
   24*8,                        // maxPacketBits
   160);                        // numSamplesPerFrame - 20ms frame

const MpCodecInfo MpdIPPG728::smCodecInfo12800(
   SdpCodec::SDP_CODEC_G728,    // codecType
   "Intel IPP 6.0",             // codecVersion
   8000,                        // samplingRate
   16,                          // numBitsPerSample (not used)
   1,                           // numChannels
   12800,                        // bitRate
   32*8,                        // minPacketBits
   32*8,                        // maxPacketBits
   160);                        // numSamplesPerFrame - 20ms frame

const MpCodecInfo MpdIPPG728::smCodecInfo16000(
   SdpCodec::SDP_CODEC_G728,    // codecType
   "Intel IPP 6.0",             // codecVersion
   8000,                        // samplingRate
   16,                          // numBitsPerSample (not used)
   1,                           // numChannels
   16000,                        // bitRate
   40*8,                        // minPacketBits
   40*8,                        // maxPacketBits
   160);                        // numSamplesPerFrame - 20ms frame

#endif /* !HAVE_INTEL_IPP ] */
