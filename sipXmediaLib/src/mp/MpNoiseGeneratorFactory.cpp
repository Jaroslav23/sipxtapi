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
#include <mp/MpDefs.h>
#include <mp/MpNoiseGeneratorFactory.h>
#include <mp/MpNoiseGeneratorDefault.h>

#ifdef HAVE_SPAN_DSP
#include <mp/MpNoiseGeneratorSpanDsp.h>
#endif // HAVE_SPAN_DSP ]

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

MpNoiseGeneratorFactory::MpNoiseGeneratorFactory()
{

}

MpNoiseGeneratorFactory::~MpNoiseGeneratorFactory()
{

}

/* ============================ MANIPULATORS ============================== */

MpNoiseGeneratorBase* MpNoiseGeneratorFactory::createNoiseGenerator()
{
#ifdef HAVE_SPAN_DSP
   return new MpNoiseGeneratorSpanDsp(6513, NOISE_LEVEL, 5);
#else
   return new MpNoiseGeneratorDefault();
#endif // HAVE_SPAN_DSP ]
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
