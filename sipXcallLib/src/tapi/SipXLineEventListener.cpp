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
#include "tapi/SipXLineEventListener.h"
#include <tapi/LineStateEventMsg.h>
#include "tapi/SipXEvents.h"
#include <tapi/SipXEventDispatcher.h>
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

SipXLineEventListener::SipXLineEventListener( SIPX_INST pInst )
: OsSharedServerTask("SipXLineEventListener-%d")
, SipLineStateEventListener()
, m_pInst(pInst)
{

}

SipXLineEventListener::~SipXLineEventListener()
{
   release();
}

/* ============================ MANIPULATORS ============================== */

void SipXLineEventListener::OnLineRegistering(const SipLineStateEvent& event)
{
   LineStateEventMsg msg(LINESTATE_REGISTERING, event);
   postMessage(msg);
}

void SipXLineEventListener::OnLineRegistered(const SipLineStateEvent& event)
{
   LineStateEventMsg msg(LINESTATE_REGISTERED, event);
   postMessage(msg);
}

void SipXLineEventListener::OnLineUnregistering(const SipLineStateEvent& event)
{
   LineStateEventMsg msg(LINESTATE_UNREGISTERING, event);
   postMessage(msg);
}

void SipXLineEventListener::OnLineUnregistered(const SipLineStateEvent& event)
{
   LineStateEventMsg msg(LINESTATE_UNREGISTERED, event);
   postMessage(msg);
}

void SipXLineEventListener::OnLineRegisterFailed(const SipLineStateEvent& event)
{
   LineStateEventMsg msg(LINESTATE_REGISTER_FAILED, event);
   postMessage(msg);
}

void SipXLineEventListener::OnLineUnregisterFailed(const SipLineStateEvent& event)
{
   LineStateEventMsg msg(LINESTATE_UNREGISTER_FAILED, event);
   postMessage(msg);
}

void SipXLineEventListener::OnLineProvisioned(const SipLineStateEvent& event)
{
   LineStateEventMsg msg(LINESTATE_PROVISIONED, event);
   postMessage(msg);
}

UtlBoolean SipXLineEventListener::handleMessage(OsMsg& rRawMsg)
{
   UtlBoolean bResult = FALSE;

   switch (rRawMsg.getMsgType())
   {
   case LINESTATEEVENT_MSG:
      {
         LineStateEventMsg* pMsg = dynamic_cast<LineStateEventMsg*>(&rRawMsg);
         if (pMsg)
         {
            // cast succeeded
            const SipLineStateEvent& payload = pMsg->getEventPayloadRef();
            handleLineEvent(payload.m_sLineId, pMsg->getEvent(), (SIPX_LINESTATE_CAUSE)payload.m_cause, payload.m_responseCode,
               payload.m_sResponseText);
         }
      }
      bResult = TRUE;
      break;
   default:
      break;
   }
   return bResult;
}

void SipXLineEventListener::sipxFireLineEvent(const UtlString& lineIdentifier,
                                              SIPX_LINESTATE_EVENT event,
                                              SIPX_LINESTATE_CAUSE cause,
                                              int sipResponseCode /*= 0*/,
                                              const UtlString& sResponseText /*= NULL*/)
{
   SipLineStateEvent payload(lineIdentifier, (SIPXTACK_LINESTATE_CAUSE)cause, sipResponseCode, sResponseText);
   LineStateEventMsg msg(event, payload);
   postMessage(msg);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void SipXLineEventListener::handleLineEvent(const UtlString& lineIdentifier,
                                            SIPX_LINESTATE_EVENT event,
                                            SIPX_LINESTATE_CAUSE cause,
                                            int sipResponseCode,
                                            const UtlString& sResponseText)
{
   OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
      "handleLineEvent pSrc=%p szLineIdentifier=%s event=%d cause=%d",
      m_pInst, lineIdentifier.data(), event, cause);

   SIPX_LINE_DATA* pLineData = NULL;
   SIPX_LINE hLine = SIPX_LINE_NULL;

   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, m_pInst);
   hLine = sipxLineLookupHandleByURI(pInst, lineIdentifier);

   // fire event even if line doesn't exist anymore - was destroyed, in that case
   // hLine will be SIPX_LINE_NULL, we can not wait with deletion until LINESTATE_UNREGISTERED
   // is received, as server could not respond
   SIPX_LINESTATE_INFO lineInfo;
   memset((void*) &lineInfo, 0, sizeof(SIPX_LINESTATE_INFO));

   lineInfo.nSize = sizeof(SIPX_LINESTATE_INFO);
   lineInfo.hLine = hLine;
   lineInfo.szLineUri = lineIdentifier;
   lineInfo.event = event;
   lineInfo.cause = cause;
   lineInfo.sipResponseCode = sipResponseCode;
   lineInfo.szSipResponseText = sResponseText.data(); // safe to do, as dispatcher makes a copy

   SipXEventDispatcher::dispatchEvent(m_pInst, EVENT_CATEGORY_LINESTATE, &lineInfo);

}

/* ============================ FUNCTIONS ================================= */

