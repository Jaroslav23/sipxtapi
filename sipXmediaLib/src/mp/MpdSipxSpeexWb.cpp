//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// Copyright (C) 2006-2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 Hector Izquierdo Seliva. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// Copyright (C) 2008-2009 Jaroslav Libak.  All rights reserved.
// Licensed under the LGPL license.
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifdef HAVE_SPEEX /* [ */

// APPLICATION INCLUDES
#include "mp/MpdSipxSpeexWb.h"
#include "os/OsSysLog.h"
#include "os/OsSysLogFacilities.h"


const MpCodecInfo MpdSipxSpeexWb::smCodecInfo(
         SdpCodec::SDP_CODEC_SPEEX_WB_42,    // codecType
         "Speex codec",                // codecVersion
         16000,                        // samplingRate
         16,                            // numBitsPerSample (not used)
         1,                            // numChannels
         42400,                        // bitRate
         1*8,                          // minPacketBits
         106*8,                         // maxPacketBits
         320);                          // numSamplesPerFrame - 20ms frames

MpdSipxSpeexWb::MpdSipxSpeexWb(int payloadType)
: MpDecoderBase(payloadType, &smCodecInfo)
, mpDecoderState(NULL)
, mDecbits()
, mNumSamplesPerFrame(0)
{   
}

MpdSipxSpeexWb::~MpdSipxSpeexWb()
{
   freeDecode();
}

OsStatus MpdSipxSpeexWb::initDecode()
{
   if (mpDecoderState == NULL)
   {
      int tmp;
   
      // Init decoder
      mpDecoderState = speex_decoder_init(speex_lib_get_mode(SPEEX_MODEID_WB));   

      // It makes the decoded speech deviate further from the original,
      // but it sounds subjectively better.
      tmp = 1;
      speex_decoder_ctl(mpDecoderState, SPEEX_SET_ENH, &tmp);

      // Get number of samples in one frame
      speex_decoder_ctl(mpDecoderState, SPEEX_GET_FRAME_SIZE, &mNumSamplesPerFrame);

      speex_bits_init(&mDecbits);
   }

   return OS_SUCCESS;
}

OsStatus MpdSipxSpeexWb::freeDecode(void)
{
   if (mpDecoderState)
   {
      speex_decoder_destroy(mpDecoderState);
      mpDecoderState = NULL;

      speex_bits_destroy(&mDecbits);
   }

   return OS_SUCCESS;
}

int MpdSipxSpeexWb::decode(const MpRtpBufPtr &pPacket,
                           unsigned decodedBufferLength,
                           MpAudioSample *samplesBuffer,
                           UtlBoolean bIsPLCFrame)
{
   if (!pPacket.isValid())
      return 0;

   unsigned payloadSize = pPacket->getPayloadSize();
   unsigned maxPayloadSize = smCodecInfo.getMaxPacketBits()/8;

   assert(payloadSize <= maxPayloadSize);
   if (payloadSize > maxPayloadSize || payloadSize <= 1)
   {
      return 0;
   }

   if (!bIsPLCFrame)
   {
      // Prepare data for Speex decoder
      speex_bits_read_from(&mDecbits, (char*)pPacket->getDataPtr(), pPacket->getPayloadSize());
   }

   // Decode frame
   speex_decode_int(mpDecoderState,
      bIsPLCFrame ? NULL : &mDecbits,
      (spx_int16_t*)samplesBuffer);   

   return mNumSamplesPerFrame;
}

#endif /* HAVE_SPEEX ] */
