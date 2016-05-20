//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// Copyright (C) 2006 SIPfoundry Inc. 
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

#define TEST_PRINT
#include "assert.h"
// APPLICATION INCLUDES
#include "mp/MpeSipxSpeexUWb.h"
#include "mp/NetInTask.h"  // For CODEC_TYPE_SPEEX_* definitions

#include "os/OsSysLog.h"
#include "os/OsSysLogFacilities.h"

const MpCodecInfo MpeSipxSpeexUWb::smCodecInfo(
         SdpCodec::SDP_CODEC_SPEEX_UWB_44,    // codecType
         "Speex",                      // codecVersion
         32000,                        // samplingRate
         16,                            // numBitsPerSample
         1,                            // numChannels
         44000,                        // bitRate. It doesn't matter right now.
         1*8,                         // minPacketBits
         110*8,                         // maxPacketBits
         640);                         // numSamplesPerFrame - 20ms frames

MpeSipxSpeexUWb::MpeSipxSpeexUWb(int payloadType, int mode)
: MpEncoderBase(payloadType, &smCodecInfo)
, mpEncoderState(NULL)
, mSampleRate(32000)// uwb sample rate
, mMode(mode)
, mDoVad(0)
, mDoDtx(0)
, mDoVbr(0)
, mBufferLoad(0)
, mDoPreprocess(false)
, mpPreprocessState(NULL)
, mDoDenoise(0)
, mDoAgc(0)
{
   switch(mMode)
   {
   case 0:
   case 1:
   case 2:
   case 3:
      // Use preprocess so the voice is as clear as possible,
      // because the bitrate is very low.
      mDoPreprocess = true; 
      break;
   case 4:
   case 5:
   case 6:
   case 7:
   case 8:
   case 9:
   case 10:
      // Nothing to do.. it's ok.
      break;
   default:
      // If not supported mode selected, use default
      mMode = 7;
   }

//   mDoVad = 1; // Voice activity detection enabled
//   mDoDtx = 1; // Discontinuous transmission
//   mDoVbr = 1; // VBR (not used at the moment)
}

MpeSipxSpeexUWb::~MpeSipxSpeexUWb()
{
   freeEncode();
}

OsStatus MpeSipxSpeexUWb::initEncode(void)
{
   mpEncoderState = speex_encoder_init(speex_lib_get_mode(SPEEX_MODEID_UWB));
   speex_encoder_ctl(mpEncoderState, SPEEX_SET_MODE, &mMode);
   speex_encoder_ctl(mpEncoderState, SPEEX_SET_SAMPLING_RATE, &mSampleRate);

   // Enable wanted extensions.
   speex_encoder_ctl(mpEncoderState, SPEEX_SET_VAD, &mDoVad);
   speex_encoder_ctl(mpEncoderState, SPEEX_SET_DTX, &mDoDtx);
   speex_encoder_ctl(mpEncoderState, SPEEX_SET_VBR, &mDoVbr);

   speex_bits_init(&mBits);

   if(mDoPreprocess)
   {
      mpPreprocessState = speex_preprocess_state_init(getInfo()->getNumSamplesPerFrame(), mSampleRate);
      speex_preprocess_ctl(mpPreprocessState, SPEEX_PREPROCESS_SET_DENOISE,
                          &mDoDenoise);
      speex_preprocess_ctl(mpPreprocessState, SPEEX_PREPROCESS_SET_AGC, &mDoAgc);
   }

   return OS_SUCCESS;
}

OsStatus MpeSipxSpeexUWb::freeEncode(void)
{
   if (mpEncoderState)
   {
      speex_encoder_destroy(mpEncoderState);
      mpEncoderState = NULL;
      speex_bits_destroy(&mBits);
   }

   return OS_SUCCESS;
}

OsStatus MpeSipxSpeexUWb::encode(const MpAudioSample* pAudioSamples,
                              const int numSamples,
                              int& rSamplesConsumed,
                              unsigned char* pCodeBuf,
                              const int bytesLeft,
                              int& rSizeInBytes,
                              UtlBoolean& sendNow,
                              MpSpeechType& speechType)
{
   int size = 0;

   if (speechType == MP_SPEECH_SILENT && ms_bEnableVAD && mBufferLoad == 0)
   {
      // VAD must be enabled, do DTX
      rSamplesConsumed = numSamples;
      rSizeInBytes = 0;
      sendNow = TRUE; // sends any unsent frames now
      return OS_SUCCESS;
   }

   memcpy(&mpBuffer[mBufferLoad], pAudioSamples, sizeof(MpAudioSample)*numSamples);
   mBufferLoad = mBufferLoad + numSamples;
   assert(mBufferLoad <= 640);

   // Check for necessary number of samples
   if(mBufferLoad == 640)
   {
      speex_bits_reset(&mBits);

      // We don't have echo data, but it should be possible to use the
      // Speex echo cancelator in sipxtapi.
      if(mDoPreprocess)
         speex_preprocess(mpPreprocessState, mpBuffer, NULL);
      speex_encode_int(mpEncoderState, mpBuffer, &mBits);

      // Copy to the byte buffer
      size = speex_bits_write(&mBits,(char*)pCodeBuf, bytesLeft);      

      // Reset the buffer count.
      mBufferLoad = 0;

      if (size > 0)
      {
         sendNow = true;
      }
   }
   else
   {
      sendNow = false;
   }

   rSamplesConsumed = numSamples;
   rSizeInBytes = size;
   
   return OS_SUCCESS;
}

#undef TEST_PRINT

#endif /* HAVE_SPEEX ] */
