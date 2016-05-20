//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2008-2009 Jaroslav Libak.  All rights reserved.
// Licensed under the LGPL license.
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifdef HAVE_GSM /* [ */

#include "assert.h"
// APPLICATION INCLUDES
#include "mp/MpeSipxGSM.h"
#include "mp/NetInTask.h"  
extern "C" {
#include <gsm.h>
}


const MpCodecInfo MpeSipxGSM::smCodecInfo(
         SdpCodec::SDP_CODEC_GSM,    // codecType
         "GSM 6.10",                 // codecVersion
         8000,                       // samplingRate
         16,                          // numBitsPerSample
         1,                          // numChannels
         13200,                      // bitRate. It doesn't matter right now.
         33*8,                       // minPacketBits
         33*8,                       // maxPacketBits
         160);                       // numSamplesPerFrame


MpeSipxGSM::MpeSipxGSM(int payloadType)
: MpEncoderBase(payloadType, &smCodecInfo)
, mpGsmState(NULL)
, mBufferLoad(0)
{
   assert(SdpCodec::SDP_CODEC_GSM == payloadType);
}

MpeSipxGSM::~MpeSipxGSM()
{
   freeEncode();
}

OsStatus MpeSipxGSM::initEncode(void)
{
   assert(NULL == mpGsmState);
   mpGsmState = gsm_create();

   return OS_SUCCESS;
}

OsStatus MpeSipxGSM::freeEncode(void)
{
   if (mpGsmState)
   {
      gsm_destroy(mpGsmState);
      mpGsmState = NULL;
   }
   return OS_SUCCESS;
}

OsStatus MpeSipxGSM::encode(const MpAudioSample* pAudioSamples,
                            const int numSamples,
                            int& rSamplesConsumed,
                            unsigned char* pCodeBuf,
                            const int bytesLeft,
                            int& rSizeInBytes,
                            UtlBoolean& sendNow,
                            MpSpeechType& speechType)
{
   if (speechType == MP_SPEECH_SILENT && ms_bEnableVAD && mBufferLoad == 0)
   {
      // VAD must be enabled, do DTX
      rSamplesConsumed = numSamples;
      rSizeInBytes = 0;
      sendNow = TRUE; // sends any unsent frames now
      return OS_SUCCESS;
   }

   int size = 0;   
   
   assert(numSamples == 80);
   memcpy(&mpBuffer[mBufferLoad], pAudioSamples, sizeof(MpAudioSample)*numSamples);
   mBufferLoad = mBufferLoad+numSamples;
   assert(mBufferLoad <= 160);

   // Check for necessary number of samples
   if (mBufferLoad == 160)
   {
      size = 33;
      gsm_encode(mpGsmState, (gsm_signal*)mpBuffer, (gsm_byte*)pCodeBuf);
      mBufferLoad = 0;
      sendNow = true;
   } else {
      sendNow = false;
   }

   rSamplesConsumed = numSamples;
   rSizeInBytes = size;
   return OS_SUCCESS;
}

#endif /* HAVE_GSM ] */
