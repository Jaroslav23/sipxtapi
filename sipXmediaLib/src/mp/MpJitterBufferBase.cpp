//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpJitterBufferBase.h>

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

MpJitterBufferBase::MpJitterBufferBase(const UtlString& name,
                                       int payloadType,
                                       unsigned int samplesPerFrame)
: m_name(name)
, m_payloadType(payloadType)
, m_samplesPerFrame(samplesPerFrame)
{

}

MpJitterBufferBase::~MpJitterBufferBase()
{

}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

int MpJitterBufferBase::getPayloadType()
{
   return m_payloadType;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

