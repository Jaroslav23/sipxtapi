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
#include "mp/MpeIPPG728.h"

extern "C" {
#include "ippcore.h"
#include "ipps.h"
#include "usccodec.h"
}

MpeIPPG728::MpeIPPG728(int payloadType, int bitRate)
: MpEncoderBase(payloadType, getCodecInfo(bitRate))
, m_pInputBuffer(NULL)
, m_pOutputBuffer(NULL)
{
   m_pCodec = (LoadedCodec*)malloc(sizeof(LoadedCodec));
   memset(m_pCodec, 0, sizeof(LoadedCodec));
}

MpeIPPG728::~MpeIPPG728()
{
   freeEncode();
   free(m_pCodec);
}

OsStatus MpeIPPG728::initEncode(void)
{
   int lCallResult;

   ippStaticInit();
   strcpy((char*)m_pCodec->codecName, "IPP_G728");

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
   m_pCodec->uscParams.pInfo->params.modes.bitrate = getInfo()->getBitRate();
   m_pCodec->uscParams.pInfo->params.modes.vad = 0; // not supported for G.728

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

   return OS_SUCCESS;
}

OsStatus MpeIPPG728::freeEncode(void)
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


OsStatus MpeIPPG728::encode(const short* pAudioSamples,
                            const int numSamples,
                            int& rSamplesConsumed,
                            unsigned char* pCodeBuf,
                            const int bytesLeft,
                            int& rSizeInBytes,
                            UtlBoolean& sendNow,
                            MpSpeechType& speechType)
{
   assert(80 == numSamples);

   if (speechType == MP_SPEECH_SILENT && ms_bEnableVAD) // G.728 doesn't have built in VAD
   {
      // VAD must be enabled, do DTX
      rSamplesConsumed = numSamples;
      rSizeInBytes = 0;
      sendNow = TRUE; // sends any unsent frames now
      return OS_SUCCESS;
   }

   ippsSet_8u(0, m_pOutputBuffer, m_pCodec->uscParams.pInfo->maxbitsize + 1);
   ippsCopy_8u((Ipp8u*)pAudioSamples,
               (Ipp8u*)m_pInputBuffer,
               numSamples*sizeof(MpAudioSample));     

   int frmlen, infrmLen, FrmDataLen;
   USC_PCMStream PCMStream; // nbytes will be set automatically
   USC_Bitstream Bitstream;

   // Do the pre-procession of the frame
   infrmLen = USCEncoderPreProcessFrame(&m_pCodec->uscParams, m_pInputBuffer,
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
            (Ipp8s *)m_pOutputBuffer, &PCMStream, &Bitstream);

   unsigned maxPacketBytes = getInfo()->getMaxPacketBits()/8;

   ippsSet_8u(0, pCodeBuf, maxPacketBytes); 

   if (Bitstream.nbytes <= maxPacketBytes)
   {
      for(int k = 0; k < Bitstream.nbytes; ++k)
      {
         pCodeBuf[k] = m_pOutputBuffer[6 + k];
      }
   }
   else
   {
      frmlen = 0;
   }

   sendNow = FALSE;
   rSamplesConsumed = FrmDataLen / (m_pCodec->uscParams.pInfo->params.pcmType.bitPerSample / 8);

   if (Bitstream.nbytes >= 0)
   {
      rSizeInBytes = Bitstream.nbytes;
   }
   else
   {
      rSizeInBytes = 0;
   }

   return OS_SUCCESS;
}

const MpCodecInfo* MpeIPPG728::getCodecInfo(int bitRate)
{
   const MpCodecInfo* pCodecInfo = &ms_codecInfo9600;
   switch(bitRate)
   {
   case 9600:
      pCodecInfo = &ms_codecInfo9600;
      break;
   case 12800:
      pCodecInfo = &ms_codecInfo12800;
      break;
   case 16000:
      pCodecInfo = &ms_codecInfo16000;
      break;
   default:
      ;
   }

   return pCodecInfo;
}

const MpCodecInfo MpeIPPG728::ms_codecInfo9600(
   SdpCodec::SDP_CODEC_G728,    // codecType
   "Intel IPP 6.0",             // codecVersion
   8000,                        // samplingRate
   16,                          // numBitsPerSample (not used)
   1,                           // numChannels
   9600,                        // bitRate
   24*8,                        // minPacketBits
   24*8,                        // maxPacketBits
   160);                        // numSamplesPerFrame - 20ms frame

const MpCodecInfo MpeIPPG728::ms_codecInfo12800(
   SdpCodec::SDP_CODEC_G728,    // codecType
   "Intel IPP 6.0",             // codecVersion
   8000,                        // samplingRate
   16,                          // numBitsPerSample (not used)
   1,                           // numChannels
   12800,                        // bitRate
   32*8,                        // minPacketBits
   32*8,                        // maxPacketBits
   160);                        // numSamplesPerFrame - 20ms frame

const MpCodecInfo MpeIPPG728::ms_codecInfo16000(
   SdpCodec::SDP_CODEC_G728,    // codecType
   "Intel IPP 6.0",             // codecVersion
   8000,                        // samplingRate
   16,                          // numBitsPerSample (not used)
   1,                           // numChannels
   16000,                        // bitRate
   40*8,                        // minPacketBits
   40*8,                        // maxPacketBits
   160);                        // numSamplesPerFrame - 20ms frame

#endif // HAVE_INTEL_IPP ]
