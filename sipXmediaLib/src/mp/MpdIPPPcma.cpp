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
#include "mp/MpdIPPPcma.h"

extern "C" {
#include "ippcore.h"
#include "ipps.h"
#include "usccodec.h"
}

MpdIPPPcma::MpdIPPPcma(int payloadType)
: MpDecoderBase(payloadType, getCodecInfo())
{
   m_pCodec = (LoadedCodec*)malloc(sizeof(LoadedCodec));
   memset(m_pCodec, 0, sizeof(LoadedCodec));
}

MpdIPPPcma::~MpdIPPPcma()
{
   freeDecode();
   free(m_pCodec);
}

OsStatus MpdIPPPcma::initDecode()
{
   int lCallResult;

   ippStaticInit();
   m_pCodec->lIsVad = 1;
   strcpy((char*)m_pCodec->codecName, "IPP_G711A");

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

   // Prepare input buffer parameters
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

OsStatus MpdIPPPcma::freeDecode(void)
{
   // Free codec memory
   USCFree(&m_pCodec->uscParams);

   return OS_SUCCESS;
}

int MpdIPPPcma::decode(const MpRtpBufPtr &rtpPacket,
                       unsigned decodedBufferLength,
                       MpAudioSample *samplesBuffer,
                       UtlBoolean bIsPLCFrame) 
{
   if (!rtpPacket.isValid())
      return 0;

   unsigned payloadSize = rtpPacket->getPayloadSize();
   unsigned maxPayloadSize = getInfo()->getMaxPacketBits()/8;

   assert(payloadSize <= maxPayloadSize);
   if (payloadSize > maxPayloadSize)
   {
      return 0;
   }

   if (payloadSize <= 2 && !bIsPLCFrame)
   {
      // MpDecodeBuffer will generate comfort noise
      return 0;
   }
   else
   {
      unsigned int decodedSamples = 0;

      configureBitStream(payloadSize);

      int frames = 0;
      frames = payloadSize / Bitstream.nbytes;

      // Setup input and output pointers
      Bitstream.pBuffer = const_cast<char*>(rtpPacket->getDataPtr());
      PCMStream.pBuffer = reinterpret_cast<char*>(samplesBuffer);
      // zero the buffer in case we decode less than 320 bytes
      memset(PCMStream.pBuffer, 0, 320);

      // Decode frames
      for (int i = 0; i < frames; i++)
      {
         // Decode one frame
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
}

void MpdIPPPcma::configureBitStream(int rtpPayloadBytes)
{
   if (rtpPayloadBytes % 80 == 0)
   {
      Bitstream.nbytes = 80;
      Bitstream.frametype = 3;
   }
   else
   {
      Bitstream.nbytes = rtpPayloadBytes;
      Bitstream.frametype = 1;
   }

   Bitstream.bitrate = getInfo()->getBitRate();
}

const MpCodecInfo* MpdIPPPcma::getCodecInfo()
{
   return &ms_codecInfo;
}

const MpCodecInfo MpdIPPPcma::ms_codecInfo(
   SdpCodec::SDP_CODEC_PCMA,    // codecType
   "Intel IPP 6.0",             // codecVersion
   8000,                        // samplingRate
   16,                          // numBitsPerSample (not used)
   1,                           // numChannels
   64000,                        // bitRate
   160*8,                        // minPacketBits
   160*8,                        // maxPacketBits
   160);                        // numSamplesPerFrame - 20ms frame

#endif /* !HAVE_INTEL_IPP ] */
