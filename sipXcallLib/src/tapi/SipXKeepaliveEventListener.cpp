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
#include "tapi/SipXKeepaliveEventListener.h"
#include <tapi/KeepaliveEventMsg.h>
#include "tapi/sipXtapiEvents.h"
#include "tapi/SipXEvents.h"
#include <tapi/SipXEventDispatcher.h>

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

SipXKeepaliveEventListener::SipXKeepaliveEventListener(SIPX_INST pInst)
: OsSharedServerTask("SipXKeepaliveEventListener-%d")
, OsNatKeepaliveListener()
{
   m_pInst = pInst;
}

SipXKeepaliveEventListener::~SipXKeepaliveEventListener()
{
   release();
}

/* ============================ MANIPULATORS ============================== */


void SipXKeepaliveEventListener::OnKeepaliveStart(const OsNatKeepaliveEvent& event)
{
   KeepaliveEventMsg msg(KEEPALIVE_START, KEEPALIVE_CAUSE_NORMAL, event);
   postMessage(msg);
}

void SipXKeepaliveEventListener::OnKeepaliveStop(const OsNatKeepaliveEvent& event)
{
   KeepaliveEventMsg msg(KEEPALIVE_STOP, KEEPALIVE_CAUSE_NORMAL, event);
   postMessage(msg);
}

void SipXKeepaliveEventListener::OnKeepaliveFeedback(const OsNatKeepaliveEvent& event)
{
   KeepaliveEventMsg msg(KEEPALIVE_FEEDBACK, KEEPALIVE_CAUSE_NORMAL, event);
   postMessage(msg);
}

void SipXKeepaliveEventListener::OnKeepaliveFailure(const OsNatKeepaliveEvent& event)
{
   KeepaliveEventMsg msg(KEEPALIVE_FAILURE, KEEPALIVE_CAUSE_NORMAL, event);
   postMessage(msg);
}

UtlBoolean SipXKeepaliveEventListener::handleMessage(OsMsg& rRawMsg)
{
   UtlBoolean bResult = FALSE;

   switch (rRawMsg.getMsgType())
   {
   case KEEPALIVE_MSG:
      {
         KeepaliveEventMsg* pMsg = dynamic_cast<KeepaliveEventMsg*>(&rRawMsg);
         if (pMsg)
         {
            // cast succeeded
            const OsNatKeepaliveEvent& payload = pMsg->getEventPayloadRef();
            handleKeepaliveEvent(pMsg->getEvent(), pMsg->getEventCause(),
               (SIPX_KEEPALIVE_TYPE)payload.type, payload.remoteAddress, payload.remotePort,
               payload.keepAliveSecs, payload.mappedAddress, payload.mappedPort);
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

void SipXKeepaliveEventListener::handleKeepaliveEvent(SIPX_KEEPALIVE_EVENT event,
                                                      SIPX_KEEPALIVE_CAUSE cause,
                                                      SIPX_KEEPALIVE_TYPE type,
                                                      const char* szRemoteAddress,
                                                      int remotePort,
                                                      int keepAliveSecs,
                                                      const char* szFeedbackAddress,
                                                      int feedbackPort)
{
   OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
      "handleKeepaliveEvent src=%p event=%s:%s type=%s remote=%s:%d keepalive=%ds mapped=%s:%d\n",
      m_pInst, 
      sipxKeepaliveEventToString(event),
      sipxKeepaliveCauseToString(cause), 
      convertKeepaliveTypeToString(type),
      szRemoteAddress ? szRemoteAddress : "",
      remotePort, 
      keepAliveSecs,
      szFeedbackAddress ? szFeedbackAddress : "",
      feedbackPort);

   SIPX_KEEPALIVE_INFO keepaliveInfo;
   memset(&keepaliveInfo, 0, sizeof(SIPX_KEEPALIVE_INFO));

   keepaliveInfo.nSize = sizeof(SIPX_KEEPALIVE_INFO);
   keepaliveInfo.event = event;
   keepaliveInfo.cause = cause;
   keepaliveInfo.type = type;
   keepaliveInfo.szRemoteAddress = SAFE_STRDUP(szRemoteAddress);
   keepaliveInfo.remotePort = remotePort;
   keepaliveInfo.keepAliveSecs = keepAliveSecs;
   keepaliveInfo.szFeedbackAddress = SAFE_STRDUP(szFeedbackAddress);
   keepaliveInfo.feedbackPort = feedbackPort;

   SipXEventDispatcher::dispatchEvent(m_pInst, EVENT_CATEGORY_KEEPALIVE, &keepaliveInfo);

   // free doesn't mind NULL value
   free((void*)keepaliveInfo.szRemoteAddress);
   free((void*)keepaliveInfo.szFeedbackAddress);
}

/* ============================ FUNCTIONS ================================= */


