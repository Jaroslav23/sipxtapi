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
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "net/SipPimClient.h"
#include "tapi/SipXPim.h"
#include "tapi/sipXtapi.h"
#include "tapi/SipXCore.h"
#include "os/OsDefs.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* ============================ FUNCTIONS ================================= */


/****************************************************************************
* Public Presence & Instant messaging Functions
***************************************************************************/

SIPXTAPI_API SIPX_RESULT sipxPIMSendPagerMessage(SIPX_INST hInst,
                                                 const char* destinationAor, 
                                                 const char* messageText,
                                                 const char* subject,
                                                 int* responseCode,
                                                 char* responseCodeText,
                                                 size_t buffLength,
                                                 SIPX_TRANSPORT_TYPE transport)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxPIMSendPagerMessage");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxPIMSendPagerMessage hInst=%p, destinationAor=%s, messageText=%s, "
      "subject=%s, responseCode=%i, responseCodeText=%s, buffLength=%i",
      hInst, destinationAor, messageText, subject, responseCode, responseCodeText, buffLength);

   SIPX_RESULT rc = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst && pInst->pSipPimClient && messageText && subject)
   {
      Url urlAor(destinationAor);
      UtlString responseText;

      UtlBoolean res = pInst->pSipPimClient->sendPagerMessage(urlAor,
                              messageText,
                              subject,
                              *responseCode,
                              responseText,
                              (SIP_TRANSPORT_TYPE)transport);

      if (res == TRUE)
      {
         SNPRINTF(responseCodeText, buffLength, "%s", responseText.data());
         rc = SIPX_RESULT_SUCCESS;
      }
   }

   return rc;
}
