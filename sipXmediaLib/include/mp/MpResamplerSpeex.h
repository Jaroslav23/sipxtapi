//  
// Copyright (C) 2008 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>

#ifndef _MpResamplerSpeex_h_
#define _MpResamplerSpeex_h_

#ifdef HAVE_SPEEX

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <mp/MpTypes.h>
#include <mp/MpResamplerBase.h>
#include <speex/speex_resampler.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief Wrapper for Speex audio resampler.
*/
class MpResamplerSpeex : public MpResamplerBase
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

/* =============================== CREATORS =============================== */
///@name Creators
//@{

   /** Constructor */
   MpResamplerSpeex(uint32_t inputRate, 
                    uint32_t outputRate, 
                    int32_t quality = -1);

   /** Destructor */
   ~MpResamplerSpeex();

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

   /// Reset resampler state to prepare for processing new (unrelated) stream.
   OsStatus resetStream();

   /// Resample audio data.
   OsStatus resample(const MpAudioSample* pInBuf,
                     uint32_t inBufLength,
                     uint32_t& inSamplesProcessed,
                     MpAudioSample* pOutBuf,
                     uint32_t outBufLength,
                     uint32_t& outSamplesWritten);

//@}

/* ============================== ACCESSORS =============================== */
///@name Accessors
//@{

//@}

/* =============================== INQUIRY ================================ */
///@name Inquiry
//@{


//@}

/* ////////////////////////////// PROTECTED /////////////////////////////// */
protected:

/* /////////////////////////////// PRIVATE //////////////////////////////// */
private:
   OsStatus speexErrToOsStatus(int speexErr);

private:
   SpeexResamplerState* m_pResamplerState;
};

/* ============================ INLINE METHODS ============================ */

#endif  // HAVE_SPEEX
#endif  // _MpResamplerSpeex_h_
