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
#include "mp/MpdIPPGAmrWb.h"

extern "C" {
#include "ippcore.h"
#include "ipps.h"
#include "usccodec.h"
}

MpdIPPGAmrWb::MpdIPPGAmrWb(int payloadType, int bitRate, UtlBoolean bOctetAligned)
: MpDecoderBase(payloadType, getCodecInfo())
, m_bitrate(bitRate)
, m_pInputBuffer(NULL)
, m_amrDepacketizer(NULL)
, m_pMediaData(NULL)
, m_pAmrData(NULL)
, m_bOctetAligned(bOctetAligned)
{
   codec = (LoadedCodec*)malloc(sizeof(LoadedCodec));
   memset(codec, 0, sizeof(LoadedCodec));

   m_amrDepacketizer = new UMC::AMRDePacketizer();

   UMC::AMRDePacketizerParams params;
   params.m_CodecType = UMC::WB;
   params.m_InterleavingFlag = 0;
   params.m_ptType = bOctetAligned ? UMC::OctetAlign : UMC::BandEfficient;
   UMC::Status result = m_amrDepacketizer->Init(&params);
   assert(result == USC_NoError);

   m_pMediaData = new UMC::SpeechData();
   m_pAmrData = new UMC::SpeechData();
}

MpdIPPGAmrWb::~MpdIPPGAmrWb()
{
   freeDecode();
   free(codec);
   delete m_amrDepacketizer;
   m_amrDepacketizer = NULL;
   m_pMediaData->Reset();
   delete m_pMediaData;
   m_pMediaData = NULL;
   m_pAmrData->Reset();
   delete m_pAmrData;
   m_pAmrData = NULL;
}

OsStatus MpdIPPGAmrWb::initDecode()
{
   int lCallResult;

   ippStaticInit();
   codec->lIsVad = 1;
   strcpy((char*)codec->codecName, "IPP_AMRWB");

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
   codec->uscParams.pInfo->params.modes.vad = 1;

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

   m_pInputBuffer = (Ipp8s*)ippsMalloc_8u(codec->uscParams.pInfo->maxbitsize);

   return OS_SUCCESS;
}

OsStatus MpdIPPGAmrWb::freeDecode(void)
{
   // Free codec memory
   USCFree(&codec->uscParams);
   if (m_pInputBuffer)
   {
      ippsFree(m_pInputBuffer);
      m_pInputBuffer = NULL;
   }

   return OS_SUCCESS;
}

int MpdIPPGAmrWb::decode(const MpRtpBufPtr &rtpPacket,
                       unsigned decodedBufferLength,
                       MpAudioSample *samplesBuffer,
                       UtlBoolean bIsPLCFrame) 
{
   if (!rtpPacket.isValid())
      return 0;

   unsigned payloadSize = rtpPacket->getPayloadSize();
   unsigned maxPayloadSize = getInfo()->getMaxPacketBits()/4; // allow some reserve

   assert(payloadSize <= maxPayloadSize);
   if (payloadSize > maxPayloadSize || payloadSize <= 1)
   {
      return 0;
   }

   unsigned int decodedSamples = 0;
  
   if (payloadSize <= 5 && !bIsPLCFrame)
   {
      // MpDecodeBuffer will generate comfort noise
      return 0;
   }
   else
   {
      // data in RTP is in amr format, we need to adjust it to be in samples
      UMC::Status result;
      m_pAmrData->Reset();
      result = m_pAmrData->SetBufferPointer((Ipp8u*)rtpPacket->getDataPtr(), payloadSize);
      assert(result == USC_NoError);
      result = m_amrDepacketizer->SetPacket(m_pAmrData);
      assert(result == USC_NoError);
      m_pMediaData->Reset();
      m_pMediaData->SetBufferPointer((Ipp8u*)m_pInputBuffer, codec->uscParams.pInfo->maxbitsize);
      result = m_amrDepacketizer->GetNextFrame(m_pMediaData);
      assert(result == USC_NoError);
      Bitstream.frametype = m_pMediaData->GetFrameType();
      Bitstream.bitrate = m_pMediaData->GetBitrate();
      Bitstream.nbytes = m_pMediaData->GetNBytes();

      if (Bitstream.frametype != 0 && Bitstream.frametype != 4)
      {
         return 0;
      }

      // Setup input and output pointers
      Bitstream.pBuffer = (char*)m_pMediaData->GetDataPointer();
      PCMStream.pBuffer = reinterpret_cast<char*>(samplesBuffer);
      // zero the buffer in case we decode less than 640 bytes
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

const MpCodecInfo* MpdIPPGAmrWb::getCodecInfo()
{
   return &ms_codecInfo;
}

const MpCodecInfo MpdIPPGAmrWb::ms_codecInfo(
   SdpCodec::SDP_CODEC_AMR_WB_23850,    // codecType
   "Intel IPP 6.0",             // codecVersion
   16000,                        // samplingRate
   16,                          // numBitsPerSample (not used)
   1,                           // numChannels
   23850,                        // bitRate
   60*8,                        // minPacketBits
   60*8,                        // maxPacketBits
   320);                        // numSamplesPerFrame - 20ms frame

#endif /* !HAVE_INTEL_IPP ] */
