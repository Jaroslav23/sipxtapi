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
#include <tapi/SecurityEventMsg.h>
#include "tapi/SipXSecurityEventListener.h"
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

SipXSecurityEventListener::SipXSecurityEventListener(SIPX_INST pInst)
: OsSharedServerTask("SipXSecurityEventListener-%d")
, SipSecurityEventListener()
, m_pInst(pInst)
{

}

SipXSecurityEventListener::~SipXSecurityEventListener()
{
   release();
}

/* ============================ MANIPULATORS ============================== */

void SipXSecurityEventListener::OnEncrypt(const SipSecurityEvent& event)
{
   SecurityEventMsg msg(SECURITY_ENCRYPT, event);
   postMessage(msg);
}

void SipXSecurityEventListener::OnDecrypt(const SipSecurityEvent& event)
{
   SecurityEventMsg msg(SECURITY_DECRYPT, event);
   postMessage(msg);
}

void SipXSecurityEventListener::OnTLS(const SipSecurityEvent& event)
{
   SecurityEventMsg msg(SECURITY_TLS, event);
   postMessage(msg);
}

UtlBoolean SipXSecurityEventListener::handleMessage(OsMsg& rRawMsg)
{
   UtlBoolean bResult = FALSE;

   switch (rRawMsg.getMsgType())
   {
   case SECURITY_MSG:
      {
         SecurityEventMsg* pMsg = dynamic_cast<SecurityEventMsg*>(&rRawMsg);
         if (pMsg)
         {
            // cast succeeded
            const SipSecurityEvent& payload = pMsg->getEventPayloadRef();
            handleSecurityEvent(payload.m_sSRTPkey, payload.m_pCertificate,
               payload.m_nCertificateSize, pMsg->getEvent(), (SIPX_SECURITY_CAUSE)payload.m_cause,
               payload.m_sSubjAltName, payload.m_SessionCallId, payload.m_sRemoteAddress);
         }
      }
      bResult = TRUE;
      break;
   default:
      break;
   }
   return bResult;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void SipXSecurityEventListener::handleSecurityEvent(const UtlString& sSRTPkey,
                                                    void* pCertificate,
                                                    size_t nCertificateSize,
                                                    SIPX_SECURITY_EVENT event,
                                                    SIPX_SECURITY_CAUSE cause,
                                                    const UtlString& sSubjAltName,
                                                    const UtlString& sSessionCallId,
                                                    const UtlString& sRemoteAddress)
{
   SIPX_SECURITY_INFO info;
   memset((void*)&info, 0, sizeof(SIPX_SECURITY_INFO));

   info.nSize = sizeof(SIPX_SECURITY_INFO);
   info.szSRTPkey = sSRTPkey.data();
   info.pCertificate = pCertificate;
   info.nCertificateSize = nCertificateSize;
   info.cause = cause;
   info.event = event;
   info.szSubjAltName = sSubjAltName.data();
   info.callId = sSessionCallId.data();
   info.hCall = sipxCallLookupHandleBySessionCallId(sSessionCallId, m_pInst);
   info.remoteAddress = sRemoteAddress.data();

   SipXEventDispatcher::dispatchEvent(m_pInst, EVENT_CATEGORY_SECURITY, &info);
}

/* ============================ FUNCTIONS ================================= */

