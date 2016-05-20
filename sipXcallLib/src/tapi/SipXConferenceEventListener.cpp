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
#include "tapi/SipXConferenceEventListener.h"
#include <tapi/ConferenceEventMsg.h>
#include <tapi/SipXEventDispatcher.h>
#include "tapi/SipXEvents.h"
#include <tapi/SipXCore.h>
#include <tapi/SipXCall.h>
#include <tapi/SipXConference.h>
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

SipXConferenceEventListener::SipXConferenceEventListener( SIPX_INST pInst )
: OsSharedServerTask("SipXConferenceEventListener-%d")
, CpConferenceEventListener()
, m_pInst(pInst)
{

}

SipXConferenceEventListener::~SipXConferenceEventListener()
{
   release();
}

/* ============================ MANIPULATORS ============================== */

void SipXConferenceEventListener::OnConferenceCreated(const CpConferenceEvent& event)
{
   ConferenceEventMsg msg(CONFERENCE_CREATED, event);
   postMessage(msg);
}

void SipXConferenceEventListener::OnConferenceDestroyed(const CpConferenceEvent& event)
{
   ConferenceEventMsg msg(CONFERENCE_DESTROYED, event);
   postMessage(msg);
}

void SipXConferenceEventListener::OnConferenceCallAdded(const CpConferenceEvent& event)
{
   ConferenceEventMsg msg(CONFERENCE_CALL_ADDED, event);
   postMessage(msg);
}

void SipXConferenceEventListener::OnConferenceCallAddFailure(const CpConferenceEvent& event)
{
   ConferenceEventMsg msg(CONFERENCE_CALL_ADD_FAILURE, event);
   postMessage(msg);
}

void SipXConferenceEventListener::OnConferenceCallRemoved(const CpConferenceEvent& event)
{
   ConferenceEventMsg msg(CONFERENCE_CALL_REMOVED, event);
   postMessage(msg);
}

void SipXConferenceEventListener::OnConferenceCallRemoveFailure(const CpConferenceEvent& event)
{
   ConferenceEventMsg msg(CONFERENCE_CALL_REMOVE_FAILURE, event);
   postMessage(msg);
}

UtlBoolean SipXConferenceEventListener::handleMessage(OsMsg& rRawMsg)
{
   UtlBoolean bResult = FALSE;

   switch (rRawMsg.getMsgType())
   {
   case CONFERENCEEVENT_MSG:
      {
         ConferenceEventMsg* pMsg = dynamic_cast<ConferenceEventMsg*>(&rRawMsg);
         if (pMsg)
         {
            // cast succeeded
            const CpConferenceEvent& payload = pMsg->getEventPayloadRef();
            handleConferenceEvent(pMsg->getEvent(), (SIPX_CONFERENCE_CAUSE)payload.m_cause,
               payload.m_sConferenceId, payload.m_pSipDialog);
         }
      }
      bResult = TRUE;
      break;
   default:
      break;
   }
   return bResult;
}

void SipXConferenceEventListener::sipxFireConferenceEvent(SIPX_CONFERENCE_EVENT event,
                                                          SIPX_CONFERENCE_CAUSE cause,
                                                          const UtlString& sConferenceId,
                                                          const SipDialog* pSipDialog)
{
   CpConferenceEvent payload((CP_CONFERENCE_CAUSE)cause, sConferenceId, pSipDialog);
   ConferenceEventMsg msg(event, payload);
   postMessage(msg);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void SipXConferenceEventListener::handleConferenceEvent(SIPX_CONFERENCE_EVENT event,
                                                        SIPX_CONFERENCE_CAUSE cause,
                                                        const UtlString& sConferenceId,
                                                        const SipDialog* pSipDialog)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "handleConferenceEvent");

   UtlString sSipCallId;
   if (pSipDialog)
   {
      pSipDialog->getCallId(sSipCallId);
   }

   OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
      "handleConferenceEvent ConferenceId=%s CallId=%s Event=%s:%s",
      sConferenceId.data(), sSipCallId.data(),
      sipxConferenceEventToString(event),
      sipxConferenceCauseToString(cause));

   SIPX_CONF hConf = SIPX_CONF_NULL;
   SIPX_CALL hCall = SIPX_CALL_NULL;
   SIPX_INSTANCE_DATA* pSipXInstance = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, m_pInst);
   assert(pSipXInstance);

   hConf = sipxConfLookupHandleByConfId(sConferenceId, m_pInst);
   if (hConf != SIPX_CONF_NULL)
   {
      if (!sSipCallId.isNull())
      {
         hCall = sipxCallLookupHandleBySessionCallId(sSipCallId, m_pInst);
      }

      // now create event for sipXtapi listeners
      SIPX_CONFERENCE_INFO conferenceInfo;
      memset((void*) &conferenceInfo, 0, sizeof(SIPX_CONFERENCE_INFO));

      conferenceInfo.event = event;
      conferenceInfo.cause = cause;
      conferenceInfo.hConf = hConf;
      conferenceInfo.hCall = hCall;
      conferenceInfo.nSize = sizeof(SIPX_RTP_REDIRECT_INFO);

      if (event != CONFERENCE_DESTROYED && event != CONFERENCE_CREATED && hCall == SIPX_CALL_NULL)
      {
         OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
            "handleConferenceEvent: unable to find call for conference event for conference %s.\n",
            sConferenceId.data());
      }

      switch (event)
      {
         case CONFERENCE_DESTROYED:
            sipxConfFree(hConf);
            break;
         case CONFERENCE_CALL_REMOVED:
            {
               // remove call from conference
               sipxRemoveCallHandleFromConf(hConf, hCall);

               SIPX_CALL_DATA* pCallData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);
               if (pCallData)
               {
                  if (!pCallData->m_abstractCallId.isNull() && !pCallData->m_splitCallId.isNull())
                  {
                     // for conference split, m_abstractCallId needs to be updated from m_splitCallId
                     pCallData->m_abstractCallId = pCallData->m_splitCallId;
                     pCallData->m_splitCallId.clear();
                  }
                  sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE, stackLogger);
               }
               break;
            }
         case CONFERENCE_CALL_REMOVE_FAILURE:
            {
               SIPX_CALL_DATA* pCallData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);
               if (pCallData)
               {
                  // call is still in conference when remove failed, clear m_splitCallId
                  pCallData->m_splitCallId.clear();
                  sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE, stackLogger);
               }
               break;
            }
         case CONFERENCE_CALL_ADDED:
            {
               if (hCall != SIPX_CALL_NULL)
               {
                  sipxAddCallHandleToConf(hCall, hConf);
                  sipxCallSetAbstractCallId(hCall, sConferenceId);
               }
               break;
            }
         default:
            ;
      }

      // fire event
      SipXEventDispatcher::dispatchEvent(m_pInst, EVENT_CATEGORY_CONFERENCE, &conferenceInfo);
   }
   else
   {
      // call was destroyed before event could be fully processed
      OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING,
         "handleConferenceEvent is unable to fully process event. Conference %s does not exist anymore.\n",
         sConferenceId.data());
   }
}

/* ============================ FUNCTIONS ================================= */

