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

#ifndef SipXPublish_h__
#define SipXPublish_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsRWMutex.h>
#include "tapi/SipXCore.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
class SIPX_INSTANCE_DATA;

// STRUCTS
// TYPEDEFS
class SIPX_PUBLISH_DATA
{
public:
   SIPX_INSTANCE_DATA* pInst;
   UtlString resourceId;
   UtlString eventType;
   HttpBody content;
   OsMutex mutex;

   SIPX_PUBLISH_DATA() : pInst(NULL),
      resourceId(NULL),
      eventType(NULL),
      mutex(OsMutex::Q_FIFO),
      content()
   {

   }
};

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

SIPX_PUBLISH_DATA* sipxPublishLookup(const SIPX_PUB hPub,
                                     SIPX_LOCK_TYPE type,
                                     const OsStackTraceLogger& oneBackInStack);

void sipxPublishReleaseLock(SIPX_PUBLISH_DATA* pData,
                            SIPX_LOCK_TYPE type,
                            const OsStackTraceLogger& oneBackInStack);

void sipxPublisherDestroyAll(const SIPX_INST hInst);

#endif // SipXPublish_h__
