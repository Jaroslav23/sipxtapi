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

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpResamplerFactory.h>
#include <mp/MpResamplerDefault.h>
#ifdef HAVE_SPEEX
#include <mp/MpResamplerSpeex.h>
#endif // HAVE_SPEEX ]

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpResamplerBase * MpResamplerFactory::createResampler(uint32_t inputRate,
                                                      uint32_t outputRate,
                                                      int32_t quality /*= -1*/)
{
#ifdef HAVE_SPEEX
   return new MpResamplerSpeex(inputRate, outputRate, quality);
#else
   return new MpResamplerDefault(inputRate, outputRate, quality);
#endif // HAVE_SPEEX ]
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
