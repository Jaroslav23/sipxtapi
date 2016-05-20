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
#include <cp/XCpCallManager.h>
#include "tapi/SipXRtpRedirectEventListener.h"
#include <tapi/RtpRedirectEventMsg.h>
#include <tapi/SipXEventDispatcher.h>
#include "tapi/SipXEvents.h"
#include <tapi/SipXCore.h>
#include <tapi/SipXCall.h>
#include <tapi/SipXLine.h>

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

SipXRtpRedirectEventListener::SipXRtpRedirectEventListener( SIPX_INST pInst )
: OsSharedServerTask("SipXRtpRedirectEventListener-%d")
, CpRtpRedirectEventListener()
, m_pInst(pInst)
{

}

SipXRtpRedirectEventListener::~SipXRtpRedirectEventListener()
{
   release();
}

/* ============================ MANIPULATORS ============================== */

void SipXRtpRedirectEventListener::OnRtpRedirectRequested(const CpRtpRedirectEvent& event)
{
   RtpRedirectEventMsg msg(RTP_REDIRECT_REQUESTED, event);
   postMessage(msg);
}

void SipXRtpRedirectEventListener::OnRtpRedirectActive(const CpRtpRedirectEvent& event)
{
   RtpRedirectEventMsg msg(RTP_REDIRECT_ACTIVE, event);
   postMessage(msg);
}

void SipXRtpRedirectEventListener::OnRtpRedirectError(const CpRtpRedirectEvent& event)
{
   RtpRedirectEventMsg msg(RTP_REDIRECT_ERROR, event);
   postMessage(msg);
}

void SipXRtpRedirectEventListener::OnRtpRedirectStop(const CpRtpRedirectEvent& event)
{
   RtpRedirectEventMsg msg(RTP_REDIRECT_STOP, event);
   postMessage(msg);
}

UtlBoolean SipXRtpRedirectEventListener::handleMessage(OsMsg& rRawMsg)
{
   UtlBoolean bResult = FALSE;

   switch (rRawMsg.getMsgType())
   {
   case RTPREDIRECTEVENT_MSG:
      {
         RtpRedirectEventMsg* pMsg = dynamic_cast<RtpRedirectEventMsg*>(&rRawMsg);
         if (pMsg)
         {
            // cast succeeded
            const CpRtpRedirectEvent& payload = pMsg->getEventPayloadRef();
            handleRtpRedirectEvent(payload.m_sCallId, pMsg->getEvent(), (SIPX_RTP_REDIRECT_CAUSE)payload.m_cause);
         }
      }
      bResult = TRUE;
      break;
   default:
      break;
   }
   return bResult;
}

void SipXRtpRedirectEventListener::sipxFireRtpRedirectEvent(const UtlString& sAbstractCallId,
                                                            SIPX_RTP_REDIRECT_EVENT event,
                                                            SIPX_RTP_REDIRECT_CAUSE cause)
{
   CpRtpRedirectEvent payload(sAbstractCallId, (CP_RTP_REDIRECT_CAUSE)cause);
   RtpRedirectEventMsg msg(event, payload);
   postMessage(msg);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void SipXRtpRedirectEventListener::handleRtpRedirectEvent(const UtlString& sAbstractCallId, 
                                                          SIPX_RTP_REDIRECT_EVENT event, 
                                                          SIPX_RTP_REDIRECT_CAUSE cause)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "handleRtpRedirectEvent");

   OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
      "handleRtpRedirectEvent CallId=%s Event=%s:%s",
      sAbstractCallId.data(),
      sipxRtpRedirectEventToString(event),
      sipxRtpRedirectCauseToString(cause));

   SIPX_CALL hCall = SIPX_CALL_NULL;
   SIPX_INSTANCE_DATA* pSipXInstance = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, m_pInst);
   assert(pSipXInstance);
   SIPX_LINE hLine = SIPX_LINE_NULL;

   hCall = sipxCallLookupHandleByCallId(sAbstractCallId, m_pInst);
   if (hCall != SIPX_CALL_NULL)
   {
      SIPX_CALL_DATA* pCallData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);
      if (pCallData)
      {
         hLine = pCallData->m_hLine;
         // update m_bRtpRedirectState
         pCallData->m_rtpRedirectState = getRtpRedirectState(event);
         sipxCallReleaseLock(pCallData, SIPX_LOCK_READ, stackLogger);
      }

      // now create event for sipXtapi listeners
      SIPX_RTP_REDIRECT_INFO rtpRedirectInfo;
      memset((void*) &rtpRedirectInfo, 0, sizeof(SIPX_RTP_REDIRECT_INFO));

      rtpRedirectInfo.event = event;
      rtpRedirectInfo.cause = cause;
      rtpRedirectInfo.hCall = hCall;
      rtpRedirectInfo.hLine = hLine;
      rtpRedirectInfo.nSize = sizeof(SIPX_RTP_REDIRECT_INFO);

      // fire event
      SipXEventDispatcher::dispatchEvent(m_pInst, EVENT_CATEGORY_RTP_REDIRECT, &rtpRedirectInfo);
   }
   else
   {
      // call was destroyed before event could be fully processed
      OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING,
         "handleRtpRedirectEvent is unable to fully process event. Call %s does not exist anymore.\n",
         sAbstractCallId.data());
   }
}

RTP_REDIRECT_STATE SipXRtpRedirectEventListener::getRtpRedirectState(SIPX_RTP_REDIRECT_EVENT event)
{
   RTP_REDIRECT_STATE state = RTP_REDIRECT_STATE_INACTIVE;

   switch (event)
   {
   case RTP_REDIRECT_ACTIVE:
      state = RTP_REDIRECT_STATE_ACTIVE;
      break;
   case RTP_REDIRECT_REQUESTED:
      state = RTP_REDIRECT_STATE_REQUESTED;
      break;
   case RTP_REDIRECT_ERROR:
      state = RTP_REDIRECT_STATE_ERROR;
      break;
   case RTP_REDIRECT_STOP:
   default:
      state = RTP_REDIRECT_STATE_INACTIVE;
      break;
   }

   return state;
}

/* ============================ FUNCTIONS ================================= */

