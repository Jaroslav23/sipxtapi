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

#ifdef HAVE_GSM /* [ */

// WIN32: Add libgsm to linker input.
#ifdef WIN32 // [
#   ifdef _DEBUG // [
#      pragma comment(lib, "gsmd.lib")
#   else // _DEBUG ][
#      pragma comment(lib, "gsm.lib")
#   endif // _DEBUG ]
#endif // WIN32 ]

// APPLICATION INCLUDES
#include "mp/MpdSipxGSM.h"
extern "C" {
#include <gsm.h>
}

const MpCodecInfo MpdSipxGSM::smCodecInfo(
         SdpCodec::SDP_CODEC_GSM,    // codecType
         "GSM 6.10",                 // codecVersion
         8000,                       // samplingRate
         16,                          // numBitsPerSample (not used)
         1,                          // numChannels
         13200,                      // bitRate
         33 * 8,                     // minPacketBits
         33 * 8,                     // maxPacketBits
         160);                        // numSamplesPerFrame

MpdSipxGSM::MpdSipxGSM(int payloadType)
: MpDecoderBase(payloadType, &smCodecInfo)
, mpGsmState(NULL)
{
}

MpdSipxGSM::~MpdSipxGSM()
{
   freeDecode();
}

OsStatus MpdSipxGSM::initDecode()
{
   if (mpGsmState == NULL) 
      mpGsmState = gsm_create();

   return OS_SUCCESS;
}

OsStatus MpdSipxGSM::freeDecode(void)
{
   if (mpGsmState != NULL) 
   {
      gsm_destroy(mpGsmState);
      mpGsmState = NULL;
   }

   return OS_SUCCESS;
}

int MpdSipxGSM::decode(const MpRtpBufPtr &pPacket,
                       unsigned decodedBufferLength,
                       MpAudioSample *samplesBuffer,
                       UtlBoolean bIsPLCFrame)
{
   if (!pPacket.isValid())
      return 0;

   unsigned payloadSize = pPacket->getPayloadSize();
   unsigned maxPayloadSize = smCodecInfo.getMaxPacketBits()/8;
   // do not accept frames longer than 20ms from RTP to protect against buffer overflow
   assert(payloadSize <= maxPayloadSize);
   if (payloadSize > maxPayloadSize || payloadSize <= 1)
   {
      return 0;
   }

   gsm_decode(mpGsmState, (gsm_byte*)pPacket->getDataPtr(), (gsm_signal*)samplesBuffer);
   return smCodecInfo.getNumSamplesPerFrame();
}

#endif /* HAVE_GSM ] */
