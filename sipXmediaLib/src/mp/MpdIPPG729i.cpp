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
#include "mp/MpdIPPG729i.h"

extern "C" {
#include "ippcore.h"
#include "ipps.h"
#include "usccodec.h"
}

MpdIPPG729i::MpdIPPG729i(int payloadType, int bitRate)
: MpDecoderBase(payloadType, getCodecInfo(bitRate))
{
   codec = (LoadedCodec*)malloc(sizeof(LoadedCodec));
   memset(codec, 0, sizeof(LoadedCodec));
}

MpdIPPG729i::~MpdIPPG729i()
{
   freeDecode();
   free(codec);
}

OsStatus MpdIPPG729i::initDecode()
{
   int lCallResult;

   ippStaticInit();
   codec->lIsVad = 1;
   strcpy((char*)codec->codecName, "IPP_G729I");

   // Load codec by name
   lCallResult = LoadUSCCodecByName(codec, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   // Get USC codec params
   lCallResult = USCCodecAllocInfo(&codec->uscParams, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   lCallResult = USCCodecGetInfo(&codec->uscParams, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   // Get its supported format details
   lCallResult = GetUSCCodecParamsByFormat(codec, BY_NAME, NULL);
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
   lCallResult = SetUSCDecoderPCMType(&codec->uscParams, -1, &streamType, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   // instead of SetUSCDecoderParams(...)
   codec->uscParams.pInfo->params.direction = USC_DECODE;
   codec->uscParams.pInfo->params.law = 0;
   codec->uscParams.pInfo->params.modes.vad =1;

   // Prepare input buffer parameters
   configureBitStream(0); // configure for 6400/11800 by default

   // Alloc memory for the codec
   lCallResult = USCCodecAlloc(&codec->uscParams, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   // Init decoder
   lCallResult = USCDecoderInit(&codec->uscParams, NULL, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED; 
   }

   return OS_SUCCESS;
}

OsStatus MpdIPPG729i::freeDecode(void)
{
   // Free codec memory
   USCFree(&codec->uscParams);

   return OS_SUCCESS;
}

int MpdIPPG729i::decode(const MpRtpBufPtr &rtpPacket,
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

   if (payloadSize <= 2 && !bIsPLCFrame)
   {
      // MpDecodeBuffer will generate comfort noise
      return 0;
   }
   else
   {
      configureBitStream(payloadSize); // doesn't process SID frames, those are handled above
      // decode in 10ms frames
      int frames = 0;
      if (!bIsPLCFrame)
      {
         frames = payloadSize / Bitstream.nbytes;
      }
      else
      {
         // codec will do PLC
         frames = 2;
      }

      // Setup input and output pointers
      Bitstream.pBuffer = const_cast<char*>(rtpPacket->getDataPtr());
      PCMStream.pBuffer = reinterpret_cast<char*>(samplesBuffer);
      // zero the buffer in case we decode less than 320 bytes
      // as it happens sometimes
      memset(PCMStream.pBuffer, 0, 320);

      // Decode frames
      for (int i = 0; i < frames; i++)
      {
         // Decode one frame
         USC_Status uscStatus = codec->uscParams.USC_Fns->Decode(codec->uscParams.uCodec.hUSCCodec,
            bIsPLCFrame ? NULL : &Bitstream,
            &PCMStream);
         assert(uscStatus == USC_NoError);

         if (uscStatus != USC_NoError)
         {
            return 0;
         }

         // move pointers
         Bitstream.pBuffer += Bitstream.nbytes;
         PCMStream.pBuffer += codec->uscParams.pInfo->params.framesize;
         decodedSamples += PCMStream.nbytes / sizeof(MpAudioSample);
      }
   }

   // Return number of decoded samples
   return decodedSamples;
}

void MpdIPPG729i::configureBitStream(int rtpPayloadBytes)
{
   SdpCodec::SdpCodecTypes codecType = getInfo()->getCodecType();

   if (codecType == SdpCodec::SDP_CODEC_G729D)
   {
      if (rtpPayloadBytes % 10 == 0)
      {
         // 8000 bps, G.729/D
         Bitstream.bitrate = 8000;
         Bitstream.frametype = 3;
         Bitstream.nbytes = 10; // in 10ms frames
      }
      else // unknown, assume 6400
      {
         // 6400 bps, G.729/D
         Bitstream.bitrate = 6400;
         Bitstream.frametype = 2;
         Bitstream.nbytes = 8; // in 10ms frames
      }
   }
   else if (codecType == SdpCodec::SDP_CODEC_G729E)
   {
      // 11800 bps, G.729/E
      Bitstream.bitrate = 11800;
      Bitstream.frametype = 4;
      Bitstream.nbytes = 15; // in 10ms frames
   }
}

const MpCodecInfo* MpdIPPG729i::getCodecInfo(int bitRate)
{
   const MpCodecInfo* pCodecInfo = &smCodecInfo6400;
   switch(bitRate)
   {
   case 6400:
      pCodecInfo = &smCodecInfo6400;
      break;
   case 8000:
      pCodecInfo = &smCodecInfo8000;
      break;
   case 11800:
      pCodecInfo = &smCodecInfo11800;
      break;
   default:
      ;
   }

   return pCodecInfo;
}

const MpCodecInfo MpdIPPG729i::smCodecInfo6400(
   SdpCodec::SDP_CODEC_G729D,    // codecType
   "Intel IPP 6.0",             // codecVersion
   8000,                        // samplingRate
   16,                          // numBitsPerSample (not used)
   1,                           // numChannels
   6400,                        // bitRate
   16*8,                        // minPacketBits
   16*8,                        // maxPacketBits
   160);                        // numSamplesPerFrame - 20ms frame

const MpCodecInfo MpdIPPG729i::smCodecInfo8000(
   SdpCodec::SDP_CODEC_G729D,    // codecType
   "Intel IPP 6.0",             // codecVersion
   8000,                        // samplingRate
   16,                          // numBitsPerSample (not used)
   1,                           // numChannels
   8000,                        // bitRate
   20*8,                        // minPacketBits
   20*8,                        // maxPacketBits
   160);                        // numSamplesPerFrame - 20ms frame

const MpCodecInfo MpdIPPG729i::smCodecInfo11800(
   SdpCodec::SDP_CODEC_G729E,    // codecType
   "Intel IPP 6.0",             // codecVersion
   8000,                        // samplingRate
   16,                          // numBitsPerSample (not used)
   1,                           // numChannels
   11800,                        // bitRate
   29*8,                        // minPacketBits
   30*8,                        // maxPacketBits
   160);                        // numSamplesPerFrame - 20ms frame

#endif /* !HAVE_INTEL_IPP ] */
