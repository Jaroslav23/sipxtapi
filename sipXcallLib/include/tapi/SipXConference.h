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

#ifndef SipXConference_h__
#define SipXConference_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include "tapi/SipXCore.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
class SIPX_INSTANCE_DATA;
class OsStackTraceLogger;

// STRUCTS
// TYPEDEFS
class SIPX_CONF_DATA
{
public:
   UtlString           m_sConferenceId;
   SIPX_INSTANCE_DATA* m_pInst;
   UtlSList            m_hCalls;
   CONF_HOLD_STATE     m_confHoldState;
   OsMutex             m_mutex;

   SIPX_CONF_DATA() : m_pInst(NULL),
      m_sConferenceId(NULL),
      m_confHoldState(CONF_STATE_UNHELD),
      m_mutex(OsMutex::Q_FIFO)
   {
   }

   ~SIPX_CONF_DATA()
   {
      m_hCalls.destroyAll();
   }

};

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

SIPX_CONF sipxConfLookupHandleByConfId(const UtlString& confID, SIPX_INST pInst);

SIPX_CONF_DATA* sipxConfLookup(const SIPX_CONF hConf,
                               SIPX_LOCK_TYPE type,
                               const OsStackTraceLogger& oneBackInStack);

void sipxConfReleaseLock(SIPX_CONF_DATA* pData,
                         SIPX_LOCK_TYPE type,
                         const OsStackTraceLogger& oneBackInStack);

void sipxConfFree(const SIPX_CONF hConf);

void sipxConferenceDestroyAll(const SIPX_INST hInst);

UtlBoolean validConfData(const SIPX_CONF_DATA* pData);

UtlBoolean sipxAddCallHandleToConf(const SIPX_CALL hCall,
                                   const SIPX_CONF hConf);

UtlBoolean sipxRemoveCallHandleFromConf(const SIPX_CONF hConf,
                                        const SIPX_CALL hCall);
#endif // SipXConference_h__
