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
#include <net/SipTagGenerator.h>
#include <net/NetMd5Codec.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
unsigned int SipTagGenerator::ms_instanceCounter = 0;

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

SipTagGenerator::SipTagGenerator(const UtlString& prefix, const unsigned int tagLength)
: m_tagCounter(0)
, m_sPrefix(prefix)
, m_mutex(OsMutex::Q_FIFO)
, m_RandomNumGenerator()
, m_tagLength(tagLength)
{
   memset(m_tmpTagSuffix, 0, sizeof(m_tmpTagSuffix));

   OsTime current_time;
   OsDateTime::getCurTime(current_time);
   m_creationTime = ((Int64) current_time.seconds()) * 1000000 + current_time.usecs();
   m_processId = OsProcess::getCurrentPID();
   m_instanceId = ms_instanceCounter++;
   OsSocket::getHostIp(&m_sHostIp);

   SNPRINTF(m_tmpTagSuffix, sizeof(m_tmpTagSuffix), "%d_%d_%" FORMAT_INTLL "d_%s",
      m_processId, m_instanceId, m_creationTime, m_sHostIp.data());

}

SipTagGenerator::~SipTagGenerator(void)
{

}

/* ============================ MANIPULATORS ============================== */

UtlString SipTagGenerator::getNewTag()
{
   char tagBuffer[500];
   UtlString sMd5Suffix;
   UtlString sTag;

   m_mutex.acquire();
   // md5 has a nice property that even if there is slight change, hash
   // is quite different
   SNPRINTF(tagBuffer, sizeof(tagBuffer), "%" FORMAT_INTLL "d_%d_%s",
      m_tagCounter++, m_RandomNumGenerator.rand(), m_tmpTagSuffix);
   m_mutex.release();

   // now compute md5 of counter and tagsuffix
   NetMd5Codec encoder;
   encoder.encode(tagBuffer, sMd5Suffix);
   sMd5Suffix.remove(m_tagLength);

   sTag.append(m_sPrefix);
   sTag.append(sMd5Suffix);


   return sTag;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


