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
#include <os/OsLock.h>
#include <os/OsPtrLock.h>
#include <os/OsIntPtrMsg.h>
#include <sdp/SdpCodecFactory.h>
#include <net/SipMessageEvent.h>
#include <net/SipMessage.h>
#include <net/SipDialog.h>
#include <net/SipLineProvider.h>
#include <cp/XCpCall.h>
#include <cp/XSipConnection.h>
#include <cp/msg/AcConnectMsg.h>
#include <cp/msg/AcStartRtpRedirectMsg.h>
#include <cp/msg/AcStopRtpRedirectMsg.h>
#include <cp/msg/AcDropConnectionMsg.h>
#include <cp/msg/AcDestroyConnectionMsg.h>
#include <cp/msg/CpTimerMsg.h>
#include <cp/msg/CmDestroyAbstractCallMsg.h>

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

XCpCall::XCpCall(const UtlString& sId,
                 SipUserAgent& rSipUserAgent,
                 XCpCallControl& rCallControl,
                 SipLineProvider* pSipLineProvider,
                 CpMediaInterfaceFactory& rMediaInterfaceFactory,
                 const SdpCodecList& rDefaultSdpCodecList,
                 OsMsgQ& rCallManagerQueue,
                 const CpNatTraversalConfig& rNatTraversalConfig,
                 const UtlString& sBindIpAddress,
                 int sessionTimerExpiration,
                 CP_SESSION_TIMER_REFRESH sessionTimerRefresh,
                 CP_SIP_UPDATE_CONFIG updateSetting,
                 CP_100REL_CONFIG c100relSetting,
                 CP_SDP_OFFERING_MODE sdpOfferingMode,
                 int inviteExpiresSeconds,
                 XCpCallConnectionListener* pCallConnectionListener,
                 CpCallStateEventListener* pCallEventListener,
                 SipInfoStatusEventListener* pInfoStatusEventListener,
                 SipInfoEventListener* pInfoEventListener,
                 SipSecurityEventListener* pSecurityEventListener,
                 CpMediaEventListener* pMediaEventListener,
                 CpRtpRedirectEventListener* pRtpRedirectEventListener)
: XCpAbstractCall(sId, rSipUserAgent, rCallControl, pSipLineProvider, rMediaInterfaceFactory, rDefaultSdpCodecList, rCallManagerQueue, rNatTraversalConfig,
                  sBindIpAddress, sessionTimerExpiration, sessionTimerRefresh, updateSetting, c100relSetting, sdpOfferingMode, inviteExpiresSeconds,
                  pCallConnectionListener, pCallEventListener, pInfoStatusEventListener,
                  pInfoEventListener, pSecurityEventListener, pMediaEventListener, pRtpRedirectEventListener)
, m_pSipConnection(NULL)
{

}

XCpCall::~XCpCall()
{
   destroySipConnection(); // destroy connection if it still exists
   waitUntilShutDown();
}

/* ============================ MANIPULATORS ============================== */

OsStatus XCpCall::connect(const UtlString& sipCallId,
                          SipDialog& sipDialog,
                          const UtlString& toAddress,
                          const UtlString& fromAddress,
                          const UtlString& locationHeader,
                          CP_CONTACT_ID contactId,
                          SIP_TRANSPORT_TYPE transport,
                          CP_FOCUS_CONFIG focusConfig,
                          const UtlString& replacesField,
                          CP_CALLSTATE_CAUSE callstateCause,
                          const SipDialog* pCallbackSipDialog)
{
   if (sipCallId.isNull() || toAddress.isNull() || fromAddress.isNull())
   {
      return OS_FAILED;
   }
   m_focusConfig = focusConfig;

   UtlString localTag(m_sipTagGenerator.getNewTag());
   sipDialog = SipDialog(sipCallId, localTag, NULL);

   AcConnectMsg connectMsg(sipCallId, toAddress, localTag, fromAddress, locationHeader, contactId,
      transport, replacesField, callstateCause, pCallbackSipDialog);
   return postMessage(connectMsg);
}

OsStatus XCpCall::startCallRedirectRtp(const UtlString& slaveAbstractCallId, const SipDialog& slaveSipDialog)
{
   AcStartRtpRedirectMsg startRtpRedirectMsg(slaveAbstractCallId, slaveSipDialog);
   return postMessage(startRtpRedirectMsg);
}

