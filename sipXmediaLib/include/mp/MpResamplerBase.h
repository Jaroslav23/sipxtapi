//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Alexander Chemeris <Alexander DOT Chemeris AT SIPez DOT com> and Keith Kyzivat <kkyzivat AT SIPez DOT com>

#ifndef MpResamplerBase_h__
#define MpResamplerBase_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <mp/MpTypes.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * Base class for all resamplers.
 */
class MpResamplerBase
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /** Constructor */
   MpResamplerBase(uint32_t inputRate, 
                   uint32_t outputRate, 
                   int32_t quality);

   /* ============================ MANIPULATORS ============================== */

   /// Reset resampler state to prepare for processing new (unrelated) stream.
   virtual OsStatus resetStream() = 0;

   /// Resample audio data.
   virtual OsStatus resample(const MpAudioSample* pInBuf,
                             uint32_t inBufLength,
                             uint32_t& inSamplesProcessed,
                             MpAudioSample* pOutBuf,
                             uint32_t outBufLength,
                             uint32_t& outSamplesWritten) = 0;

   /* ============================ ACCESSORS ================================= */

   /// Return input sampling rate.
   uint32_t getInputRate() const;

   /// Return output sampling rate.
   uint32_t getOutputRate() const;

   /// Return quality of resampling conversion.
   int32_t getQuality() const;

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   uint32_t m_inputRate; ///< input sampling rate
   uint32_t m_outputRate; ///< output sampling rate
   int32_t m_quality;

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // MpResamplerBase_h__
