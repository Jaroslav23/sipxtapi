//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2007 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <sdp/SdpCodec.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SdpCodec::SdpCodec(enum SdpCodecTypes sdpCodecType,
                   int payloadId,
                   const UtlString& sCodecName,
                   const UtlString& sDisplayCodecName,
                   const char* mimeType,
                   const char* mimeSubtype,
                   int sampleRate,
                   int preferredPacketLength,
                   int numChannels,
                   const char* formatSpecificData,
                   const int CPUCost,
                   const int BWCost,
                   const int videoFormat,
                   const int videoFmtp)
: mCodecPayloadId(payloadId)
, mMimeType(mimeType)
, mMimeSubtype(mimeSubtype)
, mSampleRate(sampleRate)
, mPacketLength(preferredPacketLength)
, mNumChannels(numChannels)
, mFormatSpecificData(formatSpecificData)
, mCPUCost(CPUCost)
, mBWCost(BWCost)
, mVideoFormat(videoFormat)
, mVideoFmtp(videoFmtp)
, m_sCodecName(sCodecName)
, m_sDisplayCodecName(sDisplayCodecName)
{
   setValue(sdpCodecType);
}

// Copy constructor
SdpCodec::SdpCodec(const SdpCodec& rSdpCodec)
{
    setValue(rSdpCodec.getValue());
    mCodecPayloadId = rSdpCodec.mCodecPayloadId;
    mSampleRate = rSdpCodec.mSampleRate;
    mPacketLength = rSdpCodec.mPacketLength;
    mNumChannels = rSdpCodec.mNumChannels;
    mMimeType = rSdpCodec.mMimeType;
    mMimeSubtype = rSdpCodec.mMimeSubtype;
    mFormatSpecificData = rSdpCodec.mFormatSpecificData;
    mCPUCost  = rSdpCodec.mCPUCost;
    mBWCost  = rSdpCodec.mBWCost;
    mVideoFormat = rSdpCodec.mVideoFormat;
    mVideoFmtp = rSdpCodec.mVideoFmtp;
    mVideoFmtpString = rSdpCodec.mVideoFmtpString;
    m_sCodecName = rSdpCodec.m_sCodecName;
    m_sDisplayCodecName = rSdpCodec.m_sDisplayCodecName;
}

// Destructor
SdpCodec::~SdpCodec()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SdpCodec&
SdpCodec::operator=(const SdpCodec& rhs)
{

   if (this == &rhs)            // handle the assignment to self case
      return *this;
    setValue(rhs.getValue());
    mCodecPayloadId = rhs.mCodecPayloadId;
    mSampleRate = rhs.mSampleRate;
    mPacketLength = rhs.mPacketLength;
    mNumChannels = rhs.mNumChannels;
    mMimeType = rhs.mMimeType;
    mMimeSubtype = rhs.mMimeSubtype;
    mFormatSpecificData = rhs.mFormatSpecificData;
    mCPUCost  = rhs.mCPUCost;
    mBWCost = rhs.mBWCost;
    mVideoFormat = rhs.mVideoFormat;
    mVideoFmtp = rhs.mVideoFmtp;
    mVideoFmtpString = rhs.mVideoFmtpString;
    m_sCodecName = rhs.m_sCodecName;
    m_sDisplayCodecName = rhs.m_sDisplayCodecName;

   return *this;
}

/* ============================ ACCESSORS ================================= */

SdpCodec::SdpCodecTypes SdpCodec::getCodecType() const
{
   return((enum SdpCodecTypes) getValue());
}

int SdpCodec::getCodecPayloadId() const
{
    return(mCodecPayloadId);
}

void SdpCodec::setCodecPayloadId(int formatId)
{
    mCodecPayloadId = formatId;
}

void SdpCodec::getSdpFmtpField(UtlString& formatSpecificData) const
{
    formatSpecificData = mFormatSpecificData;
}

void SdpCodec::getMediaType(UtlString& mimeType) const
{
    mimeType = mMimeType;
}

void SdpCodec::getMimeSubType(UtlString& mimeSubtype) const
{
    mimeSubtype = mMimeSubtype;
}

int SdpCodec::getSampleRate() const // samples per second
{
    return(mSampleRate);
}

int SdpCodec::getVideoFormat() const // samples per second
{
    return(mVideoFormat);
}

int SdpCodec::getPacketLength() const //micro seconds
{
    return(mPacketLength);
}

int SdpCodec::getNumChannels() const
{
    return(mNumChannels);
}

void SdpCodec::setVideoFmtp(const int videoFmtp)
{
    mVideoFmtp = videoFmtp;
}

