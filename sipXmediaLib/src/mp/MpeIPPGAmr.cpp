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

#ifdef HAVE_INTEL_IPP // [

#include "assert.h"
// APPLICATION INCLUDES
#include "winsock2.h"
#include "mp/MpeIPPGAmr.h"

extern "C" {
#include "ippcore.h"
#include "ipps.h"
#include "usccodec.h"
}

MpeIPPGAmr::MpeIPPGAmr(int payloadType, int bitRate, UtlBoolean bOctetAligned)
: MpEncoderBase(payloadType, getCodecInfo())
, m_bitrate(bitRate)
, m_bOctetAligned(bOctetAligned)
, m_storedFramesCount(0)
, m_amrPacketizer(NULL)
, m_pInputBuffer(NULL)
, m_pOutputBuffer(NULL)
{
   m_pCodec = (LoadedCodec*)malloc(sizeof(LoadedCodec));
   memset(m_pCodec, 0, sizeof(LoadedCodec));
   m_amrPacketizer = new UMC::AMRPacketizer();

   UMC::AMRPacketizerParams params;
   params.m_CodecType = UMC::NB;
   params.m_InterleavingFlag = 0;
   params.m_ptType = bOctetAligned ? UMC::OctetAlign : UMC::BandEfficient;
   UMC::Status result = m_amrPacketizer->Init(&params);
   assert(result == USC_NoError);

   UMC::AMRControlParams controlParams; // use default values
   result = m_amrPacketizer->SetControls(&controlParams);
   assert(result == USC_NoError);

   m_pMediaData = new UMC::SpeechData();
   m_pAmrData = new UMC::SpeechData();
}

MpeIPPGAmr::~MpeIPPGAmr()
{
   freeEncode();
   free(m_pCodec);
   delete m_amrPacketizer;
   m_amrPacketizer = NULL;
   m_pMediaData->Reset();
   delete m_pMediaData;
   m_pMediaData = NULL;
   m_pAmrData->Reset();
   delete m_pAmrData;
   m_pAmrData = NULL;
}

