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

#ifndef MpNoiseGeneratorFactory_h__
#define MpNoiseGeneratorFactory_h__

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
 * Creates noise generator instances.
 */
class MpNoiseGeneratorFactory
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /* ============================ MANIPULATORS ============================== */

   /**
    * Creates noise generator.
    */
   static MpNoiseGeneratorBase* createNoiseGenerator();

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /** Constructor */
   MpNoiseGeneratorFactory();

   /** Destructor */
   ~MpNoiseGeneratorFactory();
};

#endif // MpNoiseGeneratorFactory_h__
