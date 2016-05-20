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
#include <utl/UtlInit.h>

#include <net/SipUserAgent.h>
#include "tapi/SipXInfo.h"
#include "tapi/SipXHandleMap.h"
#include "tapi/SipXCall.h"
#include "os/OsDefs.h"
#include "tapi/SipXMessageObserver.h"
#include "cp/XCpCallManager.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

SIPXTAPI_API SIPX_RESULT sipxCallSendInfo(const SIPX_CALL hCall,
                                          const char* szContentType,
                                          const char* pContent,
                                          const size_t nContentLength,
                                          void* pCookie)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallSendInfo");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallSendInfo hCall=%d contentType=%s content=%p nContentLength=%d pCookie=%d",
      hCall, szContentType, pContent, nContentLength, pCookie);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   SIPX_CALL_DATA* pCall = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger); 

   if (pCall && nContentLength >=0)
   {
      SipDialog sipDialog = pCall->m_sipDialog;
      SIPX_INSTANCE_DATA* pInst = pCall->m_pInst;
      UtlString sCallId = pCall->m_abstractCallId;
      assert(pInst);
      sipxCallReleaseLock(pCall, SIPX_LOCK_READ, stackLogger);

      if (sipDialog.isEstablishedDialog())
      {
         if (pInst->pCallManager->sendInfo(sCallId, sipDialog, szContentType, pContent, nContentLength, pCookie))
         {
            sr = SIPX_RESULT_SUCCESS;
         }
      }
      else
      {
         sr = SIPX_RESULT_INVALID_STATE;
      }
   }
   else
   {
      sr = SIPX_RESULT_INVALID_ARGS;
   }

   return sr;
}
