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
#include "mp/MpeIPPGSM.h"

extern "C" {
#include "ippcore.h"
#include "ipps.h"
#include "usccodec.h"
}

MpeIPPGSM::MpeIPPGSM(int payloadType)
: MpEncoderBase(payloadType, getCodecInfo())
, m_storedFramesCount(0)
, m_pInputBuffer(NULL)
, m_pOutputBuffer(NULL)
{
   m_pCodec = (LoadedCodec*)malloc(sizeof(LoadedCodec));
   memset(m_pCodec, 0, sizeof(LoadedCodec));
}

MpeIPPGSM::~MpeIPPGSM()
{
   freeEncode();
   free(m_pCodec);
}

OsStatus MpeIPPGSM::initEncode(void)
{
   int lCallResult;

   ippStaticInit();
   strcpy((char*)m_pCodec->codecName, "IPP_GSMFR");
   m_pCodec->lIsVad = 0;

   // Load m_pCodec by name from command line
   lCallResult = LoadUSCCodecByName(m_pCodec, NULL);
   if (lCallResult < 0)
   {
      return OS_FAILED;
   }

   // Get USC m_pCodec params
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
   m_pCodec->uscParams.pInfo->params.modes.bitrate = getInfo()->getBitRate();
   m_pCodec->uscParams.pInfo->params.modes.vad = 0; // disable internal VAD

   // Alloc memory for the m_pCodec
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
   m_pInputBuffer = (Ipp8s*)ippsMalloc_8s(m_pCodec->uscParams.pInfo->params.framesize);
   m_pOutputBuffer = (Ipp8u*)ippsMalloc_8u(m_pCodec->uscParams.pInfo->maxbitsize + 10);

   m_storedFramesCount = 0;

   return OS_SUCCESS;
}

OsStatus MpeIPPGSM::freeEncode(void)
{
   // Free m_pCodec memory
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


OsStatus MpeIPPGSM::encode(const short* pAudioSamples,
                            const int numSamples,
                            int& rSamplesConsumed,
                            unsigned char* pCodeBuf,
                            const int bytesLeft,
                            int& rSizeInBytes,
                            UtlBoolean& sendNow,
                            MpSpeechType& speechType)
{
   assert(numSamples == 80);

   if (speechType == MP_SPEECH_SILENT && ms_bEnableVAD && m_storedFramesCount == 0)
   {
      // VAD must be enabled, do DTX
      rSamplesConsumed = numSamples;
      rSizeInBytes = 0;
      sendNow = TRUE; // sends any unsent frames now
      return OS_SUCCESS;
   }

   if (m_storedFramesCount == 1)
   {
      ippsSet_8u(0, (Ipp8u*)m_pOutputBuffer, m_pCodec->uscParams.pInfo->maxbitsize);
      ippsCopy_8u((unsigned char *)pAudioSamples,
                  (unsigned char *)m_pInputBuffer + m_storedFramesCount*160,
                  numSamples*sizeof(MpAudioSample));

      int frmlen, infrmLen, FrmDataLen;
      USC_PCMStream PCMStream;
      USC_Bitstream Bitstream;

      // Do the pre-procession of the frame
      infrmLen = USCEncoderPreProcessFrame(&m_pCodec->uscParams , m_pInputBuffer,
                 (Ipp8s*)m_pOutputBuffer, &PCMStream, &Bitstream);
      // Encode one frame
      FrmDataLen = USCCodecEncode(&m_pCodec->uscParams, &PCMStream, &Bitstream, 0);
      if(FrmDataLen < 0)
      {
         return OS_FAILED;
      }

      infrmLen += FrmDataLen;
      // Do the post-procession of the frame
      frmlen = USCEncoderPostProcessFrame(&m_pCodec->uscParams, m_pInputBuffer,
               (Ipp8s*)m_pOutputBuffer, &PCMStream, &Bitstream);

      ippsSet_8u(0, pCodeBuf, m_pCodec->uscParams.pInfo->maxbitsize);

      if (Bitstream.nbytes > 0)
      {
         // copy frames
         for(int k = 0; k < Bitstream.nbytes; ++k)
         {
            pCodeBuf[k] = m_pOutputBuffer[6 + k];
         }
         sendNow = TRUE;
      }
      else
      {
         sendNow = FALSE;
         frmlen = 0;
      }

      rSamplesConsumed = numSamples;
      m_storedFramesCount = 0;

      if (Bitstream.nbytes >= 0)
      {
         rSizeInBytes = Bitstream.nbytes;
      }
      else
      {
         rSizeInBytes = 0;
      }
   }
   else
   {
      ippsCopy_8u((unsigned char *)pAudioSamples,
         (unsigned char *)m_pInputBuffer+m_storedFramesCount*160,
         numSamples * sizeof(MpAudioSample));  

      m_storedFramesCount++;

      sendNow = FALSE;
      rSizeInBytes = 0;
      rSamplesConsumed = numSamples;
   }

   return OS_SUCCESS;
}

const MpCodecInfo* MpeIPPGSM::getCodecInfo()
{
   return &ms_codecInfo;
}

const MpCodecInfo MpeIPPGSM::ms_codecInfo(
   SdpCodec::SDP_CODEC_GSM,    // codecType
   "Intel IPP 6.0",             // codecVersion
   8000,                        // samplingRate
   16,                          // numBitsPerSample (not used)
   1,                           // numChannels
   13000,                        // bitRate
   33*8,                        // minPacketBits
   33*8,                        // maxPacketBits
   160);                        // numSamplesPerFrame - 20ms frame

#endif // HAVE_INTEL_IPP ]
