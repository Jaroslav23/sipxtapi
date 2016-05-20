//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsTimer.h>
#include <net/SipMessage.h>
#include <cp/CpDefs.h>
#include <cp/state/SipConnectionStateContext.h>

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

SipConnectionStateContext::SipConnectionStateContext()
: m_mediaSessionState(SipConnectionStateContext::MEDIA_SESSION_NONE)
, m_previousMediaSessionState(SipConnectionStateContext::MEDIA_SESSION_NONE)
, m_localMediaConnectionState(SipConnectionStateContext::MEDIA_CONNECTION_NONE)
, m_remoteMediaConnectionState(SipConnectionStateContext::MEDIA_CONNECTION_NONE)
, m_implicitAllowedRemote("INVITE, ACK, CANCEL, BYE, OPTIONS, REGISTER")
, m_100relSetting(CP_100REL_PREFER_RELIABLE)
, m_allowedRemoteDiscovered(FALSE)
, m_supportedRemoteDiscovered(FALSE)
, m_contactId(AUTOMATIC_CONTACT_ID)
, m_transportType(SIP_TRANSPORT_AUTO)
, m_rtpTransport(RTP_TRANSPORT_UDP)
, m_pSecurity(NULL)
, m_pLastReceivedInvite(NULL)
, m_pLastSent2xxToInvite(NULL)
, m_pLastSentRefer(NULL)
, m_pLastReceivedRefer(NULL)
, m_491failureCounter(0)
, m_bUseLocalHoldSDP(FALSE)
, m_updateSetting(CP_SIP_UPDATE_ONLY_INBOUND)
, m_inviteExpiresSeconds(CP_MAXIMUM_RINGING_EXPIRE_SECONDS)
, m_connectedIdentityState(SipConnectionStateContext::IDENTITY_NOT_YET_ANNOUNCED)
, m_localEntityType(SipConnectionStateContext::ENTITY_NORMAL)
, m_referInSubscriptionActive(FALSE)
, m_bDropReferencedCall(FALSE)
, m_referOutSubscriptionActive(FALSE)
, m_inboundReferResponse(SipConnectionStateContext::REFER_NO_RESPONSE)
, m_bRTPRedirectActive(FALSE)
, m_bRTPRedirectMasterRole(FALSE)
, m_pRemoteAttachedSdpBody(NULL)
, m_pRemoteSdpBody(NULL)
, m_bRedirecting(FALSE)
, m_bAckReceived(FALSE)
, m_bCancelSent(FALSE)
, m_bByeSent(FALSE)
, m_iByeRetryCount(0)
, m_pByeRetryTimer(NULL)
, m_pCancelTimeoutTimer(NULL)
, m_pByeTimeoutTimer(NULL)
, m_pSessionRenegotiationTimer(NULL)
, m_iRenegotiationRetryCount(0)
, m_p2xxInviteRetransmitTimer(NULL)
, m_i2xxInviteRetransmitCount(0)
, m_pSessionTimeoutCheckTimer(NULL)
, m_pSessionRefreshTimer(NULL)
, m_pInviteExpiresTimer(NULL)
, m_p100relRetransmitTimer(NULL)
, m_i100relRetransmitCount(0)
, m_pDelayedAnswerTimer(NULL)
, m_iDelayedAnswerCount(0)
{
   m_sipClientTransactionMgr.setSipTransactionListener(&m_100RelTracker);
}

SipConnectionStateContext::~SipConnectionStateContext()
{
   delete m_pSecurity;
   m_pSecurity = NULL;
   delete m_pLastReceivedInvite;
   m_pLastReceivedInvite = NULL;
   delete m_pLastSent2xxToInvite;
   m_pLastSent2xxToInvite = NULL;
   delete m_pLastSentRefer;
   m_pLastSentRefer = NULL;
   delete m_pLastReceivedRefer;
   m_pLastReceivedRefer = NULL;
   delete m_pRemoteAttachedSdpBody;
   m_pRemoteAttachedSdpBody = NULL;
   delete m_pRemoteSdpBody;
   m_pRemoteSdpBody = NULL;
   delete m_pByeRetryTimer;
   m_pByeRetryTimer = NULL;
   delete m_pCancelTimeoutTimer;
   m_pCancelTimeoutTimer = NULL;
   delete m_pByeTimeoutTimer;
   m_pByeTimeoutTimer = NULL;
   delete m_pSessionRenegotiationTimer;
   m_pSessionRenegotiationTimer = NULL;
   delete m_p2xxInviteRetransmitTimer;
   m_p2xxInviteRetransmitTimer = NULL;
   delete m_pSessionTimeoutCheckTimer;
   m_pSessionTimeoutCheckTimer = NULL;
   delete m_pSessionRefreshTimer;
   m_pSessionRefreshTimer = NULL;
   delete m_pInviteExpiresTimer;
   m_pInviteExpiresTimer = NULL;
   delete m_p100relRetransmitTimer;
   m_p100relRetransmitTimer = NULL;
   delete m_pDelayedAnswerTimer;
   m_pDelayedAnswerTimer = NULL;
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

void SipConnectionStateContext::setRemoteAttachedSdpBody(SdpBody* val)
{
   if(m_pRemoteAttachedSdpBody)
   {
      delete m_pRemoteAttachedSdpBody;
      m_pRemoteAttachedSdpBody = NULL;
   }
   m_pRemoteAttachedSdpBody = val;
}

void SipConnectionStateContext::setRemoteSdpBody(SdpBody* val)
{
   if(m_pRemoteSdpBody)
   {
      delete m_pRemoteSdpBody;
      m_pRemoteSdpBody = NULL;
   }
   m_pRemoteSdpBody = val;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
