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

#ifndef MpResamplerFactory_h__
#define MpResamplerFactory_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsIntTypes.h>
#include <mp/MpResamplerBase.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * Factory for creating instances of resamplers.
 */
class MpResamplerFactory
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /**
    * Creates new resampler with specified capabilities.
    */
   static MpResamplerBase *createResampler(uint32_t inputRate,
                                           uint32_t outputRate,
                                           int32_t quality = 3);

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /** Constructor */
   MpResamplerFactory();

   /** Destructor */
   ~MpResamplerFactory();
};

#endif // MpResamplerFactory_h__
