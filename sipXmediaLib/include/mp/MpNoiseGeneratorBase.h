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

#ifndef MpNoiseGeneratorBase_h__
#define MpNoiseGeneratorBase_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
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
 * Base class for noise generators.
 */
class MpNoiseGeneratorBase
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /** Constructor */
   MpNoiseGeneratorBase();

   /** Destructor */
   virtual ~MpNoiseGeneratorBase();

   /* ============================ MANIPULATORS ============================== */

   /**
    * Generates comfort noise into specified buffer.
    */
   virtual void generateComfortNoise(MpAudioSample* pSamplesBuffer, unsigned sampleCount) = 0;

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /** Private copy constructor */
   MpNoiseGeneratorBase(const MpNoiseGeneratorBase& rhs);

   /** Private assignment operator */
   MpNoiseGeneratorBase& operator=(const MpNoiseGeneratorBase& rhs);
};

#endif // MpNoiseGeneratorBase_h__
