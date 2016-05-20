//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include "assert.h"
// APPLICATION INCLUDES
#include "mp/MpePtAVT.h"

const MpCodecInfo MpePtAVT::smCodecInfo(
         SdpCodec::SDP_CODEC_TONES,// codecType
         "Pingtel_1.0",// codecVersion
         8000,// samplingRate
         0,// numBitsPerSample
         1,// numChannels
         6400,// bitRate. It doesn't matter right now.
         128,// minPacketBits
         128,// maxPacketBits
         160,// numSamplesPerFrame
         0,// requested length of jitter buffer
         TRUE);// signalingCodec

MpePtAVT::MpePtAVT(int payloadType)
   : MpEncoderBase(payloadType, &smCodecInfo)
{
}

MpePtAVT::~MpePtAVT()
{
   freeEncode();
}

OsStatus MpePtAVT::initEncode(void)
{
   return OS_SUCCESS;
}

OsStatus MpePtAVT::freeEncode(void)
{
   return OS_SUCCESS;
}

OsStatus MpePtAVT::encode(const MpAudioSample* pAudioSamples,
                          const int numSamples,
                          int& rSamplesConsumed,
                          unsigned char* pCodeBuf,
                          const int bytesLeft,
                          int& rSizeInBytes,
                          UtlBoolean& sendNow,
                          MpSpeechType& rAudioCategory)
{
   assert(FALSE);
   rSizeInBytes = 0;
   rSamplesConsumed = numSamples;
   sendNow = FALSE;
   return OS_INVALID;
}
