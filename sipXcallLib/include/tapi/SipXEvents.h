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

#ifndef SipXEvents_h__
#define SipXEvents_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"
#include "tapi/SipXMediaEventListener.h"
#include "utl/UtlString.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
class SipMessage;
class CpMediaEvent;

// STRUCTS
// TYPEDEFS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

SIPX_RESULT sipxEventListenerAddInternal(const SIPX_INST hInst,
                                         SIPX_EVENT_CALLBACK_PROC pCallbackProc,
                                         void *pUserData);

SIPX_RESULT sipxEventListenerRemoveInternal(const SIPX_INST hInst, 
                                            SIPX_EVENT_CALLBACK_PROC pCallbackProc, 
                                            void* pUserData);

void sipxFireConfigEvent(const SIPX_INST pInst,                                                        
                         SIPX_CONFIG_EVENT event,
                         const SIPX_CONTACT_ADDRESS* pContactAddress = NULL,
                         const SIPX_STUN_FAILURE_INFO* pStunFailureDetails = NULL);

/**
* Fires SipPimClient events
*/
void sipxFirePIMEvent(void* userData,
                      const UtlString& fromAddress,
                      const char* textMessage,
                      int textLength,
                      const char* subject,
                      const SipMessage& messageRequest);

bool sipxFireSubscriptionStatusEvent(const SIPX_INST pInst,
                                     SIPX_SUBSTATUS_INFO* pInfo);

bool sipxFireNotifyEvent(const SIPX_INST pInst,
                         SIPX_NOTIFY_INFO* pInfo);

const char* convertKeepaliveTypeToString(SIPX_KEEPALIVE_TYPE type);
const char* convertCallstateEventToString(SIPX_CALLSTATE_EVENT eMajor);
const char* convertCallstateCauseToString(SIPX_CALLSTATE_CAUSE eMinor);

#endif // SipXEvents_h__
