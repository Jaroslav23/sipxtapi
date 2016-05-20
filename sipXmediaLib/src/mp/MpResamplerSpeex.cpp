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

#ifdef HAVE_SPEEX

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <mp/MpResamplerSpeex.h>

// WIN32: Add libspeexdsp to linker input.
#ifdef WIN32 // [
#   ifdef _DEBUG // [
#      pragma comment(lib, "libspeexdspd.lib")
#   else // _DEBUG ][
#      pragma comment(lib, "libspeexdsp.lib")
#   endif // _DEBUG ]
#endif // WIN32 ]

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// TYPEDEFS
// DEFINES
// MACROS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////////// PUBLIC //////////////////////////////// */

/* =============================== CREATORS =============================== */

MpResamplerSpeex::MpResamplerSpeex(uint32_t inputRate,
                                   uint32_t outputRate, 
                                   int32_t quality)
: MpResamplerBase(inputRate, outputRate,
                  quality >= 0 ? quality : SPEEX_RESAMPLER_QUALITY_VOIP)
{
   int speexErr = 0;
   m_pResamplerState = speex_resampler_init(1, inputRate, outputRate, 
                                  quality, &speexErr);

   assert(speexErr == RESAMPLER_ERR_SUCCESS);
}

MpResamplerSpeex::~MpResamplerSpeex()
{
   // Destroy speex state object
   speex_resampler_destroy(m_pResamplerState);
   m_pResamplerState = NULL;
}

/* ============================= MANIPULATORS ============================= */

OsStatus MpResamplerSpeex::resetStream()
{
   return speexErrToOsStatus(speex_resampler_reset_mem(m_pResamplerState));
}

// Single-channel resampling.
OsStatus MpResamplerSpeex::resample(const MpAudioSample* pInBuf,
                                    uint32_t inBufLength, 
                                    uint32_t& inSamplesProcessed, 
                                    MpAudioSample* pOutBuf,
                                    uint32_t outBufLength, 
                                    uint32_t& outSamplesWritten)
{
   inSamplesProcessed = inBufLength;
   outSamplesWritten = outBufLength;
   int speexErr = speex_resampler_process_int(m_pResamplerState, 0, 
                                              pInBuf, &inSamplesProcessed, 
                                              pOutBuf, &outSamplesWritten);
   return speexErrToOsStatus(speexErr);
}

/* ============================== ACCESSORS =============================== */

/* =============================== INQUIRY ================================ */

/* ////////////////////////////// PROTECTED /////////////////////////////// */

/* /////////////////////////////// PRIVATE //////////////////////////////// */

OsStatus MpResamplerSpeex::speexErrToOsStatus(int speexErr)
{
   OsStatus ret = OS_SUCCESS;
   switch(speexErr)
   {
   case RESAMPLER_ERR_SUCCESS:
      ret = OS_SUCCESS;
      break;
   case RESAMPLER_ERR_ALLOC_FAILED:
      ret = OS_NO_MEMORY;
      break;
   case RESAMPLER_ERR_BAD_STATE:
      ret = OS_INVALID_STATE;
      break;
   case RESAMPLER_ERR_INVALID_ARG:
      ret = OS_INVALID_ARGUMENT;
      break;
   case RESAMPLER_ERR_PTR_OVERLAP:
      ret = OS_INVALID;
      break;
   default:
      ret = OS_FAILED;
      break;
   }
   return ret;
}

/* ============================== FUNCTIONS =============================== */

#endif // HAVE_SPEEX
