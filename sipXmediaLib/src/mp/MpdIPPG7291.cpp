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
#include "mp/MpdIPPG7291.h"

extern "C" {
#include "ippcore.h"
#include "ipps.h"
#include "usccodec.h"
}

MpdIPPG7291::MpdIPPG7291(int payloadType)
: MpDecoderBase(payloadType, getCodecInfo())
{
   codec = (LoadedCodec*)malloc(sizeof(LoadedCodec));
   memset(codec, 0, sizeof(LoadedCodec));
}

MpdIPPG7291::~MpdIPPG7291()
{
   freeDecode();
   free(codec);
}

OsStatus MpdIPPG7291::initDecode()
{
   int lCallResult;

   ippStaticInit();
   codec->lIsVad = 0; // built in VAD is not yet supported by Intel IPP
   strcpy((char*)codec->codecName, "IPP_G729.1");

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
   codec->uscParams.pInfo->params.modes.vad = 0; // built in VAD is not yet supported by Intel IPP

   // Prepare input buffer parameters
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

OsStatus MpdIPPG7291::freeDecode(void)
{
   // Free codec memory
   USCFree(&codec->uscParams);

   return OS_SUCCESS;
}

int MpdIPPG7291::decode(const MpRtpBufPtr &rtpPacket,
                       unsigned decodedBufferLength,
                       MpAudioSample *samplesBuffer,
                       UtlBoolean bIsPLCFrame) 
{
   if (!rtpPacket.isValid())
      return 0;

   unsigned payloadSize = rtpPacket->getPayloadSize();
   unsigned maxPayloadSize = getInfo()->getMaxPacketBits()/8;

   assert(payloadSize <= maxPayloadSize + 1);
   if (payloadSize > (maxPayloadSize+1) || payloadSize <= 2)
   {
      return 0;
   }

   unsigned int decodedSamples = 0;

   if (payloadSize <= 6 && !bIsPLCFrame)
   {
      // MpDecodeBuffer will generate comfort noise
      return 0;
   }
   else
   {
      configureBitStream(payloadSize-1, (char*)(rtpPacket->getDataPtr()));
      // Setup input and output pointers
      Bitstream.pBuffer = const_cast<char*>(rtpPacket->getDataPtr()) + 1; // skip 1 byte header
      PCMStream.pBuffer = reinterpret_cast<char*>(samplesBuffer);
      // zero the buffer in case we decode less than 640 bytes
      // as it happens sometimes
      memset(PCMStream.pBuffer, 0, 640);

      // Decode one frame
      USC_Status uscStatus = codec->uscParams.USC_Fns->Decode(codec->uscParams.uCodec.hUSCCodec,
         bIsPLCFrame ? NULL : &Bitstream,
         &PCMStream);
      assert(uscStatus == USC_NoError);

      if (uscStatus != USC_NoError)
      {
         return 0;
      }

      decodedSamples = PCMStream.nbytes / sizeof(MpAudioSample);
   }

   // Return number of decoded samples
   return decodedSamples;
}

void MpdIPPG7291::configureBitStream(int rtpPayloadBytes, char* bitstream)
{
   static int ftToBitrate[] = {8000, 12000, 14000, 16000, 18000, 20000, 22000, 24000, 26000, 28000, 30000,
      32000, 0, 0, 0, 0};

   Bitstream.nbytes = rtpPayloadBytes;

   int ftField = 0;
   if (bitstream)
   {
      ftField = bitstream[0] & 0x0F;
   }

   if (ftField == 14)
   {
      Bitstream.frametype = 0; // TODO - use real frame type once Intel IPP implement VAD/DTX for G.729.1
   }
   else
   {
      Bitstream.frametype = 0;
   }
   Bitstream.bitrate = ftToBitrate[ftField];
}

const MpCodecInfo* MpdIPPG7291::getCodecInfo()
{
   return &smCodecInfo;
}

const MpCodecInfo MpdIPPG7291::smCodecInfo(
   SdpCodec::SDP_CODEC_G7291_32000,    // codecType
   "Intel IPP 6.0",             // codecVersion
   16000,                        // samplingRate
   16,                          // numBitsPerSample (not used)
   1,                           // numChannels
   32000,                        // bitRate
   80*8,                        // minPacketBits
   80*8,                        // maxPacketBits
   320);                        // numSamplesPerFrame - 20ms frame

#endif /* !HAVE_INTEL_IPP ] */
