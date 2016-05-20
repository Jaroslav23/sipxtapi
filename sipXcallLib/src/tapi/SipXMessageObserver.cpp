//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "tapi/SipXMessageObserver.h"
#include <tapi/SipXCore.h>
#include "tapi/SipXEvents.h"
#include "tapi/SipXEventDispatcher.h"
#include "net/SipUserAgent.h"
#include <net/SipContact.h>
#include "os/OsEventMsg.h"
#include <os/OsStunResultFailureMsg.h>
#include <os/OsStunResultSuccessMsg.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */


SipXMessageObserver::SipXMessageObserver(SIPX_INSTANCE_DATA* pInst)
: OsServerTask("SipXMessageObserver%d", NULL, 2000)
, m_pInst(pInst)
{
}


SipXMessageObserver::~SipXMessageObserver(void)
{
   waitUntilShutDown();
}

/* ============================ MANIPULATORS ============================== */


UtlBoolean SipXMessageObserver::handleMessage(OsMsg& rMsg)
{
   UtlBoolean bRet = FALSE;
   unsigned char msgType = rMsg.getMsgType();
   unsigned char msgSubType = rMsg.getMsgSubType();

   // Stun event notification
   if (msgType == OsMsg::OS_STUN_RESULT_MSG)
   {
      const OsStunResultMsg* pResultMsg = dynamic_cast<const OsStunResultMsg*>(&rMsg);
      if (pResultMsg)
      {
         return handleStunOutcome(*pResultMsg);
      }
   }
   return bRet;
}

UtlBoolean SipXMessageObserver::handleStunOutcome(const OsStunResultMsg& pResultMsg) 
{
   switch(pResultMsg.getMsgSubType())
   {
      case OsStunResultMsg::STUN_RESULT_SUCCESS:
         {
            const OsStunResultSuccessMsg* pResultSuccessMsg = dynamic_cast<const OsStunResultSuccessMsg*>(&pResultMsg);
            if (pResultSuccessMsg)
            {
               return handleStunSuccess(*pResultSuccessMsg);
            }
            break;
         }
      case OsStunResultMsg::STUN_RESULT_FAILURE:
         {
            const OsStunResultFailureMsg* pResultFailureMsg = dynamic_cast<const OsStunResultFailureMsg*>(&pResultMsg);
            if (pResultFailureMsg)
            {
               return handleStunFailure(*pResultFailureMsg);
            }
            break;
         }
      default:
         ;
   }

   return TRUE;
}

UtlBoolean SipXMessageObserver::handleStunSuccess(const OsStunResultSuccessMsg& pResultMsg)
{
   UtlString adapterName;
   UtlString localIp;
   UtlString mappedIp;
   int mappedPort;
   pResultMsg.getAdapterName(adapterName);
   pResultMsg.getLocalIp(localIp);
   pResultMsg.getMappedIp(mappedIp);
   mappedPort = pResultMsg.getMappedPort();

   // create SIPX_CONTACT_ADDRESS
   SipContact stunContact(SIPX_AUTOMATIC_CONTACT_ID, SIP_CONTACT_NAT_MAPPED, SIP_TRANSPORT_UDP,
      mappedIp, mappedPort, adapterName, localIp);

   // first, find the user-agent, and add the contact to
   // the user-agent's db, will assign proper contactId
   m_pInst->pSipUserAgent->addContact(stunContact);
   SIPX_CONTACT_ADDRESS sipxContact = getSipxContact(stunContact);

   sipxFireConfigEvent(m_pInst, CONFIG_STUN_SUCCESS, &sipxContact);

   return TRUE;
}

UtlBoolean SipXMessageObserver::handleStunFailure(const OsStunResultFailureMsg& pResultMsg)
{
   UtlString adapterName;
   UtlString localIp;
   pResultMsg.getAdapterName(adapterName);
   pResultMsg.getLocalIp(localIp);

   SIPX_STUN_FAILURE_INFO eventInfo;
   memset(&eventInfo, 0, sizeof(SIPX_STUN_FAILURE_INFO));
   eventInfo.nSize = sizeof(SIPX_STUN_FAILURE_INFO);
   eventInfo.szAdapterName = SAFE_STRDUP(adapterName.data());
   eventInfo.szInterfaceIp = SAFE_STRDUP(localIp.data());
   sipxFireConfigEvent(m_pInst, CONFIG_STUN_FAILURE, NULL, &eventInfo);
   free((void*)eventInfo.szAdapterName);
   free((void*)eventInfo.szInterfaceIp);

   return TRUE;
}
