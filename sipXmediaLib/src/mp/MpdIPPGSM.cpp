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
#include "mp/MpdIPPGSM.h"

extern "C" {
#include "ippcore.h"
#include "ipps.h"
#include "usccodec.h"
}

MpdIPPGSM::MpdIPPGSM(int payloadType)
: MpDecoderBase(payloadType, getCodecInfo())
{
   codec = (LoadedCodec*)malloc(sizeof(LoadedCodec));
   memset(codec, 0, sizeof(LoadedCodec));
}

MpdIPPGSM::~MpdIPPGSM()
{
   freeDecode();
   free(codec);
}

OsStatus MpdIPPGSM::initDecode()
{
   int lCallResult;

   ippStaticInit();
   codec->lIsVad = 0;
   strcpy((char*)codec->codecName, "IPP_GSMFR");

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

OsStatus MpdIPPGSM::freeDecode(void)
{
   // Free codec memory
   USCFree(&codec->uscParams);

   return OS_SUCCESS;
}

int MpdIPPGSM::decode(const MpRtpBufPtr &rtpPacket,
                       unsigned decodedBufferLength,
                       MpAudioSample *samplesBuffer,
                       UtlBoolean bIsPLCFrame) 
{
   if (!rtpPacket.isValid())
      return 0;

   unsigned payloadSize = rtpPacket->getPayloadSize();
   unsigned maxPayloadSize = getInfo()->getMaxPacketBits()/8;

   assert(payloadSize = maxPayloadSize);
   if (payloadSize != maxPayloadSize)
   {
      return 0;
   }

   unsigned int decodedSamples = 0;

   configureBitStream(payloadSize);
   // Setup input and output pointers
   Bitstream.pBuffer = const_cast<char*>(rtpPacket->getDataPtr());
   PCMStream.pBuffer = reinterpret_cast<char*>(samplesBuffer);
   // zero the buffer in case we decode less than 320 bytes
   memset(PCMStream.pBuffer, 0, 320);

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

   // Return number of decoded samples
   return decodedSamples;
}

void MpdIPPGSM::configureBitStream(int rtpPayloadBytes)
{
   Bitstream.nbytes = rtpPayloadBytes;
   Bitstream.frametype = -1;
   Bitstream.bitrate = getInfo()->getBitRate();
}

const MpCodecInfo* MpdIPPGSM::getCodecInfo()
{
   return &ms_codecInfo;
}

const MpCodecInfo MpdIPPGSM::ms_codecInfo(
   SdpCodec::SDP_CODEC_GSM,    // codecType
   "Intel IPP 6.0",             // codecVersion
   8000,                        // samplingRate
   16,                          // numBitsPerSample (not used)
   1,                           // numChannels
   13000,                        // bitRate
   33*8,                        // minPacketBits
   33*8,                        // maxPacketBits
   160);                        // numSamplesPerFrame - 20ms frame

#endif /* !HAVE_INTEL_IPP ] */