int SdpCodec::getVideoFmtp() const
{
    return mVideoFmtp;
}

void SdpCodec::setPacketSize(const int packetSize)
{
    mPacketLength = packetSize;
}

void SdpCodec::getVideoFmtpString(UtlString& fmtpString) const
{
    fmtpString = mVideoFmtpString;
}

void SdpCodec::setVideoFmtpString(int videoFmtp)
{
    UtlString tempFmtp(NULL);

    switch (videoFmtp)
    {
    case SDP_VIDEO_FORMAT_SQCIF:
        mVideoFmtpString.append("SQCIF/");
        break;
    case SDP_VIDEO_FORMAT_QCIF:
        mVideoFmtpString.append("QCIF/");
        break;
    case SDP_VIDEO_FORMAT_CIF:
        mVideoFmtpString.append("CIF/");
        break;
    case SDP_VIDEO_FORMAT_QVGA:
        mVideoFmtpString.append("QVGA/");
        break;
    default:
        break;
    }
}

void SdpCodec::clearVideoFmtpString()
{
    mVideoFmtpString = "";
}

void SdpCodec::toString(UtlString& sdpCodecContents) const
{
    char stringBuffer[256];
    SNPRINTF(stringBuffer, sizeof(stringBuffer), "SdpCodec: codecId=%d, payloadId=%d, mime=\'%s/%s\', rate=%d, pktLen=%d, numCh=%d, fmtData=\'%s\'\n",
            getValue(), mCodecPayloadId,
            mMimeType.data(), mMimeSubtype.data(),
            mSampleRate, mPacketLength, mNumChannels,
            mFormatSpecificData.data());
    sdpCodecContents = stringBuffer;
}

// Get the CPU cost for this codec.
int SdpCodec::getCPUCost() const
{
   return mCPUCost;
}

// Get the bandwidth cost for this codec.
int SdpCodec::getBWCost() const
{
   return mBWCost;
}

/* ============================ INQUIRY =================================== */

UtlBoolean SdpCodec::isSameDefinition(const SdpCodec& codec) const
{
    return(mSampleRate == codec.mSampleRate &&
           mNumChannels == codec.mNumChannels &&
           mMimeType.compareTo(codec.mMimeType, UtlString::ignoreCase) == 0 &&
           mMimeSubtype.compareTo(codec.mMimeSubtype, UtlString::ignoreCase) == 0);
}

UtlBoolean SdpCodec::isCodecCompatible(const UtlString& mimeType, 
                                       const UtlString& mimeSubType,
                                       int sampleRate,
                                       int numChannels,
                                       const UtlString& fmtp,
                                       UtlBoolean bStrictMatch) const
{
   // If the mime type matches
   if(mMimeType.compareTo(mimeType, UtlString::ignoreCase) == 0)
   {
      // and if the mime subtype, sample rate, number of channels
      // and fmtp match.
      if ((mMimeSubtype.compareTo(mimeSubType, UtlString::ignoreCase) == 0) &&
         (sampleRate == -1 || mSampleRate == sampleRate) &&
         (numChannels == -1 || mNumChannels == numChannels))
      {
         SdpCodec::SdpCodecTypes codecType = getCodecType();

         if (codecType == SDP_CODEC_AMR_10200 || codecType == SDP_CODEC_AMR_4750)
         {
            bStrictMatch = TRUE; // always use strict match for AMR
         }

         if (bStrictMatch)
         {
            if (codecType == SDP_CODEC_ILBC_20MS) // iLBC with 20ms frame
            {
               if (fmtp.contains("mode=20"))
               {
                  return TRUE;
               }
               else
               {
                  return FALSE; // internally 30ms mode is not compatible with 20ms mode in strict match
               }
            }
            else if (codecType == SDP_CODEC_ILBC_30MS) // iLBC with 30ms frame
            {
               // check if the other codec is not 20ms
               if (!fmtp.contains("mode=20"))
               {
                  return TRUE;
               }
               else
               {
                  return FALSE; // internally 30ms mode is not compatible with 20ms mode in strict match
               }
            }
         }

         if ((bStrictMatch && fmtp.compareTo(mFormatSpecificData) == 0) || !bStrictMatch)
         {
            // we found a match
            return TRUE;
         }
      }
   }

   return FALSE;
}

