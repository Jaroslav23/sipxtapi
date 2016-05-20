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
#include "mp/MpCodecFactory.h"
#include "os/OsSysLog.h"

// all encoder child classes
#include "mp/MpePtAVT.h"
#include "mp/MpeSipxPcma.h"
#include "mp/MpeSipxPcmu.h"

#ifdef HAVE_SPEEX // [
#include "mp/MpeSipxSpeex.h"
#ifdef ENABLE_WIDEBAND_AUDIO
#include "mp/MpeSipxSpeexWb.h"
#include "mp/MpeSipxSpeexUWb.h"
#endif
#endif // HAVE_SPEEX ]

#ifdef HAVE_INTEL_IPP // [
#ifdef ENABLE_WIDEBAND_AUDIO
#include "mp/MpeIPPG7221.h"
#include "mp/MpeIPPG7291.h"
#include "mp/MpeIPPGAmrWb.h"
#endif
#include "mp/MpeIPPGAmr.h"
#include "mp/MpeIPPG728.h"
#include "mp/MpeIPPG729.h"
#include "mp/MpeIPPG729i.h"
#include "mp/MpeIPPG7231.h"
#include <mp/MpeIPPGSM.h>
#include <mp/MpeIPPPcmu.h>
#include <mp/MpeIPPPcma.h>
#endif // HAVE_IPP ]

// All decoder child classes
#include "mp/MpdPtAVT.h"
#include "mp/MpdSipxPcma.h"
#include "mp/MpdSipxPcmu.h"

#ifdef HAVE_SPEEX // [
#include "mp/MpdSipxSpeex.h"
#ifdef ENABLE_WIDEBAND_AUDIO
#include "mp/MpdSipxSpeexWb.h"
#include "mp/MpdSipxSpeexUWb.h"
#endif
#endif // HAVE_SPEEX ]

#ifdef ENABLE_WIDEBAND_AUDIO
#include "mp/MpdSipxL16.h"
#include "mp/MpeSipxL16.h"
#endif

#ifdef HAVE_SPAN_DSP // [
#include "mp/MpdSipxG726.h"
#include "mp/MpeSipxG726.h"
#ifdef ENABLE_WIDEBAND_AUDIO
#include "mp/MpdSipxG722.h"
#include "mp/MpeSipxG722.h"
#endif // ENABLE_WIDEBAND_AUDIO ]
#endif // HAVE_SPAN_DSP ]

#ifdef HAVE_GSM // [
#include "mp/MpdSipxGSM.h"
#include "mp/MpeSipxGSM.h"
#endif // HAVE_GSM ]

#ifdef HAVE_ILBC // [
#include "mp/MpdSipxILBC.h"
#include "mp/MpeSipxILBC.h"
#endif // HAVE_ILBC ]

#ifdef HAVE_INTEL_IPP // [
#ifdef ENABLE_WIDEBAND_AUDIO
#include "mp/MpdIPPG7221.h"
#include "mp/MpdIPPG7291.h"
#include "mp/MpdIPPGAmrWb.h"
#endif
#include "mp/MpdIPPGAmr.h"
#include "mp/MpdIPPG728.h"
#include "mp/MpdIPPG729.h"
#include "mp/MpdIPPG729i.h"
#include "mp/MpdIPPG7231.h"
#include <mp/MpdIPPGSM.h>
#include <mp/MpdIPPPcma.h>
#include <mp/MpdIPPPcmu.h>
#endif // HAVE_IPP ]

MpCodecFactory MpCodecFactory::sInstance;

/* ============================ CREATORS ================================== */

MpCodecFactory* MpCodecFactory::getMpCodecFactory(void)
{
   return &sInstance;
}

MpCodecFactory::MpCodecFactory(void)
{
}

//:Destructor
MpCodecFactory::~MpCodecFactory()
{
}

/* ============================ MANIPULATORS ============================== */


