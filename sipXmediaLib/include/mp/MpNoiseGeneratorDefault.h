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

#ifndef MpNoiseGeneratorDefault_h__
#define MpNoiseGeneratorDefault_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpNoiseGeneratorBase.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * Default noise generator.
 */
class MpNoiseGeneratorDefault : public MpNoiseGeneratorBase
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */
   
   /** Constructor */
   MpNoiseGeneratorDefault();

   /* ============================ MANIPULATORS ============================== */

   /**
   * Generates comfort noise into specified buffer. Default noise generator only
   * zeroes the buffer.
   */
   virtual void generateComfortNoise(MpAudioSample* pSamplesBuffer, unsigned sampleCount);

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // MpNoiseGeneratorDefault_h__
