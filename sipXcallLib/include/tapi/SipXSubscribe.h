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

#ifndef SipXSubscribe_h__
#define SipXSubscribe_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <net/SipSubscribeClient.h>
#include <tapi/SipXCore.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
class SIPX_INSTANCE_DATA;

// STRUCTS
// TYPEDEFS
class SIPX_SUBSCRIPTION_DATA
{
public:
   SIPX_INSTANCE_DATA* pInst;
   UtlString dialogHandle;
   OsMutex mutex;
   
   SIPX_SUBSCRIPTION_DATA() : dialogHandle(NULL),
      mutex(OsMutex::Q_FIFO),
      pInst(NULL)
   {

   }
};

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

SIPX_SUBSCRIPTION_DATA* sipxSubscribeLookup(const SIPX_SUB hSub,
                                            SIPX_LOCK_TYPE type,
                                            const OsStackTraceLogger& oneBackInStack);

void sipxSubscribeReleaseLock(SIPX_SUBSCRIPTION_DATA* pData,
                              SIPX_LOCK_TYPE type,
                              const OsStackTraceLogger& oneBackInStack);

void sipxSubscribeDestroyAll(const SIPX_INST hInst);

/**
* Callback for subscription client NOTIFY content
*/
void sipxSubscribeClientNotifyCallback(const char* earlyDialogHandle,
                                       const char* dialogHandle,
                                       void* applicationData,
                                       const SipMessage* notifyRequest);

/**
* Callback for subscription client state
*/
void sipxSubscribeClientSubCallback(enum SipSubscribeClient::SubscriptionState newState,
                                    const char* earlyDialogHandle,
                                    const char* dialogHandle,
                                    void* applicationData,
                                    int responseCode,
                                    const char* responseText,
                                    long expiration,
                                    const SipMessage* subscribeResponse);

#endif // SipXSubscribe_h__