// Returns a new instance of a decoder of the indicated type
// param: internalCodecId - (in) codec type identifier
// param: payloadType - (in) RTP payload type associated with this decoder
// param: rpDecoder - (out) Reference to a pointer to the new decoder object
OsStatus MpCodecFactory::createDecoder(const SdpCodec& pSdpCodec, MpDecoderBase*& rpDecoder)
{
   SdpCodec::SdpCodecTypes internalCodecId = pSdpCodec.getCodecType();
   int payloadFormat = pSdpCodec.getCodecPayloadId();
   int frameLengthMs = pSdpCodec.getPacketLength() / 1000; // frame length in ms

   rpDecoder=NULL;

   switch (internalCodecId)
   {
   case (SdpCodec::SDP_CODEC_TONES):
      rpDecoder = new MpdPtAVT(payloadFormat);
      break;
#if !defined(PREFER_INTEL_IPP_CODECS)
   case (SdpCodec::SDP_CODEC_PCMA):
      rpDecoder = new MpdSipxPcma(payloadFormat);
      break;
   case (SdpCodec::SDP_CODEC_PCMU):
      rpDecoder = new MpdSipxPcmu(payloadFormat);
      break;
#endif
#ifdef HAVE_SPEEX // [
   case (SdpCodec::SDP_CODEC_SPEEX_5):
   case (SdpCodec::SDP_CODEC_SPEEX_8):
   case (SdpCodec::SDP_CODEC_SPEEX_11):
   case (SdpCodec::SDP_CODEC_SPEEX_15):
   case (SdpCodec::SDP_CODEC_SPEEX_18):
   case (SdpCodec::SDP_CODEC_SPEEX_24):
      rpDecoder = new MpdSipxSpeex(payloadFormat);
      break;
#ifdef ENABLE_WIDEBAND_AUDIO
   case (SdpCodec::SDP_CODEC_SPEEX_WB_9):
   case (SdpCodec::SDP_CODEC_SPEEX_WB_12):
   case (SdpCodec::SDP_CODEC_SPEEX_WB_16):
   case (SdpCodec::SDP_CODEC_SPEEX_WB_20):
   case (SdpCodec::SDP_CODEC_SPEEX_WB_23):
   case (SdpCodec::SDP_CODEC_SPEEX_WB_27):
   case (SdpCodec::SDP_CODEC_SPEEX_WB_34):
   case (SdpCodec::SDP_CODEC_SPEEX_WB_42):
      rpDecoder = new MpdSipxSpeexWb(payloadFormat);
      break;
   case (SdpCodec::SDP_CODEC_SPEEX_UWB_11):
   case (SdpCodec::SDP_CODEC_SPEEX_UWB_14):
   case (SdpCodec::SDP_CODEC_SPEEX_UWB_18):
   case (SdpCodec::SDP_CODEC_SPEEX_UWB_22):
   case (SdpCodec::SDP_CODEC_SPEEX_UWB_25):
   case (SdpCodec::SDP_CODEC_SPEEX_UWB_29):
   case (SdpCodec::SDP_CODEC_SPEEX_UWB_36):
   case (SdpCodec::SDP_CODEC_SPEEX_UWB_44):
      rpDecoder = new MpdSipxSpeexUWb(payloadFormat);
      break;
#endif // ENABLE_WIDEBAND_AUDIO ]
#endif // HAVE_SPEEX ]
#ifdef ENABLE_WIDEBAND_AUDIO
      // L16 codecs
   case (SdpCodec::SDP_CODEC_L16_8000_MONO):
      rpDecoder = new MpdSipxL16(payloadFormat, 8000);
      break;
   case (SdpCodec::SDP_CODEC_L16_11025_MONO):
      rpDecoder = new MpdSipxL16(payloadFormat, 11025);
      break;
   case (SdpCodec::SDP_CODEC_L16_16000_MONO):
      rpDecoder = new MpdSipxL16(payloadFormat, 16000);
      break;
   case (SdpCodec::SDP_CODEC_L16_22050_MONO):
      rpDecoder = new MpdSipxL16(payloadFormat, 22050);
      break;
   case (SdpCodec::SDP_CODEC_L16_24000_MONO):
      rpDecoder = new MpdSipxL16(payloadFormat, 24000);
      break;
   case (SdpCodec::SDP_CODEC_L16_32000_MONO):
      rpDecoder = new MpdSipxL16(payloadFormat, 32000);
      break;
   case (SdpCodec::SDP_CODEC_L16_44100_MONO):
      rpDecoder = new MpdSipxL16(payloadFormat, 44100);
      break;
   case (SdpCodec::SDP_CODEC_L16_48000_MONO):
      rpDecoder = new MpdSipxL16(payloadFormat, 48000);
      break;
#endif
#ifdef HAVE_SPAN_DSP // [
   case (SdpCodec::SDP_CODEC_G726_16):
      rpDecoder = new MpdSipxG726(payloadFormat, MpdSipxG726::BITRATE_16);
      break;
   case (SdpCodec::SDP_CODEC_G726_24):
      rpDecoder = new MpdSipxG726(payloadFormat, MpdSipxG726::BITRATE_24);
      break;
   case (SdpCodec::SDP_CODEC_G726_32):
      rpDecoder = new MpdSipxG726(payloadFormat, MpdSipxG726::BITRATE_32);
      break;
   case (SdpCodec::SDP_CODEC_G726_40):
      rpDecoder = new MpdSipxG726(payloadFormat, MpdSipxG726::BITRATE_40);
      break;
#ifdef ENABLE_WIDEBAND_AUDIO
   case (SdpCodec::SDP_CODEC_G722):
      rpDecoder = new MpdSipxG722(payloadFormat);
      break;
#endif // ENABLE_WIDEBAND_AUDIO ]
#endif // HAVE_SPAN_DSP ]
#if defined(HAVE_GSM) && !defined(PREFER_INTEL_IPP_CODECS) // [
   case (SdpCodec::SDP_CODEC_GSM):
      rpDecoder = new MpdSipxGSM(payloadFormat);
      break;
#endif // HAVE_GSM ]
#ifdef HAVE_ILBC // [
   case (SdpCodec::SDP_CODEC_ILBC_30MS):
      rpDecoder = new MpdSipxILBC(payloadFormat, 30);
      break;
   case (SdpCodec::SDP_CODEC_ILBC_20MS):
      {
         int ilbcFrameLength = 20;
         if (frameLengthMs == 20 || frameLengthMs == 30)
         {
            ilbcFrameLength = frameLengthMs;
         }
         rpDecoder = new MpdSipxILBC(payloadFormat, ilbcFrameLength);
      }
      break;
#endif // HAVE_ILBC ]
#ifdef HAVE_INTEL_IPP // [
#ifdef ENABLE_WIDEBAND_AUDIO
   case (SdpCodec::SDP_CODEC_G7221_16): 
      rpDecoder = new MpdIPPG7221(payloadFormat, 16000);
      break;
   case (SdpCodec::SDP_CODEC_G7221_24): 
      rpDecoder = new MpdIPPG7221(payloadFormat, 24000);
      break;
   case (SdpCodec::SDP_CODEC_G7221_32): 
      rpDecoder = new MpdIPPG7221(payloadFormat, 32000);
      break;
   case (SdpCodec::SDP_CODEC_AMR_WB_12650): 
      rpDecoder = new MpdIPPGAmrWb(payloadFormat, 12650, FALSE);
      break;
   case (SdpCodec::SDP_CODEC_AMR_WB_23850): 
      rpDecoder = new MpdIPPGAmrWb(payloadFormat, 23850, TRUE);
      break;
   case (SdpCodec::SDP_CODEC_G7291_8000): 
   case (SdpCodec::SDP_CODEC_G7291_12000): 
   case (SdpCodec::SDP_CODEC_G7291_14000): 
   case (SdpCodec::SDP_CODEC_G7291_16000): 
   case (SdpCodec::SDP_CODEC_G7291_18000): 
   case (SdpCodec::SDP_CODEC_G7291_20000): 
   case (SdpCodec::SDP_CODEC_G7291_22000): 
   case (SdpCodec::SDP_CODEC_G7291_24000): 
   case (SdpCodec::SDP_CODEC_G7291_26000): 
   case (SdpCodec::SDP_CODEC_G7291_28000): 
   case (SdpCodec::SDP_CODEC_G7291_30000): 
   case (SdpCodec::SDP_CODEC_G7291_32000): 
      rpDecoder = new MpdIPPG7291(payloadFormat);
      break;
#endif
   case (SdpCodec::SDP_CODEC_AMR_4750): 
      rpDecoder = new MpdIPPGAmr(payloadFormat, 4750, FALSE);
      break;
   case (SdpCodec::SDP_CODEC_AMR_10200): 
      rpDecoder = new MpdIPPGAmr(payloadFormat, 10200, TRUE);
      break;
   case (SdpCodec::SDP_CODEC_G723): 
      rpDecoder = new MpdIPPG7231(payloadFormat);
      break;
   case (SdpCodec::SDP_CODEC_G728):
      rpDecoder = new MpdIPPG728(payloadFormat);
      break;
   case (SdpCodec::SDP_CODEC_G729):
      rpDecoder = new MpdIPPG729(payloadFormat);
      break;
   case (SdpCodec::SDP_CODEC_G729D):
      rpDecoder = new MpdIPPG729i(payloadFormat, 6400); // also supports 8000
      break;
   case (SdpCodec::SDP_CODEC_G729E):
      rpDecoder = new MpdIPPG729i(payloadFormat, 11800);
      break;
#ifdef PREFER_INTEL_IPP_CODECS
   case (SdpCodec::SDP_CODEC_PCMA):
      rpDecoder = new MpdIPPPcma(payloadFormat);
      break;
   case (SdpCodec::SDP_CODEC_PCMU):
      rpDecoder = new MpdIPPPcmu(payloadFormat);
      break;
   case (SdpCodec::SDP_CODEC_GSM):
      rpDecoder = new MpdIPPGSM(payloadFormat);
      break;
#endif // PREFER_INTEL_IPP_CODECS ]
#endif // HAVE_IPP ]
   default:
      OsSysLog::add(FAC_MP, PRI_WARNING, 
                    "MpCodecFactory::createDecoder unknown codec type "
                    "internalCodecId = (SdpCodec::SdpCodecTypes) %d, "
                    "payloadType = %d",
                    internalCodecId, payloadFormat);
      assert(FALSE);
      break;
   }

   if (rpDecoder)
   {
      return OS_SUCCESS;
   }

   return OS_INVALID_ARGUMENT;
}