OsStatus XCpCall::stopCallRedirectRtp()
{
   AcStopRtpRedirectMsg stopRtpRedirectMsg;
   return postMessage(stopRtpRedirectMsg);
}

OsStatus XCpCall::dropConnection(const SipDialog& sipDialog)
{
   AcDropConnectionMsg dropConnectionMsg(sipDialog);
   return postMessage(dropConnectionMsg);
}

OsStatus XCpCall::dropConnection()
{
   AcDropConnectionMsg dropConnectionMsg(NULL);
   return postMessage(dropConnectionMsg);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

SipDialog::DialogMatchEnum XCpCall::hasSipDialog(const SipDialog& sipDialog) const
{
   OsLock lock(m_memberMutex);

   if (m_pSipConnection)
   {
      return m_pSipConnection->compareSipDialog(sipDialog);
   }

   return SipDialog::DIALOG_MISMATCH;
}

int XCpCall::getCallCount() const
{
   return m_pSipConnection != NULL ? 1 : 0;
}

OsStatus XCpCall::getCallSipCallId(UtlString& sSipCallId) const
{
   OsStatus result = OS_INVALID;

   OsPtrLock<XSipConnection> ptrLock; // auto pointer lock
   UtlBoolean resFind = getConnection(ptrLock);
   if (resFind)
   {
      ptrLock->getSipCallId(sSipCallId);
      result = OS_SUCCESS;
   }

   return result;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean XCpCall::findConnection(const SipDialog& sipDialog, OsPtrLock<XSipConnection>& ptrLock) const
{
   OsLock lock(m_memberMutex);

   if (m_pSipConnection && m_pSipConnection->compareSipDialog(sipDialog) != SipDialog::DIALOG_MISMATCH)
   {
      // dialog matches
      ptrLock = m_pSipConnection;
      return TRUE;
   }

   return FALSE;
}

UtlBoolean XCpCall::getConnection(OsPtrLock<XSipConnection>& ptrLock) const
{
   OsLock lock(m_memberMutex);

   ptrLock = m_pSipConnection;
   return m_pSipConnection != NULL;
}

UtlBoolean XCpCall::handleCommandMessage(const AcCommandMsg& rRawMsg)
{
   switch ((AcCommandMsg::SubTypeEnum)rRawMsg.getMsgSubType())
   {
   case AcCommandMsg::AC_CONNECT:
      handleConnect((const AcConnectMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_START_RTP_REDIRECT:
      handleStartRtpRedirect((const AcStartRtpRedirectMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_STOP_RTP_REDIRECT:
      handleStopRtpRedirect((const AcStopRtpRedirectMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_DROP_CONNECTION:
      handleDropConnection((const AcDropConnectionMsg&)rRawMsg);
      return TRUE;
   default:
      break;
   }

   // we couldn't handle it, give chance to parent
   return XCpAbstractCall::handleCommandMessage(rRawMsg);
}

UtlBoolean XCpCall::handleNotificationMessage(const AcNotificationMsg& rRawMsg)
{
   // we couldn't handle it, give chance to parent
   return XCpAbstractCall::handleNotificationMessage(rRawMsg);
}

UtlBoolean XCpCall::handleTimerMessage(const CpTimerMsg& rRawMsg)
{
   // we couldn't handle it, give chance to parent
   return XCpAbstractCall::handleTimerMessage(rRawMsg);
}

OsStatus XCpCall::handleSipMessageEvent(const SipMessageEvent& rSipMsgEvent)
{
   OsStatus res = XCpAbstractCall::handleSipMessageEvent(rSipMsgEvent);

   if (res == OS_NOT_FOUND)
   {
      const SipMessage* pSipMessage = rSipMsgEvent.getMessage();
      if (pSipMessage && pSipMessage->isInviteRequest())
      {
         // we have inbound INVITE request, call doesn't exist yet
         // discover real line url
         UtlString sFullLineUrl(getRealLineIdentity(*pSipMessage));
         SipDialog sipDialog(pSipMessage);
         // sip connection doesn't yet exist, and we received new INVITE message
         createSipConnection(sipDialog, sFullLineUrl); // create sip connection
         OsPtrLock<XSipConnection> ptrLock;
         UtlBoolean resFound = getConnection(ptrLock);
         if (resFound)
         {
            if (ptrLock->handleSipMessageEvent(rSipMsgEvent)) // let it handle INVITE
            {
               return OS_SUCCESS;
            }
            else
            {
               return OS_FAILED;
            }
         }
      }
      else
      {
         return OS_NOT_FOUND;
      }
   }

   return res;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

OsStatus XCpCall::handleConnect(const AcConnectMsg& rMsg)
{
   OsStatus result = OS_FAILED;
   // thread safe check, as connection is only created/destroyed in this thread
   if (!m_pSipConnection)
   {
      SipDialog sipDialog(rMsg.getSipCallId(), rMsg.getLocalTag(), NULL, TRUE);
      sipDialog.setLocalField(Url(rMsg.getFromAddress()));
      createSipConnection(sipDialog, rMsg.getFromAddress()); // use from address as real line url (supports virtual lines)
   }

   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFound = getConnection(ptrLock);
   if (resFound)
   {
      result = ptrLock->connect(rMsg.getSipCallId(), rMsg.getLocalTag(), rMsg.getToAddress(), rMsg.getFromAddress(),
         rMsg.getLocationHeader(), rMsg.getContactId(), rMsg.getTransport(),
         rMsg.getReplacesField(), rMsg.getCallstateCause(), rMsg.getCallbackSipDialog());
   }

   return result;
}

OsStatus XCpCall::handleStartRtpRedirect(const AcStartRtpRedirectMsg& rMsg)
{
   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFound = getConnection(ptrLock);
   if (resFound)
   {
      UtlString slaveAbstractCallId;
      SipDialog slaveSipDialog;
      rMsg.getSlaveAbstractCallId(slaveAbstractCallId);
      rMsg.getSlaveSipDialog(slaveSipDialog);
      return ptrLock->startRtpRedirect(slaveAbstractCallId, slaveSipDialog);
   }

   return OS_NOT_FOUND;
}

OsStatus XCpCall::handleStopRtpRedirect(const AcStopRtpRedirectMsg& rMsg)
{
   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFound = getConnection(ptrLock);
   if (resFound)
   {
      return ptrLock->stopRtpRedirect();
   }

   return OS_NOT_FOUND;
}

OsStatus XCpCall::handleDropConnection(const AcDropConnectionMsg& rMsg)
{
   if (m_pSipConnection)
   {
      SipDialog sipDialog;
      rMsg.getSipDialog(sipDialog);
      UtlBoolean resFound = FALSE;
      // find connection by sip dialog if call-id is not null
      OsPtrLock<XSipConnection> ptrLock;
      if (sipDialog.getCallId().isNull())
      {
         resFound = getConnection(ptrLock);
      }
      else
      {
         resFound = findConnection(sipDialog, ptrLock);
      }
      if (resFound)
      {
         return ptrLock->dropConnection();
      }
   }
   else
   {
      // we don't have any connection
      requestCallDestruction();
      return OS_SUCCESS;
   }

   return OS_NOT_FOUND;
}

OsStatus XCpCall::handleDestroyConnection(const AcDestroyConnectionMsg& rMsg)
{
   // get rid of sip connection
   SipDialog sipDialog;
   rMsg.getSipDialog(sipDialog);
   destroySipConnection(sipDialog);

   requestCallDestruction();
   return OS_SUCCESS;
}

void XCpCall::createSipConnection(const SipDialog& sipDialog, const UtlString& sFullLineUrl)
{
   UtlBoolean bAdded = FALSE;
   {
      OsLock lock(m_memberMutex);
      if (!m_pSipConnection)
      {
         m_pSipConnection = new XSipConnection(m_sId, sipDialog, m_rSipUserAgent, m_rCallControl, sFullLineUrl, m_sBindIpAddress,
            m_sessionTimerExpiration, m_sessionTimerRefresh,
            m_updateSetting, m_100relSetting, m_sdpOfferingMode, m_inviteExpiresSeconds, this, this, m_natTraversalConfig,
            m_pCallEventListener, m_pInfoStatusEventListener, m_pInfoEventListener, m_pSecurityEventListener, m_pMediaEventListener,
            m_pRtpRedirectEventListener);
         bAdded = TRUE;
      }
   }

   if (bAdded)
   {
      onConnectionAddded(sipDialog.getCallId());
   }
}

void XCpCall::destroySipConnection()
{
   SipDialog sipDialog;
   // get sip dialog of our single connection
   {
      OsLock lock(m_memberMutex);
      if (m_pSipConnection)
      {
         m_pSipConnection->getSipDialog(sipDialog);
      }
   }

   if (!sipDialog.isNull())
   {
      destroySipConnection(sipDialog);
   }
}

void XCpCall::destroySipConnection(const SipDialog& sSipDialog)
{
   UtlString sSipCallId;
   {
      // check that we really have connection with given sip dialog
      OsLock lock(m_memberMutex);
      if (m_pSipConnection && m_pSipConnection->compareSipDialog(sSipDialog) != SipDialog::DIALOG_MISMATCH)
      {
         // dialog matches
         sSipDialog.getCallId(sSipCallId);
         m_pSipConnection->acquireExclusive();
         delete m_pSipConnection;
         m_pSipConnection = NULL;
      }
   }

   if (!sSipCallId.isNull())
   {
      // we destroyed some connection, notify call stack
      onConnectionRemoved(sSipCallId);
   }
}

void XCpCall::requestCallDestruction()
{
   releaseMediaInterface(); // release audio resources

   CmDestroyAbstractCallMsg msg(m_sId);
   getGlobalQueue().send(msg); // instruct call manager to destroy this call
}

void XCpCall::fireSipXMediaConnectionEvent(CP_MEDIA_EVENT event,
                                           CP_MEDIA_CAUSE cause,
                                           CP_MEDIA_TYPE type,
                                           int mediaConnectionId,
                                           intptr_t pEventData1,
                                           intptr_t pEventData2)
{
   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFound = getConnection(ptrLock);
   if (resFound)
   {
      if (ptrLock->getMediaEventConnectionId() == mediaConnectionId)
      {
         ptrLock->handleSipXMediaEvent(event, cause, type, pEventData1, pEventData2);
      }
   }
}

void XCpCall::fireSipXMediaInterfaceEvent(CP_MEDIA_EVENT event, 
                                          CP_MEDIA_CAUSE cause,
                                          CP_MEDIA_TYPE type,
                                          intptr_t pEventData1,
                                          intptr_t pEventData2)
{
   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFound = getConnection(ptrLock);
   if (resFound)
   {
      ptrLock->handleSipXMediaEvent(event, cause, type, pEventData1, pEventData2);
   }
}

void XCpCall::onFocusGained()
{
   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFound = getConnection(ptrLock);
   if (resFound)
   {
      ptrLock->onFocusGained();
   }
}

void XCpCall::onFocusLost()
{
   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFound = getConnection(ptrLock);
   if (resFound)
   {
      ptrLock->onFocusLost();
   }
}

void XCpCall::onStarted()
{
   // this gets called once thread is started
}

OsStatus XCpCall::extractConnection(XSipConnection **pSipConnection)
{
   if (isShutDown())
   {
      OsLock lock(m_memberMutex);
      if (m_pSipConnection && pSipConnection)
      {
         UtlString sSipCallId;
         m_pSipConnection->getSipCallId(sSipCallId);
         onConnectionRemoved(sSipCallId);

         *pSipConnection = m_pSipConnection;
         m_pSipConnection = NULL;
         return OS_SUCCESS;
      }
   }

   return OS_FAILED;
}

OsStatus XCpCall::setConnection(XSipConnection *pSipConnection)
{
   if (isShutDown())
   {
      OsLock lock(m_memberMutex);
      if (!m_pSipConnection)
      {
         m_pSipConnection = pSipConnection;
         m_pSipConnection->setAbstractCallId(m_sId);
         m_pSipConnection->setMediaInterfaceProvider(this);
         m_pSipConnection->setMessageQueueProvider(this);

         UtlString sSipCallId;
         m_pSipConnection->getSipCallId(sSipCallId);
         onConnectionAddded(sSipCallId);
         return OS_SUCCESS;
      }
   }

   return OS_FAILED;
}

/* ============================ FUNCTIONS ================================= */
