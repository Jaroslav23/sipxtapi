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
#include "tapi/SipXInfoStatusEventListener.h"
#include <tapi/InfoStatusEventMsg.h>
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

SipXInfoStatusEventListener::SipXInfoStatusEventListener(SIPX_INST pInst)
: OsSharedServerTask("SipXInfoStatusEventListener-%d")
, SipInfoStatusEventListener()
, m_pInst(pInst)
{

}

SipXInfoStatusEventListener::~SipXInfoStatusEventListener()
{
   release();
}

/* ============================ MANIPULATORS ============================== */

void SipXInfoStatusEventListener::OnResponse(const SipInfoStatusEvent& event)
{
   InfoStatusEventMsg msg(INFOSTATUS_RESPONSE, event);
   postMessage(msg);
}

void SipXInfoStatusEventListener::OnNetworkError(const SipInfoStatusEvent& event)
{
   InfoStatusEventMsg msg(INFOSTATUS_NETWORK_ERROR, event);
   postMessage(msg);
}

UtlBoolean SipXInfoStatusEventListener::handleMessage(OsMsg& rRawMsg)
{
   UtlBoolean bResult = FALSE;

   switch (rRawMsg.getMsgType())
   {
   case INFOSTATUS_MSG:
      {
         InfoStatusEventMsg* pMsg = dynamic_cast<InfoStatusEventMsg*>(&rRawMsg);
         if (pMsg)
         {
            // cast succeeded
            const SipInfoStatusEvent& payload = pMsg->getEventPayloadRef();
            handleInfoStatusEvent(payload.m_sCallId, (SIPX_MESSAGE_STATUS)payload.m_status,
               payload.m_iResponseCode, payload.m_sResponseText, pMsg->getEvent(), payload.m_pCookie);
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

void SipXInfoStatusEventListener::handleInfoStatusEvent(const UtlString& sAbstractCallId,
                                                        SIPX_MESSAGE_STATUS status,
                                                        int responseCode,
                                                        const UtlString& sResponseText,
                                                        SIPX_INFOSTATUS_EVENT event,
                                                        void* pCookie)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "handleInfoStatusEvent");
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

   SIPX_INFOSTATUS_INFO infoStatus;
   memset((void*)&infoStatus, 0, sizeof(SIPX_INFOSTATUS_INFO));

   infoStatus.nSize = sizeof(SIPX_INFOSTATUS_INFO);
   infoStatus.hCall = hCall;
   infoStatus.hLine = hLine;
   infoStatus.status = status;
   infoStatus.responseCode = responseCode;
   infoStatus.szResponseText = sResponseText.data();
   infoStatus.event = event;
   infoStatus.pCookie = pCookie;

   SipXEventDispatcher::dispatchEvent(m_pInst, EVENT_CATEGORY_INFO_STATUS, &infoStatus);
}

/* ============================ FUNCTIONS ================================= */