UtlBoolean SdpCodec::hasDynamicPayloadId() const
{
   if (mCodecPayloadId == -1 || mCodecPayloadId >= SDP_MIN_DYNAMIC_PAYLOAD_ID)
   {
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}

SdpCodec::SdpCodecTypes SdpCodec::getCodecType(const UtlString& shortCodecName)
{
    SdpCodec::SdpCodecTypes retType = SdpCodec::SDP_CODEC_UNKNOWN;
    UtlString compareString(shortCodecName);
    compareString.toUpper();

    if (strcmp(compareString,"TELEPHONE-EVENT") == 0)
       retType = SdpCodec::SDP_CODEC_TONES;
    else if (strcmp(compareString,"PCMU") == 0)
       retType = SdpCodec::SDP_CODEC_PCMU;
    else if (strcmp(compareString,"PCMA") == 0)
       retType = SdpCodec::SDP_CODEC_PCMA;
#ifdef HAVE_INTEL_IPP
    // G.723.1
    else if (strcmp(compareString,"G723.1") == 0)
       retType = SdpCodec::SDP_CODEC_G723;
    // G.728
    else if (strcmp(compareString,"G728") == 0)
       retType = SdpCodec::SDP_CODEC_G728;
    // G.729
    else if (strcmp(compareString,"G729") == 0)
       retType = SdpCodec::SDP_CODEC_G729;
    else if (strcmp(compareString,"G729D") == 0)
       retType = SdpCodec::SDP_CODEC_G729D;
    else if (strcmp(compareString,"G729E") == 0)
       retType = SdpCodec::SDP_CODEC_G729E;
    // amr
    else if (strcmp(compareString,"AMR_4750") == 0)
       retType = SdpCodec::SDP_CODEC_AMR_4750;
    else if (strcmp(compareString,"AMR_10200") == 0)
       retType = SdpCodec::SDP_CODEC_AMR_10200;
#ifdef ENABLE_WIDEBAND_AUDIO
    // G.722.1 wideband codec
    else if (strcmp(compareString,"G722.1_16") == 0)
       retType = SdpCodec::SDP_CODEC_G7221_16;
    else if (strcmp(compareString,"G722.1_24") == 0)
       retType = SdpCodec::SDP_CODEC_G7221_24;
    else if (strcmp(compareString,"G722.1_32") == 0)
       retType = SdpCodec::SDP_CODEC_G7221_32;
    // G.729.1
    else if (strcmp(compareString,"G729.1_8000") == 0)
       retType = SdpCodec::SDP_CODEC_G7291_8000;
    else if (strcmp(compareString,"G729.1_12000") == 0)
       retType = SdpCodec::SDP_CODEC_G7291_12000;
    else if (strcmp(compareString,"G729.1_14000") == 0)
       retType = SdpCodec::SDP_CODEC_G7291_14000;
    else if (strcmp(compareString,"G729.1_16000") == 0)
       retType = SdpCodec::SDP_CODEC_G7291_16000;
    else if (strcmp(compareString,"G729.1_18000") == 0)
       retType = SdpCodec::SDP_CODEC_G7291_18000;
    else if (strcmp(compareString,"G729.1_20000") == 0)
       retType = SdpCodec::SDP_CODEC_G7291_20000;
    else if (strcmp(compareString,"G729.1_22000") == 0)
       retType = SdpCodec::SDP_CODEC_G7291_22000;
    else if (strcmp(compareString,"G729.1_24000") == 0)
       retType = SdpCodec::SDP_CODEC_G7291_24000;
    else if (strcmp(compareString,"G729.1_26000") == 0)
       retType = SdpCodec::SDP_CODEC_G7291_26000;
    else if (strcmp(compareString,"G729.1_28000") == 0)
       retType = SdpCodec::SDP_CODEC_G7291_28000;
    else if (strcmp(compareString,"G729.1_30000") == 0)
       retType = SdpCodec::SDP_CODEC_G7291_30000;
    else if (strcmp(compareString,"G729.1_32000") == 0)
       retType = SdpCodec::SDP_CODEC_G7291_32000;
    // amr wb
    else if (strcmp(compareString,"AMR_WB_12650") == 0)
       retType = SdpCodec::SDP_CODEC_AMR_WB_12650;
    else if (strcmp(compareString,"AMR_WB_23850") == 0)
       retType = SdpCodec::SDP_CODEC_AMR_WB_23850;
#endif // ENABLE_WIDEBAND_AUDIO ]
#endif // HAVE_INTEL_IPP ]
#ifdef HAVE_ILBC
    // ILBC
    else if (strcmp(compareString,"ILBC_30MS") == 0)
       retType = SdpCodec::SDP_CODEC_ILBC_30MS;
    else if (strcmp(compareString,"ILBC_20MS") == 0)
       retType = SdpCodec::SDP_CODEC_ILBC_20MS;
#endif // HAVE_ILBC ]
#ifdef HAVE_SPAN_DSP
    // G.726
    else if (strcmp(compareString,"G726_16") == 0)
       retType = SdpCodec::SDP_CODEC_G726_16;
    else if (strcmp(compareString,"G726_24") == 0)
       retType = SdpCodec::SDP_CODEC_G726_24;
    else if (strcmp(compareString,"G726_32") == 0)
       retType = SdpCodec::SDP_CODEC_G726_32;
    else if (strcmp(compareString,"G726_40") == 0)
       retType = SdpCodec::SDP_CODEC_G726_40;
#ifdef ENABLE_WIDEBAND_AUDIO
    // G.722 codec
    else if (strcmp(compareString,"G722") == 0)
       retType = SdpCodec::SDP_CODEC_G722;
#endif // ENABLE_WIDEBAND_AUDIO ]
#endif // HAVE_SPAN_DSP ]
#ifdef HAVE_GSM
    // gsm full rate
    else if (strcmp(compareString,"GSM") == 0)
       retType = SdpCodec::SDP_CODEC_GSM;
#endif // HAVE_GSM ]
#ifdef HAVE_SPEEX
    // speex narrowband codecs
    else if (strcmp(compareString,"SPEEX_5") == 0)
       retType = SdpCodec::SDP_CODEC_SPEEX_5;
    else if (strcmp(compareString,"SPEEX_8") == 0)
       retType = SdpCodec::SDP_CODEC_SPEEX_8;
    else if (strcmp(compareString,"SPEEX_11") == 0)
       retType = SdpCodec::SDP_CODEC_SPEEX_11;
    else if (strcmp(compareString,"SPEEX_15") == 0)
       retType = SdpCodec::SDP_CODEC_SPEEX_15;
    else if (strcmp(compareString,"SPEEX_18") == 0)
       retType = SdpCodec::SDP_CODEC_SPEEX_18;
    else if (strcmp(compareString,"SPEEX_24") == 0)
       retType = SdpCodec::SDP_CODEC_SPEEX_24;
#ifdef ENABLE_WIDEBAND_AUDIO
    // speex wideband codecs
    else if (strcmp(compareString,"SPEEX_WB_9") == 0)
       retType = SdpCodec::SDP_CODEC_SPEEX_WB_9;
    else if (strcmp(compareString,"SPEEX_WB_12") == 0)
       retType = SdpCodec::SDP_CODEC_SPEEX_WB_12;
    else if (strcmp(compareString,"SPEEX_WB_16") == 0)
       retType = SdpCodec::SDP_CODEC_SPEEX_WB_16;
    else if (strcmp(compareString,"SPEEX_WB_20") == 0)
       retType = SdpCodec::SDP_CODEC_SPEEX_WB_20;
    else if (strcmp(compareString,"SPEEX_WB_23") == 0)
       retType = SdpCodec::SDP_CODEC_SPEEX_WB_23;
    else if (strcmp(compareString,"SPEEX_WB_27") == 0)
       retType = SdpCodec::SDP_CODEC_SPEEX_WB_27;
    else if (strcmp(compareString,"SPEEX_WB_34") == 0)
       retType = SdpCodec::SDP_CODEC_SPEEX_WB_34;
    else if (strcmp(compareString,"SPEEX_WB_42") == 0)
       retType = SdpCodec::SDP_CODEC_SPEEX_WB_42;
    // speex ultra wideband codecs
    else if (strcmp(compareString,"SPEEX_UWB_11") == 0)
       retType = SdpCodec::SDP_CODEC_SPEEX_UWB_11;
    else if (strcmp(compareString,"SPEEX_UWB_14") == 0)
       retType = SdpCodec::SDP_CODEC_SPEEX_UWB_14;
    else if (strcmp(compareString,"SPEEX_UWB_18") == 0)
       retType = SdpCodec::SDP_CODEC_SPEEX_UWB_18;
    else if (strcmp(compareString,"SPEEX_UWB_22") == 0)
       retType = SdpCodec::SDP_CODEC_SPEEX_UWB_22;
    else if (strcmp(compareString,"SPEEX_UWB_25") == 0)
       retType = SdpCodec::SDP_CODEC_SPEEX_UWB_25;
    else if (strcmp(compareString,"SPEEX_UWB_29") == 0)
       retType = SdpCodec::SDP_CODEC_SPEEX_UWB_29;
    else if (strcmp(compareString,"SPEEX_UWB_36") == 0)
       retType = SdpCodec::SDP_CODEC_SPEEX_UWB_36;
    else if (strcmp(compareString,"SPEEX_UWB_44") == 0)
       retType = SdpCodec::SDP_CODEC_SPEEX_UWB_44;
#endif // ENABLE_WIDEBAND_AUDIO ]
#endif // HAVE_SPEEX ]
#ifdef ENABLE_WIDEBAND_AUDIO
    // L16 codec
    else if (strcmp(compareString,"L16_8000_MONO") == 0)
       retType = SdpCodec::SDP_CODEC_L16_8000_MONO;
    else if (strcmp(compareString,"L16_11025_MONO") == 0)
       retType = SdpCodec::SDP_CODEC_L16_11025_MONO;
    else if (strcmp(compareString,"L16_16000_MONO") == 0)
       retType = SdpCodec::SDP_CODEC_L16_16000_MONO;
    else if (strcmp(compareString,"L16_22050_MONO") == 0)
       retType = SdpCodec::SDP_CODEC_L16_22050_MONO;
    else if (strcmp(compareString,"L16_24000_MONO") == 0)
       retType = SdpCodec::SDP_CODEC_L16_24000_MONO;
    else if (strcmp(compareString,"L16_32000_MONO") == 0)
       retType = SdpCodec::SDP_CODEC_L16_32000_MONO;
    else if (strcmp(compareString,"L16_44100_MONO") == 0)
       retType = SdpCodec::SDP_CODEC_L16_44100_MONO;
    else if (strcmp(compareString,"L16_48000_MONO") == 0)
       retType = SdpCodec::SDP_CODEC_L16_48000_MONO;
#endif // ENABLE_WIDEBAND_AUDIO ]
#ifdef VIDEO
    // video codecs
    else if (strcmp(compareString,"VP71-CIF") == 0)
       retType = SdpCodec::SDP_CODEC_VP71_CIF;
    else if (strcmp(compareString,"VP71-QCIF") == 0)
       retType = SdpCodec::SDP_CODEC_VP71_QCIF;
    else if (strcmp(compareString,"VP71-SQCIF") == 0)
       retType = SdpCodec::SDP_CODEC_VP71_SQCIF;
    else if (strcmp(compareString,"VP71-QVGA") == 0)
       retType = SdpCodec::SDP_CODEC_VP71_QVGA;
    else if (strcmp(compareString,"IYUV-CIF") == 0)
       retType = SdpCodec::SDP_CODEC_IYUV_CIF;
    else if (strcmp(compareString,"IYUV-QCIF") == 0)
       retType = SdpCodec::SDP_CODEC_IYUV_QCIF;
    else if (strcmp(compareString,"IYUV-SQCIF") == 0)
       retType = SdpCodec::SDP_CODEC_IYUV_SQCIF;
    else if (strcmp(compareString,"IYUV-QVGA") == 0)
       retType = SdpCodec::SDP_CODEC_IYUV_QVGA;
    else if (strcmp(compareString,"I420-CIF") == 0)
       retType = SdpCodec::SDP_CODEC_I420_CIF;
    else if (strcmp(compareString,"I420-QCIF") == 0)
       retType = SdpCodec::SDP_CODEC_I420_QCIF;
    else if (strcmp(compareString,"I420-SQCIF") == 0)
       retType = SdpCodec::SDP_CODEC_I420_SQCIF;
    else if (strcmp(compareString,"I420-QVGA") == 0)
       retType = SdpCodec::SDP_CODEC_I420_QVGA;
    else if (strcmp(compareString,"H263-CIF") == 0)
       retType = SdpCodec::SDP_CODEC_H263_CIF;
    else if (strcmp(compareString,"H263-QCIF") == 0)
       retType = SdpCodec::SDP_CODEC_H263_QCIF;
    else if (strcmp(compareString,"H263-SQCIF") == 0)
       retType = SdpCodec::SDP_CODEC_H263_SQCIF;
    else if (strcmp(compareString,"H263-QVGA") == 0)
       retType = SdpCodec::SDP_CODEC_H263_QVGA;
    else if (strcmp(compareString,"RGB24-CIF") == 0)
       retType = SdpCodec::SDP_CODEC_RGB24_CIF;
    else if (strcmp(compareString,"RGB24-QCIF") == 0)
       retType = SdpCodec::SDP_CODEC_RGB24_QCIF;
    else if (strcmp(compareString,"RGB24-SQCIF") == 0)
       retType = SdpCodec::SDP_CODEC_RGB24_SQCIF;
    else if (strcmp(compareString,"RGB24-QVGA") == 0)
       retType = SdpCodec::SDP_CODEC_RGB24_QVGA;
#endif // VIDEO ]
    else
       retType = SdpCodec::SDP_CODEC_UNKNOWN;
    return retType;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