OsStatus MpeIPPGAmr::initEncode(void)
{
   int lCallResult;

   ippStaticInit();
   strcpy((char*)m_pCodec->codecName, "IPP_GSMAMR");
   m_pCodec->lIsVad = 1;

   // Load codec by name from command line
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

   // Set params for encode
   USC_PCMType streamType;
   streamType.bitPerSample = getInfo()->getNumBitsPerSample();
   streamType.nChannels = getInfo()->getNumChannels();
   streamType.sample_frequency = getInfo()->getSamplingRate();

   lCallResult = SetUSCEncoderPCMType(&m_pCodec->uscParams, LINEAR_PCM, &streamType, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   // instead of SetUSCEncoderParams(...)
   m_pCodec->uscParams.pInfo->params.direction = USC_ENCODE;
   m_pCodec->uscParams.pInfo->params.law = 0;
   m_pCodec->uscParams.pInfo->params.modes.bitrate = m_bitrate;
   m_pCodec->uscParams.pInfo->params.modes.vad = ms_bEnableVAD ? 1 : 0;

   // Alloc memory for the codec
   lCallResult = USCCodecAlloc(&m_pCodec->uscParams, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   // Init decoder
   lCallResult = USCEncoderInit(&m_pCodec->uscParams, NULL, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;  
   }

   // Allocate memory for the output buffer. Size of output buffer is equal
   // to the size of 1 frame
   m_pInputBuffer = (Ipp8s *)ippsMalloc_8s(m_pCodec->uscParams.pInfo->params.framesize);
   m_pOutputBuffer = (Ipp8u *)ippsMalloc_8u(m_pCodec->uscParams.pInfo->maxbitsize + 10);

   m_storedFramesCount = 0;

   return OS_SUCCESS;
}

OsStatus MpeIPPGAmr::freeEncode(void)
{
   // Free codec memory
   USCFree(&m_pCodec->uscParams);
   if (m_pInputBuffer)
   {
      ippsFree(m_pInputBuffer);
      m_pInputBuffer = NULL;
   }
   if (m_pOutputBuffer)
   {
      ippsFree(m_pOutputBuffer);
      m_pOutputBuffer = NULL;
   }

   return OS_SUCCESS;
}


OsStatus MpeIPPGAmr::encode(const short* pAudioSamples,
                            const int numSamples,
                            int& rSamplesConsumed,
                            unsigned char* pCodeBuf,
                            const int bytesLeft,
                            int& rSizeInBytes,
                            UtlBoolean& sendNow,
                            MpSpeechType& speechType)
{
   assert(numSamples == 80);

   if (m_storedFramesCount == 1)
   {
      ippsSet_8u(0, (Ipp8u *)m_pOutputBuffer, m_pCodec->uscParams.pInfo->maxbitsize);
      ippsCopy_8u((unsigned char *)pAudioSamples, 
                  (unsigned char *)m_pInputBuffer+m_storedFramesCount*160,
                  numSamples * sizeof(MpAudioSample));

      int infrmLen, FrmDataLen;
      USC_PCMStream PCMStream;
      USC_Bitstream Bitstream;

      // Do the pre-procession of the frame
      infrmLen = USCEncoderPreProcessFrame(&m_pCodec->uscParams , m_pInputBuffer,
                 (Ipp8s *)m_pOutputBuffer, &PCMStream, &Bitstream);
      // Encode one frame
      FrmDataLen = USCCodecEncode(&m_pCodec->uscParams, &PCMStream, &Bitstream, 0);
      if(FrmDataLen < 0)
      {
         return OS_FAILED;
      }

      infrmLen += FrmDataLen;
      // Do the post-procession of the frame
      USCEncoderPostProcessFrame(&m_pCodec->uscParams, m_pInputBuffer,
               (Ipp8s *)m_pOutputBuffer, &PCMStream, &Bitstream);

      ippsSet_8u(0, pCodeBuf, Bitstream.nbytes);

      // store encoded frames in AMR format. Frames cannot go directly into RTP payload
      UMC::Status result;
      m_pMediaData->Reset();
      result = m_pMediaData->SetBufferPointer(m_pOutputBuffer + 6, Bitstream.nbytes);
      m_pMediaData->SetDataSize(Bitstream.nbytes);
      m_pMediaData->SetFrameType(Bitstream.frametype);
      m_pMediaData->SetBitrate(Bitstream.bitrate);
      m_pMediaData->SetNBytes(Bitstream.nbytes);
      result = m_amrPacketizer->AddFrame(m_pMediaData);
      m_pAmrData->Reset();
      result = m_pAmrData->SetBufferPointer(pCodeBuf, bytesLeft);
      result = m_amrPacketizer->GetPacket(m_pAmrData);// creates data in amr format in pCodeBuf
      assert(result == USC_NoError);
      rSizeInBytes = m_pAmrData->GetDataSize();

      sendNow = TRUE;
      m_storedFramesCount = 0;
   }
   else
   {
      ippsCopy_8u((unsigned char *)pAudioSamples,
         (unsigned char *)m_pInputBuffer+m_storedFramesCount*160,
         numSamples * sizeof(MpAudioSample));

      m_storedFramesCount++;
      sendNow = FALSE;
      rSizeInBytes = 0;
   }

   rSamplesConsumed = numSamples;

   return OS_SUCCESS;
}

const MpCodecInfo* MpeIPPGAmr::getCodecInfo()
{
   return &ms_codecInfo;
}

const MpCodecInfo MpeIPPGAmr::ms_codecInfo(
   SdpCodec::SDP_CODEC_AMR_10200,    // codecType
   "Intel IPP 6.0",             // codecVersion
   8000,                        // samplingRate
   16,                          // numBitsPerSample (not used)
   1,                           // numChannels
   10200,                        // bitRate
   26*8,                        // minPacketBits
   26*8,                        // maxPacketBits
   160);                        // numSamplesPerFrame - 20ms frame

#endif // HAVE_INTEL_IPP ]
