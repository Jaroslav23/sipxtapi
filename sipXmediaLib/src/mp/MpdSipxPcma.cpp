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

// APPLICATION INCLUDES
#include "mp/MpdSipxPcma.h"
#include "mp/MpSipxDecoders.h"
#include "mp/MpDspUtils.h"

const MpCodecInfo MpdSipxPcma::smCodecInfo(
         SdpCodec::SDP_CODEC_PCMA,// codecType
         "SIPfoundry 1.0",// codecVersion
         8000,// samplingRate
         16,// numBitsPerSample
         1,// numChannels
         64000,// bitRate. It doesn't matter right now.
         160*8,// minPacketBits
         160*8,// maxPacketBits
         160);// numSamplesPerFrame - 20ms frame

MpdSipxPcma::MpdSipxPcma(int payloadType)
: MpDecoderBase(payloadType, &smCodecInfo)
{
}

MpdSipxPcma::~MpdSipxPcma()
{
   freeDecode();
}

OsStatus MpdSipxPcma::initDecode()
{
   return OS_SUCCESS;
}

OsStatus MpdSipxPcma::freeDecode()
{
   return OS_SUCCESS;
}

int MpdSipxPcma::decode(const MpRtpBufPtr &pPacket,
                        unsigned decodedBufferLength,// has always sufficient size
                        MpAudioSample *samplesBuffer,
                        UtlBoolean bIsPLCFrame) 
{
   if (!pPacket.isValid())
      return 0;

   unsigned payloadSize = pPacket->getPayloadSize();
   unsigned maxPayloadSize = smCodecInfo.getMaxPacketBits()/8;

   assert(payloadSize <= maxPayloadSize);
   if (payloadSize > maxPayloadSize)
   {
      return 0;
   }

   int sampleCount = pPacket->getPayloadSize(); // number of samples to decode
   G711A_Decoder(sampleCount,
                 (const uint8_t*)pPacket->getDataPtr(),
                 samplesBuffer);
   return sampleCount;
}

