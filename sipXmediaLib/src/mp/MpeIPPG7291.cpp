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
#include "mp/MpeIPPG7291.h"

extern "C" {
#include "ippcore.h"
#include "ipps.h"
#include "usccodec.h"
}

MpeIPPG7291::MpeIPPG7291(int payloadType, int bitRate)
: MpEncoderBase(payloadType, getCodecInfo())
, m_bitRate(bitRate)
, m_storedFramesCount(0)
, m_pInputBuffer(NULL)
, m_pOutputBuffer(NULL)
{
   m_pCodec = (LoadedCodec*)malloc(sizeof(LoadedCodec));
   memset(m_pCodec, 0, sizeof(LoadedCodec));
}

MpeIPPG7291::~MpeIPPG7291()
{
   freeEncode();
   free(m_pCodec);
}

OsStatus MpeIPPG7291::initEncode(void)
{
   int lCallResult;

   ippStaticInit();
   strcpy((char*)m_pCodec->codecName, "IPP_G729.1");
   m_pCodec->lIsVad = 0; // built in VAD is not yet supported by Intel IPP

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
   m_pCodec->uscParams.pInfo->params.modes.bitrate = m_bitRate;
   m_pCodec->uscParams.pInfo->params.modes.vad = 0; // built in VAD is not yet supported by Intel IPP

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

OsStatus MpeIPPG7291::freeEncode(void)
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


OsStatus MpeIPPG7291::encode(const short* pAudioSamples,
                            const int numSamples,
                            int& rSamplesConsumed,
                            unsigned char* pCodeBuf,
                            const int bytesLeft,
                            int& rSizeInBytes,
                            UtlBoolean& sendNow,
                            MpSpeechType& speechType)
{
   assert(numSamples == 160);

   if (speechType == MP_SPEECH_SILENT && ms_bEnableVAD)
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
                  (unsigned char *)m_pInputBuffer + m_storedFramesCount*320,
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
         // construct G.729.1 header
         pCodeBuf[0] = 0xF0 | getFTField(Bitstream.nbytes);
         // copy the rest
         for(int k = 0; k < Bitstream.nbytes; ++k)
         {
            pCodeBuf[k + 1] = m_pOutputBuffer[6 + k];
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
         rSizeInBytes = Bitstream.nbytes + 1;
      }
      else
      {
         rSizeInBytes = 0;
      }
   }
   else
   {
      ippsCopy_8u((unsigned char *)pAudioSamples,
         (unsigned char *)m_pInputBuffer+m_storedFramesCount*320,
         numSamples * sizeof(MpAudioSample));  

      m_storedFramesCount++;

      sendNow = FALSE;
      rSizeInBytes = 0;
      rSamplesConsumed = numSamples;
   }

   return OS_SUCCESS;
}

const MpCodecInfo* MpeIPPG7291::getCodecInfo()
{
   return &smCodecInfo;
}

char MpeIPPG7291::getFTField(int octets) const
{
   char ftField = 15;

   switch (octets)
   {
   case 20:
      ftField = 0;
      break;
   case 30:
      ftField = 1;
      break;
   case 35:
      ftField = 2;
      break;
   case 40:
      ftField = 3;
      break;
   case 45:
      ftField = 4;
      break;
   case 50:
      ftField = 5;
      break;
   case 55:
      ftField = 6;
      break;
   case 60:
      ftField = 7;
      break;
   case 65:
      ftField = 8;
      break;
   case 70:
      ftField = 9;
      break;
   case 75:
      ftField = 10;
      break;
   case 80:
      ftField = 11;
      break;
   case 2: // SID
   case 3:
   case 6:
      ftField = 14;
      break;
   default:
      ;
   }

   return ftField;
}

const MpCodecInfo MpeIPPG7291::smCodecInfo(
   SdpCodec::SDP_CODEC_G7291_32000,    // codecType
   "Intel IPP 6.0",             // codecVersion
   16000,                        // samplingRate
   16,                          // numBitsPerSample (not used)
   1,                           // numChannels
   32000,                        // bitRate
   80*8,                        // minPacketBits
   80*8,                        // maxPacketBits
   320);                        // numSamplesPerFrame - 20ms frame

#endif // HAVE_INTEL_IPP ]
