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

#ifndef SipXMediaEventListener_h__
#define SipXMediaEventListener_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsSharedServerTask.h>
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"
#include "cp/CpMediaEventListener.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
// STRUCTS
// TYPEDEFS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

SIPX_CODEC_INFO getSipXCodecInfo(const CpCodecInfo& codecInfo);

class SipXMediaEvent
{
public:
   UtlString m_sCallId;
   UtlString m_sSessionCallId;
   UtlString m_sRemoteAddress;
   void* m_pCookie; // only for audio playback start/stop
   int m_playBufferIndex; // only for audio playback start/stop
   SIPX_MEDIA_EVENT m_event;
   SIPX_MEDIA_CAUSE m_cause;
   SIPX_MEDIA_TYPE m_mediaType;

   SIPX_CODEC_INFO m_codec; // only for local audio/video start
   int m_idleTime; // only for RemoteSilent
   SIPX_TONE_ID m_toneId; // only for DTMF event

   SipXMediaEvent() : m_sCallId(NULL)
      , m_sSessionCallId(NULL)
      , m_sRemoteAddress(NULL)
      , m_pCookie(NULL)
      , m_playBufferIndex(0)
      , m_event(MEDIA_UNKNOWN)
      , m_cause(MEDIA_CAUSE_NORMAL)
      , m_mediaType(MEDIA_TYPE_AUDIO)
      , m_idleTime(0)
      , m_toneId(ID_DTMF_0)
   {
      memset(&m_codec, 0, sizeof(SIPX_CODEC_INFO));
   }

   ~SipXMediaEvent()
   {

   }

   SipXMediaEvent(const SipXMediaEvent& event)
   {
      *this = event;
   }

   SipXMediaEvent& operator=(const SipXMediaEvent& event)
   {
      if (&event == this)
      {
         return *this;
      }

      m_sCallId = event.m_sCallId;
      m_sSessionCallId = event.m_sSessionCallId;
      m_sRemoteAddress = event.m_sRemoteAddress;
      m_pCookie = event.m_pCookie;
      m_playBufferIndex = event.m_playBufferIndex;
      m_event = event.m_event;
      m_cause = event.m_cause;
      m_mediaType = event.m_mediaType;

      m_codec = event.m_codec;
      m_idleTime = event.m_idleTime;
      m_toneId = event.m_toneId;

      return *this;
   }

   SipXMediaEvent(const CpMediaEvent& eventPayload, SIPX_MEDIA_EVENT event = MEDIA_UNKNOWN)
   {
      m_sCallId = eventPayload.m_sCallId;
      m_sSessionCallId = eventPayload.m_sSessionCallId;
      m_sRemoteAddress = eventPayload.m_sRemoteAddress;
      m_pCookie = eventPayload.m_pCookie;
      m_playBufferIndex = eventPayload.m_playBufferIndex;
      m_event = event;
      m_cause = (SIPX_MEDIA_CAUSE)eventPayload.m_cause;
      m_mediaType = (SIPX_MEDIA_TYPE)eventPayload.m_mediaType;
      m_codec = getSipXCodecInfo(eventPayload.m_codec);
      m_idleTime = eventPayload.m_idleTime;
      m_toneId = (SIPX_TONE_ID)eventPayload.m_toneId;
   }
};

/**
* Listener for Media events
*/
class SipXMediaEventListener : public OsSharedServerTask, public CpMediaEventListener
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */

   /* ============================ CREATORS ================================== */
public:
   SipXMediaEventListener(SIPX_INST pInst);
   virtual ~SipXMediaEventListener();

   /* ============================ MANIPULATORS ============================== */

   virtual void OnMediaLocalStart(const CpMediaEvent& event);

   virtual void OnMediaLocalStop(const CpMediaEvent& event);

   virtual void OnMediaRemoteStart(const CpMediaEvent& event);

   virtual void OnMediaRemoteStop(const CpMediaEvent& event);

   virtual void OnMediaRemoteSilent(const CpMediaEvent& event);

   virtual void OnMediaPlayfileStart(const CpMediaEvent& event);

   virtual void OnMediaPlayfileStop(const CpMediaEvent& event);

   virtual void OnMediaPlaybufferStart(const CpMediaEvent& event);

   virtual void OnMediaPlaybufferStop(const CpMediaEvent& event);

   virtual void OnMediaPlaybackPaused(const CpMediaEvent& event);

   virtual void OnMediaPlaybackResumed(const CpMediaEvent& event);

   virtual void OnMediaRemoteDTMF(const CpMediaEvent& event);

   virtual void OnMediaDeviceFailure(const CpMediaEvent& event);

   virtual void OnMediaRemoteActive(const CpMediaEvent& event);

   virtual void OnMediaRecordingStart(const CpMediaEvent& event);

   virtual void OnMediaRecordingStop(const CpMediaEvent& event);

   virtual UtlBoolean handleMessage(OsMsg& rRawMsg);

   void sipxFireMediaEvent(const UtlString& sCallId,
                           const UtlString& sSessionCallId,
                           const UtlString& sRemoteAddress,
                           SIPX_MEDIA_EVENT event,
                           SIPX_MEDIA_CAUSE cause,
                           SIPX_MEDIA_TYPE type);

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   SipXMediaEventListener(const SipXMediaEventListener& rhs);

   SipXMediaEventListener& operator=(const SipXMediaEventListener& rhs);

   void handleMediaEvent(const SipXMediaEvent& eventPayload);

   SIPX_INST m_pInst;
};

#endif // SipXMediaEventListener_h__
