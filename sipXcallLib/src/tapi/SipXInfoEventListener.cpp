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
#include "tapi/SipXInfoEventListener.h"
#include <tapi/InfoEventMsg.h>
#include "tapi/SipXEvents.h"
#include <tapi/SipXEventDispatcher.h>
#include <tapi/SipXCall.h>

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

SipXInfoEventListener::SipXInfoEventListener(SIPX_INST pInst)
: OsSharedServerTask("SipXInfoEventListener-%d")
, SipInfoEventListener()
, m_pInst(pInst)
{

}

SipXInfoEventListener::~SipXInfoEventListener()
{
   release();
}

/* ============================ MANIPULATORS ============================== */

void SipXInfoEventListener::OnInfoMessage(const SipInfoEvent& event)
{
   InfoEventMsg msg(event);
   postMessage(msg);
}

UtlBoolean SipXInfoEventListener::handleMessage(OsMsg& rRawMsg)
{
   UtlBoolean bResult = FALSE;

   switch (rRawMsg.getMsgType())
   {
   case INFO_MSG:
      {
         InfoEventMsg* pMsg = dynamic_cast<InfoEventMsg*>(&rRawMsg);
         if (pMsg)
         {
            // cast succeeded
            const SipInfoEvent& payload = pMsg->getEventPayloadRef();
            handleInfoEvent(payload.m_sCallId, payload.m_sContentType, payload.m_pContent, payload.m_nContentLength);
         }
      }
      bResult = TRUE;
      break;
   default:
      break;
   }
   return bResult;

}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void SipXInfoEventListener::handleInfoEvent(const UtlString& sAbstractCallId,
                                            const UtlString& sContentType,
                                            const char* pContent,
                                            size_t nContentLength)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "handleInfoEvent");
   SIPX_CALL hCall = SIPX_CALL_NULL;
   SIPX_LINE hLine = SIPX_LINE_NULL;

   hCall = sipxCallLookupHandleByCallId(sAbstractCallId, m_pInst);
   if (hCall != SIPX_CALL_NULL)
   {
      // also try to find line handle
      SIPX_CALL_DATA* pCallData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);
      if (pCallData)
      {
         hLine = pCallData->m_hLine;
         sipxCallReleaseLock(pCallData, SIPX_LOCK_READ, stackLogger);
      }
   }

   SIPX_INFO_INFO info;
   memset((void*)&info, 0, sizeof(SIPX_INFO_INFO));

   info.nSize = sizeof(SIPX_INFO_INFO);
   info.hCall = hCall;
   info.hLine = hLine;
   info.szContentType = sContentType.data(); // safe, since dispatcher makes a copy
   info.pContent = pContent; // safe, since dispatcher makes a copy
   info.nContentLength = nContentLength;

   SipXEventDispatcher::dispatchEvent(m_pInst, EVENT_CATEGORY_INFO, &info);
}

/* ============================ FUNCTIONS ================================= */

