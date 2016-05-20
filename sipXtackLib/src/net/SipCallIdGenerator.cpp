//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
//
// Copyright (C) 2005-2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsSocket.h>
#include <os/OsDateTime.h>
#include <os/OsProcess.h>
#include <net/SipCallIdGenerator.h>
#include <net/NetMd5Codec.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
unsigned int SipCallIdGenerator::ms_instanceCounter = 0;

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

SipCallIdGenerator::SipCallIdGenerator(const UtlString& prefix) : m_callIdCounter(0)
, m_sPrefix(prefix)
, m_mutex(OsMutex::Q_FIFO)
, m_RandomNumGenerator()
{
   memset(m_callIdSuffix, 0, sizeof(m_callIdSuffix));

   OsTime current_time;
   OsDateTime::getCurTime(current_time);
   m_creationTime = ((Int64) current_time.seconds()) * 1000000 + current_time.usecs();
   m_processId = OsProcess::getCurrentPID();
   m_instanceId = ms_instanceCounter++;

   SNPRINTF(m_callIdSuffix, sizeof(m_callIdSuffix), "%d_%d_%" FORMAT_INTLL "d",
      m_processId, m_instanceId, m_creationTime);

}

SipCallIdGenerator::~SipCallIdGenerator(void)
{

}

/* ============================ MANIPULATORS ============================== */

UtlString SipCallIdGenerator::getNewCallId()
{
   char callIdBuffer[500];
   UtlString sMd5Suffix;
   UtlString sCallId;

   m_mutex.acquire();
   // md5 has a nice property that even if there is slight change, hash
   // is quite different
   SNPRINTF(callIdBuffer, sizeof(callIdBuffer), "%" FORMAT_INTLL "d_%d_%s",
      m_callIdCounter++, m_RandomNumGenerator.rand(), m_callIdSuffix);
   m_mutex.release();

   // now compute md5 of counter and callidsuffix
   NetMd5Codec encoder;
   encoder.encode(callIdBuffer, sMd5Suffix);

   sCallId.append(m_sPrefix);
   sCallId.append(sMd5Suffix);


   return sCallId;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


