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
#include "tapi/SipXMediaEventListener.h"
#include <tapi/MediaEventMsg.h>
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

SipXMediaEventListener::SipXMediaEventListener(SIPX_INST pInst)
: OsSharedServerTask("SipXMediaEventListener-%d")
, CpMediaEventListener()
, m_pInst(pInst)
{

}

SipXMediaEventListener::~SipXMediaEventListener()
{
   release();
}

/* ============================ MANIPULATORS ============================== */

void SipXMediaEventListener::OnMediaLocalStart(const CpMediaEvent& event)
{
   SipXMediaEvent eventPayload(event, MEDIA_LOCAL_START);
   MediaEventMsg msg(eventPayload);
   postMessage(msg);
}

void SipXMediaEventListener::OnMediaLocalStop(const CpMediaEvent& event)
{
   SipXMediaEvent eventPayload(event, MEDIA_LOCAL_STOP);
   MediaEventMsg msg(eventPayload);
   postMessage(msg);
}

void SipXMediaEventListener::OnMediaRemoteStart(const CpMediaEvent& event)
{
   SipXMediaEvent eventPayload(event, MEDIA_REMOTE_START);
   MediaEventMsg msg(eventPayload);
   postMessage(msg);
}

void SipXMediaEventListener::OnMediaRemoteStop(const CpMediaEvent& event)
{
   SipXMediaEvent eventPayload(event, MEDIA_REMOTE_STOP);
   MediaEventMsg msg(eventPayload);
   postMessage(msg);
}

void SipXMediaEventListener::OnMediaRemoteSilent(const CpMediaEvent& event)
{
   SipXMediaEvent eventPayload(event, MEDIA_REMOTE_SILENT);
   MediaEventMsg msg(eventPayload);
   postMessage(msg);
}

void SipXMediaEventListener::OnMediaPlayfileStart(const CpMediaEvent& event)
{
   SipXMediaEvent eventPayload(event, MEDIA_PLAYFILE_START);
   MediaEventMsg msg(eventPayload);
   postMessage(msg);
}

void SipXMediaEventListener::OnMediaPlayfileStop(const CpMediaEvent& event)
{
   SipXMediaEvent eventPayload(event, MEDIA_PLAYFILE_STOP);
   MediaEventMsg msg(eventPayload);
   postMessage(msg);
}

void SipXMediaEventListener::OnMediaPlaybufferStart(const CpMediaEvent& event)
{
   SipXMediaEvent eventPayload(event, MEDIA_PLAYBUFFER_START);
   MediaEventMsg msg(eventPayload);
   postMessage(msg);
}

void SipXMediaEventListener::OnMediaPlaybufferStop(const CpMediaEvent& event)
{
   SipXMediaEvent eventPayload(event, MEDIA_PLAYBUFFER_STOP);
   MediaEventMsg msg(eventPayload);
   postMessage(msg);
}

void SipXMediaEventListener::OnMediaPlaybackPaused(const CpMediaEvent& event)
{
   SipXMediaEvent eventPayload(event, MEDIA_PLAYBACK_PAUSED);
   MediaEventMsg msg(eventPayload);
   postMessage(msg);
}

void SipXMediaEventListener::OnMediaPlaybackResumed(const CpMediaEvent& event)
{
   SipXMediaEvent eventPayload(event, MEDIA_PLAYBACK_RESUMED);
   MediaEventMsg msg(eventPayload);
   postMessage(msg);
}

void SipXMediaEventListener::OnMediaRemoteDTMF(const CpMediaEvent& event)
{
   SipXMediaEvent eventPayload(event, MEDIA_REMOTE_DTMF);
   MediaEventMsg msg(eventPayload);
   postMessage(msg);
}

void SipXMediaEventListener::OnMediaDeviceFailure(const CpMediaEvent& event)
{
   SipXMediaEvent eventPayload(event, MEDIA_DEVICE_FAILURE);
   MediaEventMsg msg(eventPayload);
   postMessage(msg);
}

void SipXMediaEventListener::OnMediaRemoteActive(const CpMediaEvent& event)
{
   SipXMediaEvent eventPayload(event, MEDIA_REMOTE_ACTIVE);
   MediaEventMsg msg(eventPayload);
   postMessage(msg);
}

void SipXMediaEventListener::OnMediaRecordingStart(const CpMediaEvent& event)
{
   SipXMediaEvent eventPayload(event, MEDIA_RECORDING_START);
   MediaEventMsg msg(eventPayload);
   postMessage(msg);
}

