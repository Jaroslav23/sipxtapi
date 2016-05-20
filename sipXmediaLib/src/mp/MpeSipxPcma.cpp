//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include "assert.h"
// APPLICATION INCLUDES
#include "mp/MpeSipxPcma.h"
#include "mp/MpSipxDecoders.h"

const MpCodecInfo MpeSipxPcma::smCodecInfo(
         SdpCodec::SDP_CODEC_PCMA,// codecType
         "SIPfoundry 1.0",// codecVersion
         8000,// samplingRate
         16,// numBitsPerSample
         1,// numChannels
         64000,// bitRate. It doesn't matter right now.
         1280,// minPacketBits
         1280,// maxPacketBits
         160);// numSamplesPerFrame - 20ms frame

MpeSipxPcma::MpeSipxPcma(int payloadType)
   : MpEncoderBase(payloadType, &smCodecInfo)
{

}

MpeSipxPcma::~MpeSipxPcma()
{
   freeEncode();
}

OsStatus MpeSipxPcma::initEncode(void)
{
   return OS_SUCCESS;
}

OsStatus MpeSipxPcma::freeEncode(void)
{
   return OS_SUCCESS;
}

OsStatus MpeSipxPcma::encode(const MpAudioSample* pAudioSamples,
                             const int numSamples,
                             int& rSamplesConsumed,
                             unsigned char* pCodeBuf,
                             const int bytesLeft,
                             int& rSizeInBytes,
                             UtlBoolean& sendNow,
                             MpSpeechType& speechType)
{
   if (speechType == MP_SPEECH_SILENT && ms_bEnableVAD)
   {
      // VAD must be enabled, do DTX
      rSamplesConsumed = numSamples;
      rSizeInBytes = 0;
      sendNow = TRUE; // sends any unsent frames now
      return OS_SUCCESS;
   }

   G711A_Encoder(numSamples, pAudioSamples, (uint8_t*)pCodeBuf);
   rSizeInBytes = numSamples;
   sendNow = FALSE;
   rSamplesConsumed = numSamples;

   return OS_SUCCESS;
}