// Returns a new instance of an encoder of the indicated type
// param: internalCodecId - (in) codec type identifier
// param: payloadType - (in) RTP payload type associated with this encoder
// param: rpEncoder - (out) Reference to a pointer to the new encoder object

OsStatus MpCodecFactory::createEncoder(const SdpCodec& pSdpCodec, MpEncoderBase*& rpEncoder)
{
   SdpCodec::SdpCodecTypes internalCodecId = pSdpCodec.getCodecType();
   int payloadFormat = pSdpCodec.getCodecPayloadId();
   int frameLengthMs = pSdpCodec.getPacketLength() / 1000; // frame length in ms

   rpEncoder=NULL;

   switch (internalCodecId)
   {
   case (SdpCodec::SDP_CODEC_TONES):
      rpEncoder = new MpePtAVT(payloadFormat);
      break;
#if !defined(PREFER_INTEL_IPP_CODECS)
   case (SdpCodec::SDP_CODEC_PCMA):
      rpEncoder = new MpeSipxPcma(payloadFormat);
      break;
   case (SdpCodec::SDP_CODEC_PCMU):
      rpEncoder = new MpeSipxPcmu(payloadFormat);
      break;
#endif
#ifdef HAVE_SPEEX // [
   case (SdpCodec::SDP_CODEC_SPEEX_5):
      rpEncoder = new MpeSipxSpeex(payloadFormat, 2);
      break;
   case (SdpCodec::SDP_CODEC_SPEEX_8):
      rpEncoder = new MpeSipxSpeex(payloadFormat, 3);
      break;
   case (SdpCodec::SDP_CODEC_SPEEX_11):
      rpEncoder = new MpeSipxSpeex(payloadFormat, 4);
      break;
   case (SdpCodec::SDP_CODEC_SPEEX_15):
      rpEncoder = new MpeSipxSpeex(payloadFormat, 5);
      break;
   case (SdpCodec::SDP_CODEC_SPEEX_18):
      rpEncoder = new MpeSipxSpeex(payloadFormat, 6);
      break;
   case (SdpCodec::SDP_CODEC_SPEEX_24):
      rpEncoder = new MpeSipxSpeex(payloadFormat, 7);
      break;
#ifdef ENABLE_WIDEBAND_AUDIO
   case (SdpCodec::SDP_CODEC_SPEEX_WB_9):
      rpEncoder = new MpeSipxSpeexWb(payloadFormat, 3);
      break;
   case (SdpCodec::SDP_CODEC_SPEEX_WB_12):
      rpEncoder = new MpeSipxSpeexWb(payloadFormat, 4);
      break;
   case (SdpCodec::SDP_CODEC_SPEEX_WB_16):
      rpEncoder = new MpeSipxSpeexWb(payloadFormat, 5);
      break;
   case (SdpCodec::SDP_CODEC_SPEEX_WB_20):
      rpEncoder = new MpeSipxSpeexWb(payloadFormat, 6);
      break;
   case (SdpCodec::SDP_CODEC_SPEEX_WB_23):
      rpEncoder = new MpeSipxSpeexWb(payloadFormat, 7);
      break;
   case (SdpCodec::SDP_CODEC_SPEEX_WB_27):
      rpEncoder = new MpeSipxSpeexWb(payloadFormat, 8);
      break;
   case (SdpCodec::SDP_CODEC_SPEEX_WB_34):
      rpEncoder = new MpeSipxSpeexWb(payloadFormat, 9);
      break;
   case (SdpCodec::SDP_CODEC_SPEEX_WB_42):
      rpEncoder = new MpeSipxSpeexWb(payloadFormat, 10);
      break;
   // speex ultra wide band
   case (SdpCodec::SDP_CODEC_SPEEX_UWB_11):
      rpEncoder = new MpeSipxSpeexUWb(payloadFormat, 3);
      break;
   case (SdpCodec::SDP_CODEC_SPEEX_UWB_14):
      rpEncoder = new MpeSipxSpeexUWb(payloadFormat, 4);
      break;
   case (SdpCodec::SDP_CODEC_SPEEX_UWB_18):
      rpEncoder = new MpeSipxSpeexUWb(payloadFormat, 5);
      break;
   case (SdpCodec::SDP_CODEC_SPEEX_UWB_22):
      rpEncoder = new MpeSipxSpeexUWb(payloadFormat, 6);
      break;
   case (SdpCodec::SDP_CODEC_SPEEX_UWB_25):
      rpEncoder = new MpeSipxSpeexUWb(payloadFormat, 7);
      break;
   case (SdpCodec::SDP_CODEC_SPEEX_UWB_29):
      rpEncoder = new MpeSipxSpeexUWb(payloadFormat, 8);
      break;
   case (SdpCodec::SDP_CODEC_SPEEX_UWB_36):
      rpEncoder = new MpeSipxSpeexUWb(payloadFormat, 9);
      break;
   case (SdpCodec::SDP_CODEC_SPEEX_UWB_44):
      rpEncoder = new MpeSipxSpeexUWb(payloadFormat, 10);
      break;
#endif // ENABLE_WIDEBAND_AUDIO ]
#endif // HAVE_SPEEX ]
#ifdef ENABLE_WIDEBAND_AUDIO
      // L16 codecs
   case (SdpCodec::SDP_CODEC_L16_8000_MONO):
      rpEncoder = new MpeSipxL16(payloadFormat, 8000);
      break;
   case (SdpCodec::SDP_CODEC_L16_11025_MONO):
      rpEncoder = new MpeSipxL16(payloadFormat, 11025);
      break;
   case (SdpCodec::SDP_CODEC_L16_16000_MONO):
      rpEncoder = new MpeSipxL16(payloadFormat, 16000);
      break;
   case (SdpCodec::SDP_CODEC_L16_22050_MONO):
      rpEncoder = new MpeSipxL16(payloadFormat, 22050);
      break;
   case (SdpCodec::SDP_CODEC_L16_24000_MONO):
      rpEncoder = new MpeSipxL16(payloadFormat, 24000);
      break;
   case (SdpCodec::SDP_CODEC_L16_32000_MONO):
      rpEncoder = new MpeSipxL16(payloadFormat, 32000);
      break;
   case (SdpCodec::SDP_CODEC_L16_44100_MONO):
      rpEncoder = new MpeSipxL16(payloadFormat, 44100);
      break;
   case (SdpCodec::SDP_CODEC_L16_48000_MONO):
      rpEncoder = new MpeSipxL16(payloadFormat, 48000);
      break;
#endif
#ifdef HAVE_SPAN_DSP // [
   case (SdpCodec::SDP_CODEC_G726_16):
      rpEncoder = new MpeSipxG726(payloadFormat, MpeSipxG726::BITRATE_16);
      break;
   case (SdpCodec::SDP_CODEC_G726_24):
      rpEncoder = new MpeSipxG726(payloadFormat, MpeSipxG726::BITRATE_24);
      break;
   case (SdpCodec::SDP_CODEC_G726_32):
      rpEncoder = new MpeSipxG726(payloadFormat, MpeSipxG726::BITRATE_32);
      break;
   case (SdpCodec::SDP_CODEC_G726_40):
      rpEncoder = new MpeSipxG726(payloadFormat, MpeSipxG726::BITRATE_40);
      break;
#ifdef ENABLE_WIDEBAND_AUDIO
   case (SdpCodec::SDP_CODEC_G722):
      rpEncoder = new MpeSipxG722(payloadFormat);
      break;
#endif // ENABLE_WIDEBAND_AUDIO ]
#endif // HAVE_SPAN_DSP ]
#if defined(HAVE_GSM) && !defined(PREFER_INTEL_IPP_CODECS) // [
   case (SdpCodec::SDP_CODEC_GSM):
      rpEncoder = new MpeSipxGSM(payloadFormat);
      break;
#endif // HAVE_GSM ]
#ifdef HAVE_ILBC // [
   case (SdpCodec::SDP_CODEC_ILBC_30MS):
      rpEncoder = new MpeSipxILBC(payloadFormat, 30);
      break;
   case (SdpCodec::SDP_CODEC_ILBC_20MS):
      {
         int ilbcFrameLength = 20;
         if (frameLengthMs == 20 || frameLengthMs == 30)
         {
            ilbcFrameLength = frameLengthMs;
         }
         rpEncoder = new MpeSipxILBC(payloadFormat, ilbcFrameLength);
      }
      break;
#endif // HAVE_ILBC ]
#ifdef HAVE_INTEL_IPP // [
#ifdef ENABLE_WIDEBAND_AUDIO
   case (SdpCodec::SDP_CODEC_G7221_16): 
      rpEncoder = new MpeIPPG7221(payloadFormat, 16000);
      break;
   case (SdpCodec::SDP_CODEC_G7221_24): 
      rpEncoder = new MpeIPPG7221(payloadFormat, 24000);
      break;
   case (SdpCodec::SDP_CODEC_G7221_32): 
      rpEncoder = new MpeIPPG7221(payloadFormat, 32000);
      break;
   case (SdpCodec::SDP_CODEC_AMR_WB_12650): 
      rpEncoder = new MpeIPPGAmrWb(payloadFormat, 12650, FALSE);
      break;
   case (SdpCodec::SDP_CODEC_AMR_WB_23850): 
      rpEncoder = new MpeIPPGAmrWb(payloadFormat, 23850, TRUE);
      break;
   case (SdpCodec::SDP_CODEC_G7291_8000):
      rpEncoder = new MpeIPPG7291(payloadFormat, 8000);
      break;
   case (SdpCodec::SDP_CODEC_G7291_12000):
      rpEncoder = new MpeIPPG7291(payloadFormat, 12000);
      break;
   case (SdpCodec::SDP_CODEC_G7291_14000):
      rpEncoder = new MpeIPPG7291(payloadFormat, 14000);
      break;
   case (SdpCodec::SDP_CODEC_G7291_16000):
      rpEncoder = new MpeIPPG7291(payloadFormat, 16000);
      break;
   case (SdpCodec::SDP_CODEC_G7291_18000):
      rpEncoder = new MpeIPPG7291(payloadFormat, 18000);
      break;
   case (SdpCodec::SDP_CODEC_G7291_20000):
      rpEncoder = new MpeIPPG7291(payloadFormat, 20000);
      break;
   case (SdpCodec::SDP_CODEC_G7291_22000):
      rpEncoder = new MpeIPPG7291(payloadFormat, 22000);
      break;
   case (SdpCodec::SDP_CODEC_G7291_24000):
      rpEncoder = new MpeIPPG7291(payloadFormat, 24000);
      break;
   case (SdpCodec::SDP_CODEC_G7291_26000):
      rpEncoder = new MpeIPPG7291(payloadFormat, 26000);
      break;
   case (SdpCodec::SDP_CODEC_G7291_28000):
      rpEncoder = new MpeIPPG7291(payloadFormat, 28000);
      break;
   case (SdpCodec::SDP_CODEC_G7291_30000):
      rpEncoder = new MpeIPPG7291(payloadFormat, 30000);
      break;
   case (SdpCodec::SDP_CODEC_G7291_32000):
      rpEncoder = new MpeIPPG7291(payloadFormat, 32000);
      break;
#endif
   case (SdpCodec::SDP_CODEC_AMR_4750): 
      rpEncoder = new MpeIPPGAmr(payloadFormat, 4750, FALSE);
      break;
   case (SdpCodec::SDP_CODEC_AMR_10200): 
      rpEncoder = new MpeIPPGAmr(payloadFormat, 10200, TRUE);
      break;
   case (SdpCodec::SDP_CODEC_G723):
      rpEncoder = new MpeIPPG7231(payloadFormat);
      break;
   case (SdpCodec::SDP_CODEC_G728):
      rpEncoder = new MpeIPPG728(payloadFormat);
      break;
   case (SdpCodec::SDP_CODEC_G729):
      rpEncoder = new MpeIPPG729(payloadFormat);
      break;
   case (SdpCodec::SDP_CODEC_G729D):
      rpEncoder = new MpeIPPG729i(payloadFormat, 6400);
      break;
   case (SdpCodec::SDP_CODEC_G729E):
      rpEncoder = new MpeIPPG729i(payloadFormat, 11800);
      break;
#ifdef PREFER_INTEL_IPP_CODECS
   case (SdpCodec::SDP_CODEC_PCMA):
      rpEncoder = new MpeIPPPcma(payloadFormat);
      break;
   case (SdpCodec::SDP_CODEC_PCMU):
      rpEncoder = new MpeIPPPcmu(payloadFormat);
      break;
   case (SdpCodec::SDP_CODEC_GSM):
      rpEncoder = new MpeIPPGSM(payloadFormat);
      break;
#endif // PREFER_INTEL_IPP_CODECS ]
#endif // HAVE_IPP ]

   default:
      OsSysLog::add(FAC_MP, PRI_WARNING, 
                    "MpCodecFactory::createEncoder unknown codec type "
                    "internalCodecId = (SdpCodec::SdpCodecTypes) %d, "
                    "payloadType = %d",
                    internalCodecId, payloadFormat);
      assert(FALSE);
      break;
   }

   if (rpEncoder)
   {
      return OS_SUCCESS;
   }

   return OS_INVALID_ARGUMENT;
}