void SipXMediaEventListener::OnMediaRecordingStop(const CpMediaEvent& event)
{
   SipXMediaEvent eventPayload(event, MEDIA_RECORDING_STOP);
   MediaEventMsg msg(eventPayload);
   postMessage(msg);
}

UtlBoolean SipXMediaEventListener::handleMessage(OsMsg& rRawMsg)
{
   UtlBoolean bResult = FALSE;

   switch (rRawMsg.getMsgType())
   {
   case MEDIAEVENT_MSG:
      {
         MediaEventMsg* pMsg = dynamic_cast<MediaEventMsg*>(&rRawMsg);
         if (pMsg)
         {
            // cast succeeded
            const SipXMediaEvent& payload = pMsg->getEventPayloadRef();
            handleMediaEvent(payload);
         }
      }
      bResult = TRUE;
      break;
   default:
      break;
   }
   return bResult;
}

void SipXMediaEventListener::sipxFireMediaEvent(const UtlString& sCallId,
                                                const UtlString& sSessionCallId,
                                                const UtlString& sRemoteAddress,
                                                SIPX_MEDIA_EVENT event,
                                                SIPX_MEDIA_CAUSE cause,
                                                SIPX_MEDIA_TYPE type)
{
   SipXMediaEvent eventPayload;
   eventPayload.m_event = event;
   eventPayload.m_cause = cause;
   eventPayload.m_mediaType = type;
   eventPayload.m_sCallId = sCallId;
   eventPayload.m_sSessionCallId = sSessionCallId;
   eventPayload.m_sRemoteAddress = sRemoteAddress;

   MediaEventMsg msg(eventPayload);
   postMessage(msg);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void SipXMediaEventListener::handleMediaEvent(const SipXMediaEvent& eventPayload)
{
   OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
      "handleMediaEvent Src=%p CallId=%s RemoteAddress=%s Event=%s:%s type=%d",
      m_pInst,
      eventPayload.m_sCallId.data(),
      eventPayload.m_sRemoteAddress.data(),
      sipxMediaEventToString(eventPayload.m_event),
      sipxMediaCauseToString(eventPayload.m_cause),
      eventPayload.m_mediaType);

   SIPX_MEDIA_EVENT event = eventPayload.m_event;
   SIPX_CALL hCall = sipxCallLookupHandleBySessionCallId(eventPayload.m_sSessionCallId, m_pInst);

   if (hCall != SIPX_CALL_NULL)
   {
      SIPX_MEDIA_INFO mediaInfo;
      memset(&mediaInfo, 0, sizeof(mediaInfo));
      mediaInfo.nSize = sizeof(SIPX_MEDIA_INFO);
      mediaInfo.event = event;
      mediaInfo.cause = eventPayload.m_cause;
      mediaInfo.mediaType = eventPayload.m_mediaType;
      mediaInfo.hCall = hCall;
      mediaInfo.pCookie = eventPayload.m_pCookie;
      mediaInfo.playBufferIndex = eventPayload.m_playBufferIndex;
      mediaInfo.codec = eventPayload.m_codec;
      mediaInfo.idleTime = eventPayload.m_idleTime;
      mediaInfo.toneId = eventPayload.m_toneId;

      SipXEventDispatcher::dispatchEvent((SIPX_INST)m_pInst, EVENT_CATEGORY_MEDIA, &mediaInfo);
   }
   else
   {
      OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING, "Media event received but call was not found for CallId=%s", eventPayload.m_sSessionCallId.data());
   }
}

/* ============================ FUNCTIONS ================================= */


SIPX_CODEC_INFO getSipXCodecInfo(const CpCodecInfo& codecInfo)
{
   SIPX_CODEC_INFO sipxCodecInfo;
   memset(&sipxCodecInfo, 0, sizeof(SIPX_CODEC_INFO));

   sipxCodecInfo.bIsEncrypted = codecInfo.m_bIsEncrypted;
   SAFE_STRNCPY(sipxCodecInfo.cAudioCodecName, codecInfo.m_audioCodec.m_codecName.data(), SIPXTAPI_STRING_MEDIUM_LENGTH);
   SAFE_STRNCPY(sipxCodecInfo.cVideoCodecName, codecInfo.m_videoCodec.m_codecName.data(), SIPXTAPI_STRING_MEDIUM_LENGTH);

   return sipxCodecInfo;
}
