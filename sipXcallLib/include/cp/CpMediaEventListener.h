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

#ifndef CpMediaEventListener_h__
#define CpMediaEventListener_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <cp/CpDefs.h>
#include <cp/CpCodecInfo.h>

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

class CpMediaEvent
{
public:
   UtlString m_sCallId;
   UtlString m_sSessionCallId;
   UtlString m_sRemoteAddress;
   void* m_pCookie; // only for audio playback start/stop
   int m_playBufferIndex; // only for audio playback start/stop
   CP_MEDIA_CAUSE m_cause;
   CP_MEDIA_TYPE m_mediaType;

   CpCodecInfo m_codec; // only for local audio/video start
   int m_idleTime; // only for RemoteSilent
   CP_TONE_ID m_toneId; // only for DTMF event

   CpMediaEvent() : m_sCallId(NULL)
      , m_sSessionCallId(NULL)
      , m_sRemoteAddress(NULL)
      , m_pCookie(NULL)
      , m_playBufferIndex(0)
      , m_cause(CP_MEDIA_CAUSE_NORMAL)
      , m_mediaType(CP_MEDIA_TYPE_AUDIO)
      , m_codec()
      , m_idleTime(0)
      , m_toneId(CP_ID_DTMF_0)
   {
   }

   ~CpMediaEvent()
   {
      // do nothing
   }

   CpMediaEvent(const CpMediaEvent& event)
   {
      *this = event;
   }

   CpMediaEvent& operator=(const CpMediaEvent& event)
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
      m_cause = event.m_cause;
      m_mediaType = event.m_mediaType;

      m_codec = event.m_codec;
      m_idleTime = event.m_idleTime;
      m_toneId = event.m_toneId;

      return *this;
   }
};


/**
* Listener for Media events
*/
class CpMediaEventListener
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */

   /* ============================ CREATORS ================================== */
public:
   CpMediaEventListener() {}
   virtual ~CpMediaEventListener() {}

   /* ============================ MANIPULATORS ============================== */

   virtual void OnMediaLocalStart(const CpMediaEvent& event) = 0;

   virtual void OnMediaLocalStop(const CpMediaEvent& event) = 0;

   virtual void OnMediaRemoteStart(const CpMediaEvent& event) = 0;

   virtual void OnMediaRemoteStop(const CpMediaEvent& event) = 0;

   virtual void OnMediaRemoteSilent(const CpMediaEvent& event) = 0;

   virtual void OnMediaPlayfileStart(const CpMediaEvent& event) = 0;

   virtual void OnMediaPlayfileStop(const CpMediaEvent& event) = 0;

   virtual void OnMediaPlaybufferStart(const CpMediaEvent& event) = 0;

   virtual void OnMediaPlaybufferStop(const CpMediaEvent& event) = 0;

   virtual void OnMediaPlaybackPaused(const CpMediaEvent& event) = 0;

   virtual void OnMediaPlaybackResumed(const CpMediaEvent& event) = 0;

   virtual void OnMediaRemoteDTMF(const CpMediaEvent& event) = 0;

   virtual void OnMediaDeviceFailure(const CpMediaEvent& event) = 0;

   virtual void OnMediaRemoteActive(const CpMediaEvent& event) = 0;

   virtual void OnMediaRecordingStart(const CpMediaEvent& event) = 0;

   virtual void OnMediaRecordingStop(const CpMediaEvent& event) = 0;

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // CpMediaEventListener_h__



