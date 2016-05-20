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

#ifndef MpNoiseGeneratorSpanDsp_h__
#define MpNoiseGeneratorSpanDsp_h__
#ifdef HAVE_SPAN_DSP

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpNoiseGeneratorBase.h>
#include <spandsp/telephony.h>
#include <spandsp/noise.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * Span DSP noise generator.
 */
class MpNoiseGeneratorSpanDsp : public MpNoiseGeneratorBase
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /** Constructor */
   MpNoiseGeneratorSpanDsp(int seed, float noiseLevel, int noiseQuality);

   /** Destructor */
   virtual ~MpNoiseGeneratorSpanDsp();

   /* ============================ MANIPULATORS ============================== */

   /**
   * Generates comfort noise into specified buffer.
   */
   virtual void generateComfortNoise(MpAudioSample* pSamplesBuffer, unsigned sampleCount);

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   noise_state_t* m_pNoiseState; ///< state of noise generator
};


#endif // HAVE_SPAN_DSP ]
#endif // MpNoiseGeneratorSpanDsp_h__
