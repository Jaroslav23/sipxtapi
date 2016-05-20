//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// Copyright (C) 2006-2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2008-2009 Jaroslav Libak.  All rights reserved.
// Licensed under the LGPL license.
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifdef HAVE_SPAN_DSP /* [ */

// APPLICATION INCLUDES
#include "mp/MpdSipxG722.h"

const MpCodecInfo MpdSipxG722::ms_codecInfo64(
   SdpCodec::SDP_CODEC_G722,    // codecType
   "",                 // codecVersion
   16000,                       // samplingRate
   16,                          // numBitsPerSample
   1,                          // numChannels
   64000,                      // bitRate. It doesn't matter right now.
   160*8,                       // minPacketBits
   160*8,                       // maxPacketBits
   320);                       // numSamplesPerFrame - 10ms frame

MpdSipxG722::MpdSipxG722(int payloadType)
: MpDecoderBase(payloadType, getCodecInfo())
, m_pG722state(NULL)
{
}

MpdSipxG722::~MpdSipxG722()
{
   freeDecode();
}

OsStatus MpdSipxG722::initDecode()
{
   m_pG722state = g722_decode_init(NULL, getInfo()->getBitRate(), 0);

   if (m_pG722state)
   {
      return OS_SUCCESS;
   }
   else
   {
      return OS_FAILED;
   }
}

OsStatus MpdSipxG722::freeDecode(void)
{
   int res = 0;
   
   if (m_pG722state)
   {
      res = g722_decode_release(m_pG722state);
      m_pG722state = NULL;
   }

   if (res == 0)
   {
      return OS_SUCCESS;
   }
   else
   {
      return OS_FAILED;
   }
}

int MpdSipxG722::decode(const MpRtpBufPtr &pPacket,
                        unsigned decodedBufferLength,
                        MpAudioSample *samplesBuffer,
                        UtlBoolean bIsPLCFrame)
{
   if (!pPacket.isValid())
      return 0;

   unsigned payloadSize = pPacket->getPayloadSize();
   unsigned maxPayloadSize = ms_codecInfo64.getMaxPacketBits()/8;
   // do not accept frames longer than 20ms from RTP to protect against buffer overflow
   assert(payloadSize <= maxPayloadSize);
   if (payloadSize > maxPayloadSize || payloadSize <= 1)
   {
      return 0;
   }

   int samplesDecoded = g722_decode(m_pG722state, samplesBuffer, (const uint8_t*)pPacket->getDataPtr(), (int)pPacket->getPayloadSize());

   return samplesDecoded;
}

const MpCodecInfo* MpdSipxG722::getCodecInfo()
{
   return &ms_codecInfo64;
}

#endif /* HAVE_SPAN_DSP ] */
