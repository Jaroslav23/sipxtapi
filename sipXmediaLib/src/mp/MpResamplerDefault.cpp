//  
// Copyright (C) 2007-2008 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com>

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsIntTypes.h"
#include <os/OsSysLog.h>
#include "mp/MpResamplerDefault.h"
#include <mp/MpAudioUtils.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// DEFINES
// MACROS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////////// PUBLIC //////////////////////////////// */

/* =============================== CREATORS =============================== */

MpResamplerDefault::MpResamplerDefault(uint32_t inputRate,
                                       uint32_t outputRate, 
                                       int32_t quality)
: MpResamplerBase(inputRate, outputRate, quality)
{
   // No other initialization needed.
}

MpResamplerDefault::~MpResamplerDefault()
{
   // No de-initialization needed.
}

/* ============================= MANIPULATORS ============================= */

OsStatus MpResamplerDefault::resetStream()
{
   // nothing to reset in this case.
   // Meant for child classes.
   return OS_SUCCESS;
}

OsStatus MpResamplerDefault::resample(const MpAudioSample* pInBuf,
                                      uint32_t inBufLength, 
                                      uint32_t& inSamplesProcessed, 
                                      MpAudioSample* pOutBuf,
                                      uint32_t outBufLength,
                                      uint32_t& outSamplesWritten)
{
   // state is not maintained between calls for this particular default
   // resampler, so the channelIndex doesn't really matter here, but for
   // consistency, we do check to see if the channel specified is out of 
   // range (i.e. larger than the number of channels that were specified)

   OsStatus ret = OS_FAILED;
   if(m_inputRate > m_outputRate)
   {
      // Upsampling is not supported without SPEEX.
      ret = OS_NOT_YET_IMPLEMENTED;
   }
   else if(m_inputRate == m_outputRate)
   {
      if (inBufLength > outBufLength)
      {
         ret = OS_NO_MEMORY;
      }
      else
      {
         outSamplesWritten = min(inBufLength, outBufLength);
         inSamplesProcessed = outSamplesWritten;
         memcpy(pOutBuf, pInBuf, outSamplesWritten * sizeof(MpAudioSample));
         ret = OS_SUCCESS;
      }
   }
   else
   {
      // Downsampling.
      uint32_t keptSamples = 0, currentSample = 0;
      uint32_t rkeptSamples = 0, rcurrentSample = 0;

      uint32_t rateGcd = gcd(m_inputRate, m_outputRate);
      uint32_t inRateDivGCD = m_inputRate / rateGcd;
      uint32_t outRateDivGCD = m_outputRate / rateGcd;

      for(; currentSample < inBufLength && keptSamples < outBufLength; 
          currentSample++, rcurrentSample++)
      {
         if (rkeptSamples * inRateDivGCD <= rcurrentSample * outRateDivGCD)
         {
            pOutBuf[rkeptSamples++, keptSamples++] = pInBuf[currentSample];
            if(rkeptSamples == outRateDivGCD && rcurrentSample == inRateDivGCD)
               rkeptSamples = rcurrentSample = 0;
         }
      }
      inSamplesProcessed = currentSample;
      outSamplesWritten = keptSamples;

      ret = (outSamplesWritten == outBufLength) ? OS_SUCCESS : OS_NO_MEMORY;
   }
   return ret;
}

/* ============================== ACCESSORS =============================== */

/* =============================== INQUIRY ================================ */

/* ////////////////////////////// PROTECTED /////////////////////////////// */

/* /////////////////////////////// PRIVATE //////////////////////////////// */

/* ============================== FUNCTIONS =============================== */
