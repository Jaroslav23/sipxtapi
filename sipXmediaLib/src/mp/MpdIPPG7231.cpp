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

// APPLICATION INCLUDES
#include "mp/MpdIPPG7231.h"

extern "C" {
#include "ippcore.h"
#include "ipps.h"
#include "usccodec.h"
}

#define G723_PATTERN_LENGTH_5300 20
#define G723_PATTERN_LENGTH_6300 24

const MpCodecInfo MpdIPPG7231::smCodecInfo(
   SdpCodec::SDP_CODEC_G723,    // codecType
   "Intel IPP 6.0",             // codecVersion
   8000,                        // samplingRate
   16,                          // numBitsPerSample (not used)
   1,                           // numChannels
   8000,                        // bitRate
   20*8,                         // minPacketBits
   24*8,                         // maxPacketBits
   240);                          // numSamplesPerFrame

MpdIPPG7231::MpdIPPG7231(int payloadType)
: MpDecoderBase(payloadType, &smCodecInfo)
{
   codec5300 = (LoadedCodec*)malloc(sizeof(LoadedCodec));
   memset(codec5300, 0, sizeof(LoadedCodec));
   codec6300 = (LoadedCodec*)malloc(sizeof(LoadedCodec));
   memset(codec6300, 0, sizeof(LoadedCodec));
}

MpdIPPG7231::~MpdIPPG7231()
{
   freeDecode();
   free(codec5300);
   free(codec6300);
}

OsStatus MpdIPPG7231::initDecode()
{
   int lCallResult;

   ippStaticInit();

   // Apply codec name and VAD to codec definition structure
   strcpy((char*)codec6300->codecName, "IPP_G723.1");
   codec6300->lIsVad = 1;
   strcpy((char*)codec5300->codecName, "IPP_G723.1");
   codec5300->lIsVad = 1;

   // Load codec by name from command line
   lCallResult = LoadUSCCodecByName(codec6300,NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   lCallResult = LoadUSCCodecByName(codec5300,NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   // Get USC codec params
   lCallResult = USCCodecAllocInfo(&codec6300->uscParams, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   lCallResult = USCCodecAllocInfo(&codec5300->uscParams, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   lCallResult = USCCodecGetInfo(&codec6300->uscParams, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   lCallResult = USCCodecGetInfo(&codec5300->uscParams, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   // Get its supported format details
   lCallResult = GetUSCCodecParamsByFormat(codec6300, BY_NAME, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   // Get its supported format details
   lCallResult = GetUSCCodecParamsByFormat(codec5300, BY_NAME, NULL);
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
   lCallResult = SetUSCDecoderPCMType(&codec6300->uscParams, -1, &streamType, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   // instead of SetUSCDecoderParams(...)
   codec6300->uscParams.pInfo->params.direction = USC_DECODE;
   codec6300->uscParams.pInfo->params.law = 0;
   codec6300->uscParams.pInfo->params.modes.bitrate = 6300;
   codec6300->uscParams.pInfo->params.modes.vad =1;

   // decoder doesn't need to know PCM type
   lCallResult = SetUSCDecoderPCMType(&codec5300->uscParams, -1, &streamType, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   // instead of SetUSCDecoderParams(...)
   codec5300->uscParams.pInfo->params.direction = USC_DECODE;
   codec5300->uscParams.pInfo->params.law = 0;
   codec5300->uscParams.pInfo->params.modes.bitrate = 5300;
   codec5300->uscParams.pInfo->params.modes.vad =1;

   // Alloc memory for the codec
   lCallResult = USCCodecAlloc(&codec6300->uscParams, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   lCallResult = USCCodecAlloc(&codec5300->uscParams, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   // Init decoder
   lCallResult = USCDecoderInit(&codec6300->uscParams, NULL, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   lCallResult = USCDecoderInit(&codec5300->uscParams, NULL, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   return OS_SUCCESS;
}

OsStatus MpdIPPG7231::freeDecode(void)
{
   // Free codec memory
   USCFree(&codec6300->uscParams);
   USCFree(&codec5300->uscParams);

   return OS_SUCCESS;
}

int MpdIPPG7231::decode(const MpRtpBufPtr &rtpPacket,
                       unsigned int decodedBufferLength,
                       MpAudioSample *samplesBuffer,
                       UtlBoolean bIsPLCFrame) 
{
   if (!rtpPacket.isValid())
      return 0;

   unsigned payloadSize = rtpPacket->getPayloadSize();
   unsigned maxPayloadSize = smCodecInfo.getMaxPacketBits()/8;

   assert(payloadSize <= maxPayloadSize);
   if (payloadSize > maxPayloadSize || payloadSize <= 1)
   {
      return 0;
   }

   // Prepare encoded buffer parameters
   if (payloadSize == G723_PATTERN_LENGTH_6300)
   {
      Bitstream.bitrate = 6300;
      Bitstream.frametype = 0;
      Bitstream.nbytes = 24;
   }
   else if (payloadSize == G723_PATTERN_LENGTH_5300)
   {
      Bitstream.bitrate = 5300;
      Bitstream.frametype = 0;
      Bitstream.nbytes = 20;
   }
   else if (!bIsPLCFrame)
   {
      // MpDecodeBuffer will generate comfort noise
      return 0;
   }

   Bitstream.pBuffer = const_cast<char*>(rtpPacket->getDataPtr());
   PCMStream.pBuffer = reinterpret_cast<char*>(samplesBuffer);

   unsigned int decodedSamples = 0;
   USC_Status uscStatus;

   // Decode one frame
   if (payloadSize == G723_PATTERN_LENGTH_6300)
   {
      uscStatus = codec6300->uscParams.USC_Fns->Decode(codec6300->uscParams.uCodec.hUSCCodec,
         bIsPLCFrame ? NULL : &Bitstream, &PCMStream);
      assert(uscStatus == USC_NoError);
      decodedSamples = PCMStream.nbytes / sizeof(MpAudioSample);
   }
   else
   {
      uscStatus = codec5300->uscParams.USC_Fns->Decode(codec5300->uscParams.uCodec.hUSCCodec,
         bIsPLCFrame ? NULL : &Bitstream, &PCMStream);
      assert(uscStatus == USC_NoError);
      decodedSamples = PCMStream.nbytes / sizeof(MpAudioSample);
   }

   if (uscStatus != USC_NoError)
   {
      return 0;
   }

   // Return number of decoded samples
   return decodedSamples;
}

#endif /* !HAVE_INTEL_IPP ] */
