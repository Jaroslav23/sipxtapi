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
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif /* _WIN32 */

// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <utl/UtlRandom.h>
#include "tapi/SecurityHelper.h"
#include "tapi/sipXtapi.h"

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

/* ============================ MANIPULATORS ============================== */


/* SecurityHelper */
void SecurityHelper::generateSrtpKey(SIPX_SECURITY_ATTRIBUTES& securityAttrib)
{
   char buffer[32];
   UtlRandom r;
   long t;

   for (int i = 0; i < 30; ++i)
   {
      t = 91;
      while (t >=91 && t <=96)
      {
         t = (r.rand() % 58) + 65;
      }
      buffer[i] = (char)t;
   }
   buffer[30] = 0;
   securityAttrib.setSrtpKey(buffer, 30);
   return;
}

/* ============================ ACCESSORS ================================= */

void SecurityHelper::setDbLocation(SIPX_SECURITY_ATTRIBUTES& securityAttrib, const char* dbLocation)
{
   SAFE_STRNCPY(securityAttrib.dbLocation, dbLocation, sizeof(securityAttrib.dbLocation));
   return;
}

void SecurityHelper::setDbPassword(SIPX_SECURITY_ATTRIBUTES& securityAttrib, const char* dbPassword)
{
   SAFE_STRNCPY(securityAttrib.szCertDbPassword, dbPassword, sizeof(securityAttrib.szCertDbPassword));
   return;
}

void SecurityHelper::setMyCertNickname(SIPX_SECURITY_ATTRIBUTES& securityAttrib, const char* szMyCertNickname)
{
   SAFE_STRNCPY(securityAttrib.szMyCertNickname, szMyCertNickname, sizeof(securityAttrib.szMyCertNickname));
   return;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
