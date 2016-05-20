//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
//
// Copyright (C) 2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006 Robert J. Andreasen, Jr.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _SIPXEVENTDISPATCHER_H_
#define _SIPXEVENTDISPATCHER_H_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsServerTask.h"
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"
#include "utl/UtlHashMap.h"
#include "os/OsRWMutex.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS

// FORWARD DECLARATIONS
class OsEventMsg;

/**
 * The SipXEventDispatcher adds a listener to sipXtapi and then redispatches
 * all of the sipXtapi events on its own callback.  Ideally, this should be 
 * part of sipXtapi -- however, I don't want to change the behavior at this 
 * time.  Also, solutions for the TODOs below are needed.
 *
 * TODO: 
 *
 * - The S/MIME (and TLS?) code expects a return value.  This should be broken
 *   into a new/different callback.  Right now, that functionality is lost
 *   and I believe we will accept all certificates (return true at end of 
 *   SipXEventDispatcher::EventCallBack.
 *
 * - Some synchronization is required on the CALL_DESTROY (perhaps other
 *   final events).  Right now, the call could be removed from sipXtapi's
 *   internal bookkeeping structures before the application has the ability
 *   to handle the event.  I believe we should have some sort of "ACK" to the 
 *   call final events (perhaps after it is dispatched from here) and the
 *   call should not be freed up in sipXtapi until after that ACK.  Today,
 *   the call handle is freed in the fire***Event method in 
 *   sipXtapiEvents.cpp.
 */
class SipXEventDispatcher : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
  
    static void initDispatcher();

    static void shutdownDispatcher();

/* ============================ MANIPULATORS ============================== */

    /**
     * Implementation of OsServerTask's pure virtual method
     */
    UtlBoolean handleMessage(OsMsg& rMsg);

    static UtlBoolean addListener(const SIPX_INST hInst,
                                  SIPX_EVENT_CALLBACK_PROC  pCallbackProc,
                                  void*                     pUserData);

    static UtlBoolean removeListener(const SIPX_INST hInst,
                                     SIPX_EVENT_CALLBACK_PROC  pCallbackProc,
                                     void*                     pUserData);

    static void removeAllListeners(const SIPX_INST hInst);

    static void dispatchEvent(const SIPX_INST hInst,
                              SIPX_EVENT_CATEGORY category, 
                              void*               pInfo);
/* ============================ INQUIRY =================================== */

    static int getListenerCount(const SIPX_INST hInst);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   SipXEventDispatcher();

   virtual ~SipXEventDispatcher(void);

   SipXEventDispatcher(const SipXEventDispatcher& rhs);

   SipXEventDispatcher& operator=(const SipXEventDispatcher& rhs);

   void serviceListeners(const SIPX_INST hInst,
                         SIPX_EVENT_CATEGORY category, 
                         void*               pInfo);

   static SipXEventDispatcher* m_spInstance;
   static OsRWMutex m_InstanceLock;
   static int m_sCounter;

   static OsRWMutex m_MemberLock;
   static UtlHashMap m_Listeners;
};

#endif // _SIPXEVENTDISPATCHER_H_

