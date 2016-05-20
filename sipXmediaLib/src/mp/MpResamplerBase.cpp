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
#include <mp/MpResamplerBase.h>

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

MpResamplerBase::MpResamplerBase(uint32_t inputRate,
                                 uint32_t outputRate,
                                 int32_t quality)
: m_inputRate(inputRate)
, m_outputRate(outputRate)
, m_quality(quality >= 0 ? quality : 0)
{

}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

uint32_t MpResamplerBase::getInputRate() const
{
   return m_inputRate;
}

uint32_t MpResamplerBase::getOutputRate() const
{
   return m_outputRate;
}

int32_t MpResamplerBase::getQuality() const
{
   return m_quality;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
