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

#ifndef SipXCall_h__
#define SipXCall_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <net/Url.h>
#include <net/SipDialog.h>
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"
#include "tapi/SipXCore.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
class SIPX_INSTANCE_DATA;
class OsStackTraceLogger;

typedef enum
{
   RTP_REDIRECT_STATE_INACTIVE = 0,
   RTP_REDIRECT_STATE_ACTIVE,
   RTP_REDIRECT_STATE_REQUESTED,
   RTP_REDIRECT_STATE_ERROR
} RTP_REDIRECT_STATE;

// STRUCTS
// TYPEDEFS
class SIPX_CALL_DATA
{
public:
   UtlString m_abstractCallId; ///< Id identifying XCpAbstractCall instance. For conference calls this is conference id.
   UtlString m_splitCallId; ///< temporary Id for conference call splitting. Identifies new call which will receive split call.
   Url m_fullLineUrl; ///< like lineURI, but with display name, field parameters or brackets. Can be used for new out of dialog requests.
   Url m_lineUri; ///< URI of line. Copy of m_lineURI from SIPX_LINE_DATA. This one will never contain a tag. Stored here to avoid line lookups.
   SipDialog m_sipDialog; ///< sip dialog of this call
   SIPX_LINE m_hLine;
   SIPX_INSTANCE_DATA* m_pInst;
   OsMutex m_mutex;
   SIPX_CONF m_hConf;
   RTP_REDIRECT_STATE m_rtpRedirectState; ///< state of RTP redirecting. If active then call cannot be used in conference.
   SIPX_SECURITY_ATTRIBUTES m_security;
   SIPX_VIDEO_DISPLAY m_display;
   SIPX_CALLSTATE_EVENT m_callState; ///< current call state
   SIPX_CALLSTATE_CAUSE m_callStateCause; ///< cause of current call state
   UtlBoolean m_bInFocus; ///< TRUE when call is in focus

   SIPX_CALL_DATA()
      : m_mutex(OsMutex::Q_FIFO),
      m_abstractCallId(NULL),
      m_splitCallId(NULL),
      m_lineUri(),
      m_sipDialog(NULL),
      m_hLine(0),
      m_pInst(NULL),
      m_hConf(NULL),
      m_rtpRedirectState(RTP_REDIRECT_STATE_INACTIVE),
      m_callState(CALLSTATE_UNKNOWN),
      m_callStateCause(CALLSTATE_CAUSE_UNKNOWN),
      m_bInFocus(false)
   {

   }
};

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

void sipxCallReleaseLock(SIPX_CALL_DATA* pData,
                         SIPX_LOCK_TYPE type,
                         const OsStackTraceLogger& oneBackInStack);

UtlBoolean validCallData(SIPX_CALL_DATA* pData);

SIPX_CALL_DATA* sipxCallLookup(const SIPX_CALL hCall,
                               SIPX_LOCK_TYPE type,
                               const OsStackTraceLogger& oneBackInStack);

void sipxCallObjectFree(const SIPX_CALL hCall, const OsStackTraceLogger& oneBackInStack);

SIPX_CALL sipxCallLookupHandleBySessionCallId(const UtlString& sessionCallID, SIPX_INST pInst);
SIPX_CALL sipxCallLookupHandleByCallId(const UtlString& callID, SIPX_INST pInst);

void destroyCallData(SIPX_CALL_DATA* pData);

UtlBoolean sipxCallGetState(SIPX_CALL hCall, 
                            SIPX_CALLSTATE_EVENT& lastEvent,
                            SIPX_CALLSTATE_CAUSE& lastCause);

UtlBoolean sipxCallGetCommonData(SIPX_CALL hCall,
                                 SIPX_INSTANCE_DATA** pInst,
                                 UtlString* pCallId,
                                 UtlString* pSessionCallId,
                                 UtlString* pRemoteField,
                                 UtlString* pLocalField,
                                 UtlString* pGhostCallId = NULL, 
                                 UtlString* pRemoteContactAddress = NULL);

/**
* Get the list of active calls for the specified sipxtapi instance.
*/
SIPXTAPI_API SIPX_RESULT sipxGetAllAbstractCallIds(SIPX_INST hInst, UtlSList& idList);

void sipxCallDestroyAll(const SIPX_INST hInst);

SIPX_CONF sipxCallGetConf(SIPX_CALL hCall);

void sipxCallSetConf(SIPX_CALL hCall, SIPX_CONF hConf);

SIPX_CONTACT_TYPE sipxCallGetLineContactType(SIPX_CALL hCall);

SIPX_RESULT sipxCallCreateHelper(const SIPX_INST hInst,
                                 const SIPX_LINE hLine,
                                 const char* szLine,
                                 const SIPX_CONF hConf,
                                 SIPX_CALL* phCall,
                                 const UtlString& sCallId = "",
                                 const UtlString& sSessionCallId = "",
                                 bool bIsConferenceCall = false);


SIPX_RESULT sipxCallDrop(SIPX_CALL& hCall);

UtlBoolean sipxCallSetState(SIPX_CALL hCall, 
                            SIPX_CALLSTATE_EVENT event,
                            SIPX_CALLSTATE_CAUSE cause);

UtlBoolean sipxCallSetAbstractCallId(SIPX_CALL hCall,
                                     const UtlString& sAbstractCallId);

#endif // SipXCall_h__
