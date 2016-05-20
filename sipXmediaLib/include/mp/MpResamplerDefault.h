//  
// Copyright (C) 2007-2008 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2007-2008 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com> and Keith Kyzivat <kkyzivat AT SIPez DOT com>

#ifndef _MpResamplerDefault_h_
#define _MpResamplerDefault_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpTypes.h>
#include <os/OsStatus.h>
#include <mp/MpMisc.h>
#include <mp/MpResamplerBase.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief Generic audio resampler.
*/
class MpResamplerDefault : public MpResamplerBase
{
/* //////////////////////////////// PUBLIC //////////////////////////////// */
public:

/* =============================== CREATORS =============================== */
///@name Creators
//@{

   /// Constructor
   MpResamplerDefault(uint32_t inputRate, 
                      uint32_t outputRate, 
                      int32_t quality);
     /**<
     *  @param[in] inputRate - The sample rate of the input audio.
     *  @param[in] outputRate - The sample rate of the output audio.
     *  @param[in] quality - The quality parameter is used by some resamplers to
     *             control the tradeoff of quality for latency and complexity.
     */

     /// Destructor
   virtual ~MpResamplerDefault();

//@}

/* ============================= MANIPULATORS ============================= */
///@name Manipulators
//@{

   /// Reset resampler state to prepare for processing new (unrelated) stream.
   virtual OsStatus resetStream();

   /// Resample audio data coming from the specified channel.
   virtual OsStatus resample(const MpAudioSample* pInBuf,
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
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpResamplerDefault_h_
