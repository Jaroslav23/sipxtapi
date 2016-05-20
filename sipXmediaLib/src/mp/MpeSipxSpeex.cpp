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
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifdef HAVE_SPEEX /* [ */

#define TEST_PRINT
#include "assert.h"
// APPLICATION INCLUDES
#include "mp/MpeSipxSpeex.h"
#include "mp/NetInTask.h"  // For CODEC_TYPE_SPEEX_* definitions

#include "os/OsSysLog.h"
#include "os/OsSysLogFacilities.h"



const MpCodecInfo MpeSipxSpeex::smCodecInfo(
         SdpCodec::SDP_CODEC_SPEEX_24,    // codecType
         "Speex",                      // codecVersion
         8000,                         // samplingRate
         16,                            // numBitsPerSample
         1,                            // numChannels
         24600,                         // bitRate. It doesn't matter right now.
         1*8,                           // minPacketBits
         63*8,                         // maxPacketBits
         160);                         // numSamplesPerFrame


MpeSipxSpeex::MpeSipxSpeex(int payloadType, int mode)
: MpEncoderBase(payloadType, &smCodecInfo)
, mpEncoderState(NULL)
, mSampleRate(8000)        // Sample rate of 8000Hz. We'll stick with NB for now.
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
   case 2:
      // Use preprocess so the voice is as clear as possible,
      // because the bitrate is very low.
      mDoPreprocess = true; 
      break;

   case 3:
   case 4:
   case 5:
   case 6:
   case 7:
      // Nothing to do.. it's ok.
      break;

   default:
      // If not supported mode selected, use default
      mMode = 3;  // 8,000 bps
   }

//   mDoVad = 1; // Voice activity detection enabled
//   mDoDtx = 1; // Discontinuous transmission
//   mDoVbr = 1; // VBR (not used at the moment)
   
}

MpeSipxSpeex::~MpeSipxSpeex()
{
   freeEncode();
}

OsStatus MpeSipxSpeex::initEncode(void)
{
   mpEncoderState = speex_encoder_init(speex_lib_get_mode(SPEEX_MODEID_NB));
   speex_encoder_ctl(mpEncoderState, SPEEX_SET_MODE,&mMode);
   speex_encoder_ctl(mpEncoderState, SPEEX_SET_SAMPLING_RATE, &mSampleRate);

   // Enable wanted extensions.
   speex_encoder_ctl(mpEncoderState, SPEEX_SET_VAD, &mDoVad);
   speex_encoder_ctl(mpEncoderState, SPEEX_SET_DTX, &mDoDtx);
   speex_encoder_ctl(mpEncoderState, SPEEX_SET_VBR, &mDoVbr);

   speex_bits_init(&mBits);

   if(mDoPreprocess)
   {
      mpPreprocessState = speex_preprocess_state_init(getInfo()->getNumSamplesPerFrame(), mSampleRate);
      speex_preprocess_ctl(mpPreprocessState, SPEEX_PREPROCESS_SET_DENOISE, &mDoDenoise);
      speex_preprocess_ctl(mpPreprocessState, SPEEX_PREPROCESS_SET_AGC, &mDoAgc);
   }   

   return OS_SUCCESS;
}

OsStatus MpeSipxSpeex::freeEncode(void)
{
   if (mpEncoderState)
   {
      speex_encoder_destroy(mpEncoderState);
      mpEncoderState = NULL;
      speex_bits_destroy(&mBits);
   }

   return OS_SUCCESS;
}



OsStatus MpeSipxSpeex::encode(const MpAudioSample* pAudioSamples,
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
   mBufferLoad = mBufferLoad+numSamples;
   assert(mBufferLoad <= 160);

   // Check for necessary number of samples
   if(mBufferLoad == 160)
   {
      speex_bits_reset(&mBits);

      // We don't have echo data, but it should be possible to use the
      // Speex echo cancelator in sipxtapi.
      if(mDoPreprocess)
      {
         speex_preprocess(mpPreprocessState, mpBuffer, NULL);
      }

      speex_encode_int(mpEncoderState, mpBuffer, &mBits);

      // Copy to the byte buffer
      size = speex_bits_write(&mBits,(char*)pCodeBuf, bytesLeft);      

      // Reset the buffer count.
      mBufferLoad = 0;

      if (size>0) {
         sendNow = true;
      }
   }
   else
   {
      sendNow = FALSE;
   }

   rSamplesConsumed = numSamples;
   rSizeInBytes = size;
   
   return OS_SUCCESS;
}

#undef TEST_PRINT

#endif /* HAVE_SPEEX ] */
