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
#include <utl/UtlSListIterator.h>
#include <net/SipDialog.h>
#include <net/SipLine.h>
#include <net/SipMessageEvent.h>
#include <net/SipUserAgent.h>
#include <sdp/SdpCodecFactory.h>
#include <cp/XCpConference.h>
#include <cp/XCpCall.h>
#include <cp/XSipConnection.h>
#include <cp/msg/AcConnectMsg.h>
#include <cp/msg/AcDropConnectionMsg.h>
#include <cp/msg/AcDropAllConnectionsMsg.h>
#include <cp/msg/AcDestroyConnectionMsg.h>
#include <cp/msg/AcHoldAllConnectionsMsg.h>
#include <cp/msg/AcUnholdAllConnectionsMsg.h>
#include <cp/msg/AcRenegotiateCodecsAllMsg.h>
#include <cp/msg/AcConferenceJoinMsg.h>
#include <cp/msg/AcConferenceSplitMsg.h>
#include <cp/msg/CmDestroyAbstractCallMsg.h>
#include <cp/msg/CpTimerMsg.h>
#include <cp/CpConferenceEventListener.h>
#include <cp/XCpCallLookup.h>

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

XCpConference::XCpConference(const UtlString& sId,
                             const UtlString& sConferenceUri,
                             SipUserAgent& rSipUserAgent,
                             XCpCallControl& rCallControl,
                             XCpCallLookup& rCallLookup,
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
                             CpRtpRedirectEventListener* pRtpRedirectEventListener,
							 CpConferenceEventListener* pConferenceEventListener)
: XCpAbstractCall(sId, rSipUserAgent, rCallControl, pSipLineProvider, rMediaInterfaceFactory, rDefaultSdpCodecList, rCallManagerQueue, rNatTraversalConfig,
                  sBindIpAddress, sessionTimerExpiration, sessionTimerRefresh, updateSetting, c100relSetting, sdpOfferingMode, inviteExpiresSeconds,
                  pCallConnectionListener, pCallEventListener, pInfoStatusEventListener,
                  pInfoEventListener, pSecurityEventListener, pMediaEventListener, pRtpRedirectEventListener)
, m_conferenceUri(SipLine::getLineUri(sConferenceUri))
, m_pConferenceEventListener(pConferenceEventListener)
, m_rCallLookup(rCallLookup)
, m_bDestroyConference(FALSE)
{
}

XCpConference::~XCpConference()
{
   destroyAllSipConnections();
   waitUntilShutDown();
}

/* ============================ MANIPULATORS ============================== */

OsStatus XCpConference::connect(const UtlString& sipCallId,
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

OsStatus XCpConference::dropConnection(const SipDialog& sipDialog)
{
   AcDropConnectionMsg dropConnectionMsg(sipDialog);
   return postMessage(dropConnectionMsg);
}

OsStatus XCpConference::dropAllConnections(UtlBoolean bDestroyConference)
{
   AcDropAllConnectionsMsg dropAllConnectionsMsg(bDestroyConference);
   return postMessage(dropAllConnectionsMsg);
}

OsStatus XCpConference::holdAllConnections()
{
   AcHoldAllConnectionsMsg holdAllConnectionsMsg;
   return postMessage(holdAllConnectionsMsg);
}

OsStatus XCpConference::unholdAllConnections()
{
   AcUnholdAllConnectionsMsg unholdAllConnectionsMsg;
   return postMessage(unholdAllConnectionsMsg);
}

OsStatus XCpConference::renegotiateCodecsAllConnections(const UtlString& sAudioCodecs,
                                                        const UtlString& sVideoCodecs)
{
   AcRenegotiateCodecsAllMsg renegotiateCodecsMsg(sAudioCodecs, sVideoCodecs);
   return postMessage(renegotiateCodecsMsg);
}

OsStatus XCpConference::join(const SipDialog& sipDialog)
{
   AcConferenceJoinMsg conferenceJoinMsg(sipDialog);
   return postMessage(conferenceJoinMsg);
}

OsStatus XCpConference::split(const SipDialog& sipDialog,
                              const UtlString& sNewCallId)
{
   AcConferenceSplitMsg conferenceSplitMsg(sipDialog, sNewCallId);
   return postMessage(conferenceSplitMsg);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

SipDialog::DialogMatchEnum XCpConference::hasSipDialog(const SipDialog& sipDialog) const
{
   SipDialog::DialogMatchEnum result = SipDialog::DIALOG_MISMATCH;
   OsLock lock(m_memberMutex);

   UtlSListIterator itor(m_sipConnections);
   XSipConnection* pSipConnection = NULL;

   while (itor())
   {
      pSipConnection = dynamic_cast<XSipConnection*>(itor.item());
      if (pSipConnection)
      {
         SipDialog::DialogMatchEnum tmpResult = pSipConnection->compareSipDialog(sipDialog);
         if (tmpResult == SipDialog::DIALOG_ESTABLISHED_MATCH ||
             tmpResult == SipDialog::DIALOG_INITIAL_INITIAL_MATCH)
         {
            // return immediately if found perfect match for established dialog
            // initial match is also perfect if supplied dialog is initial
            return tmpResult;
         }
         else if (tmpResult != SipDialog::DIALOG_MISMATCH)
         {
            // only override result if we found some match, as we could have some connection at the end of list that would not match
            result = tmpResult;
         }
      }
   }

   return result;
}

int XCpConference::getCallCount() const
{
   // thread safe
   return (int)m_sipConnections.entries();
}

OsStatus XCpConference::getConferenceSipCallIds(UtlSList& sipCallIdList) const
{
   sipCallIdList.destroyAll();

   OsLock lock(m_memberMutex);

   UtlSListIterator itor(m_sipConnections);
   XSipConnection* pSipConnection = NULL;
   UtlString sipCallId;

   while (itor())
   {
      pSipConnection = dynamic_cast<XSipConnection*>(itor.item());
      if (pSipConnection)
      {
         pSipConnection->getSipCallId(sipCallId);
         if (!sipCallId.isNull())
         {
            sipCallIdList.append(sipCallId.clone());
         }
      }
   }

   return OS_SUCCESS;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean XCpConference::findConnection(const SipDialog& sipDialog, OsPtrLock<XSipConnection>& ptrLock) const
{
   OsLock lock(m_memberMutex);
   XSipConnection* pSipConnection = findConnection(sipDialog);

   if (pSipConnection)
   {
      ptrLock = pSipConnection;
      return TRUE;
   }

   return FALSE;
}

UtlBoolean XCpConference::handleCommandMessage(const AcCommandMsg& rRawMsg)
{
   switch ((AcCommandMsg::SubTypeEnum)rRawMsg.getMsgSubType())
   {
   case AcCommandMsg::AC_CONNECT:
      handleConnect((const AcConnectMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_DROP_CONNECTION:
      handleDropConnection((const AcDropConnectionMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_DROP_ALL_CONNECTIONS:
      handleDropAllConnections((const AcDropAllConnectionsMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_HOLD_ALL_CONNECTIONS:
      handleHoldAllConnections((const AcHoldAllConnectionsMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_UNHOLD_ALL_CONNECTIONS:
      handleUnholdAllConnections((const AcUnholdAllConnectionsMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_RENEGOTIATE_CODECS_ALL:
      handleRenegotiateCodecsAll((const AcRenegotiateCodecsAllMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_CONFERENCE_JOIN:
      handleJoin((const AcConferenceJoinMsg&)rRawMsg);
      return TRUE;
   case AcCommandMsg::AC_CONFERENCE_SPLIT:
      handleSplit((const AcConferenceSplitMsg&)rRawMsg);
      return TRUE;
   default:
      break;
   }

   // we couldn't handle it, give chance to parent
   return XCpAbstractCall::handleCommandMessage(rRawMsg);
}

UtlBoolean XCpConference::handleNotificationMessage(const AcNotificationMsg& rRawMsg)
{
   // we couldn't handle it, give chance to parent
   return XCpAbstractCall::handleNotificationMessage(rRawMsg);
}

UtlBoolean XCpConference::handleTimerMessage(const CpTimerMsg& rRawMsg)
{
   // we couldn't handle it, give chance to parent
   return XCpAbstractCall::handleTimerMessage(rRawMsg);
}

OsStatus XCpConference::handleSipMessageEvent(const SipMessageEvent& rSipMsgEvent)
{
   OsStatus res = XCpAbstractCall::handleSipMessageEvent(rSipMsgEvent);

   if (res == OS_NOT_FOUND)
   {
      const SipMessage* pSipMessage = rSipMsgEvent.getMessage();
      if (pSipMessage && pSipMessage->isInviteRequest())
      {
         if (getCallCount() < CONF_MAX_CONNECTIONS)
         {
            // we have inbound INVITE request, call doesn't exist yet
            // discover real line url
            UtlString sFullLineUrl(getRealLineIdentity(*pSipMessage));
            SipDialog sipDialog(pSipMessage);
            // sip connection doesn't yet exist, and we received new INVITE message
            createSipConnection(sipDialog, sFullLineUrl, FALSE); // create sip connection
            OsPtrLock<XSipConnection> ptrLock;
            UtlBoolean resFound = findConnection(sipDialog, ptrLock);
            if (resFound)
            {
               if (ptrLock->handleSipMessageEvent(rSipMsgEvent)) // let it handle INVITE
               {
                  fireConferenceEvent(CP_CONFERENCE_CALL_ADDED, CP_CONFERENCE_CAUSE_NORMAL, &sipDialog);
                  return OS_SUCCESS;
               }
               else
               {
                  destroySipConnection(sipDialog, FALSE);
                  return OS_FAILED;
               }
            }
         }
         else // too many conference connections
         {
            OsSysLog::add(FAC_CP, PRI_WARNING,
               "XCpConference::handleSipMessageEvent - unable to receive new call, connection count reached its limit of %d",
               CONF_MAX_CONNECTIONS);
            // send 486 Busy here
            SipMessage busyHereResponse;
            busyHereResponse.setInviteBusyData(pSipMessage);
            m_rSipUserAgent.send(busyHereResponse);
            return OS_SUCCESS;
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

OsStatus XCpConference::handleConnect(const AcConnectMsg& rMsg)
{
   OsStatus result = OS_FAILED;
   SipDialog sipDialog(rMsg.getSipCallId(), rMsg.getLocalTag(), NULL, TRUE);
   sipDialog.setLocalField(Url(rMsg.getFromAddress()));

   if (getCallCount() < CONF_MAX_CONNECTIONS)
   {
      // use from address as real line url (supports virtual lines)
      createSipConnection(sipDialog, rMsg.getFromAddress());

      {
         OsPtrLock<XSipConnection> ptrLock;
         UtlBoolean resFound = findConnection(sipDialog, ptrLock);
         if (resFound)
         {
            result = ptrLock->connect(rMsg.getSipCallId(), rMsg.getLocalTag(), rMsg.getToAddress(), rMsg.getFromAddress(),
               rMsg.getLocationHeader(), rMsg.getContactId(), rMsg.getTransport(),
               rMsg.getReplacesField(), rMsg.getCallstateCause(), rMsg.getCallbackSipDialog());
         }
         else
         {
            result = OS_NOT_FOUND;
         }
      }
   }
   else
   {
      result = OS_LIMIT_REACHED;
   }

   return result;
}

OsStatus XCpConference::handleDropConnection(const AcDropConnectionMsg& rMsg)
{
   SipDialog sipDialog;
   rMsg.getSipDialog(sipDialog);

   OsPtrLock<XSipConnection> ptrLock;
   UtlBoolean resFind = findConnection(sipDialog, ptrLock);
   if (resFind)
   {
      return ptrLock->dropConnection();
   }

   return OS_NOT_FOUND;
}

OsStatus XCpConference::handleDropAllConnections(const AcDropAllConnectionsMsg& rMsg)
{
   m_bDestroyConference = rMsg.getDestroyAbstractCall();

   if (!m_sipConnections.isEmpty())
   {
      OsLock lock(m_memberMutex);
      UtlSListIterator itor(m_sipConnections);

      while (itor())
      {
         XSipConnection *pConnection = dynamic_cast<XSipConnection*>(itor.item());
         if (pConnection)
         {
            pConnection->dropConnection();
         }
      }
   }
   else
   {
      // there are no connections to drop
      if (m_bDestroyConference)
      {
         requestConferenceDestruction();
      }
   }

   return OS_SUCCESS;
}

// called as a result of sip connection requesting its deletion
OsStatus XCpConference::handleDestroyConnection(const AcDestroyConnectionMsg& rMsg)
{
   SipDialog sipDialog;
   rMsg.getSipDialog(sipDialog);
   destroySipConnection(sipDialog);

   if (m_bDestroyConference && m_sipConnections.isEmpty())
   {
      requestConferenceDestruction();
   }

   return OS_SUCCESS;
}

OsStatus XCpConference::handleHoldAllConnections(const AcHoldAllConnectionsMsg& rMsg)
{
   // execute hold on all connections
   OsLock lock(m_memberMutex);
   UtlSListIterator itor(m_sipConnections);

   while (itor())
   {
      XSipConnection *pConnection = dynamic_cast<XSipConnection*>(itor.item());
      if (pConnection)
      {
         pConnection->holdConnection();
      }
   }

   return OS_SUCCESS;
}

OsStatus XCpConference::handleUnholdAllConnections(const AcUnholdAllConnectionsMsg& rMsg)
{
   // execute unhold on all connections
   OsLock lock(m_memberMutex);
   UtlSListIterator itor(m_sipConnections);

   while (itor())
   {
      XSipConnection *pConnection = dynamic_cast<XSipConnection*>(itor.item());
      if (pConnection)
      {
         pConnection->unholdConnection();
      }
   }

   return OS_SUCCESS;
}

OsStatus XCpConference::handleRenegotiateCodecsAll(const AcRenegotiateCodecsAllMsg& rMsg)
{
   UtlString audioCodecs = SdpCodecFactory::getFixedAudioCodecs(rMsg.getAudioCodecs()); // add "telephone-event" if its missing

   if (doLimitCodecPreferences(audioCodecs, rMsg.getVideoCodecs()) == OS_SUCCESS)
   {
      // codec limiting succeeded, renegotiate codecs on all connections
      OsLock lock(m_memberMutex);
      UtlSListIterator itor(m_sipConnections);

      while (itor())
      {
         XSipConnection *pConnection = dynamic_cast<XSipConnection*>(itor.item());
         if (pConnection)
         {
            pConnection->renegotiateCodecsConnection();
         }
      }

      return OS_SUCCESS;
   }

   return OS_FAILED;
}

OsStatus XCpConference::handleJoin(const AcConferenceJoinMsg& rMsg)
{
   OsStatus result = OS_FAILED;
   CP_CONFERENCE_CAUSE eventCause = CP_CONFERENCE_CAUSE_NORMAL;
   SipDialog sipDialog;
   XSipConnection *pSipConnection = NULL;
   rMsg.getSipDialog(sipDialog);

   if (getCallCount() < CONF_MAX_CONNECTIONS)
   {
      OsPtrLock<XCpCall> ptrLock;
      UtlBoolean bFound = m_rCallLookup.findCall(sipDialog, ptrLock);
      if (bFound)
      {
         ptrLock->waitUntilShutDown(); // stops thread, will not process any more messages
         if (ptrLock->extractConnection(&pSipConnection) == OS_SUCCESS)
         {
            m_sipConnections.append(pSipConnection);
            onConnectionAddded(sipDialog.getCallId());
            // now call thread is suspended and empty, we have the connection
            if (pSipConnection->terminateMediaConnection() == OS_SUCCESS)
            {
               // update references to queue, media interface, callId
               pSipConnection->setAbstractCallId(m_sId);
               pSipConnection->setMediaInterfaceProvider(this);
               pSipConnection->setMessageQueueProvider(this);
               // ask call manager to destroy the old call shell
               ptrLock->start();
               ptrLock->dropConnection();
               // request codec renegotiation
               pSipConnection->renegotiateCodecsConnection();
               result = OS_SUCCESS;
            }
            else
            {
               m_sipConnections.remove(pSipConnection);
               onConnectionRemoved(sipDialog.getCallId());
               eventCause = CP_CONFERENCE_CAUSE_INVALID_STATE;
               // return the connection to call
               if (ptrLock->setConnection(pSipConnection) != OS_SUCCESS)
               {
                  // this would be a major problem. CP_CALLSTATE_DESTROYED will be fired but call will not be disconnected
                  OsSysLog::add(FAC_CP, PRI_ERR,
                     "handleJoin failed and was unable to restore call connection");
                  delete pSipConnection;
                  pSipConnection = NULL;
                  ptrLock->start();
                  ptrLock->dropConnection();
               }
               else
               {
                  ptrLock->start();
               }
            }
         }
         else // extractConnection failed
         {
            ptrLock->start();
            eventCause = CP_CONFERENCE_CAUSE_UNEXPECTED_ERROR;
         }
      }
      else // findCall failed
      {
         eventCause = CP_CONFERENCE_CAUSE_NOT_FOUND;
      }
   }
   else // too many sip connections in conference
   {
      result = OS_LIMIT_REACHED;
      eventCause = CP_CONFERENCE_CAUSE_LIMIT_REACHED;
   }

   if (result == OS_SUCCESS)
   {
      fireConferenceEvent(CP_CONFERENCE_CALL_ADDED, eventCause, &sipDialog);
   }
   else
   {
      fireConferenceEvent(CP_CONFERENCE_CALL_ADD_FAILURE, eventCause, &sipDialog);
   }

   return result;
}

OsStatus XCpConference::handleSplit(const AcConferenceSplitMsg& rMsg)
{
   OsStatus result = OS_FAILED;
   CP_CONFERENCE_CAUSE eventCause = CP_CONFERENCE_CAUSE_NORMAL;
   SipDialog sipDialog;
   UtlString newCallId;
   rMsg.getNewCallId(newCallId);
   rMsg.getSipDialog(sipDialog);

   {
      OsPtrLock<XCpCall> ptrLock;
      UtlBoolean bFound = m_rCallLookup.findCall(newCallId, ptrLock);
      if (bFound)
      {
         OsLock lock(m_memberMutex);
         XSipConnection *pSipConnection = findConnection(sipDialog);
         if (pSipConnection)
         {
            OsLock connectionLock(*pSipConnection);
            ptrLock->waitUntilShutDown(); // stops thread, will not process any more messages
            if (pSipConnection->terminateMediaConnection() == OS_SUCCESS)
            {
               if (ptrLock->setConnection(pSipConnection) == OS_SUCCESS)
               {
                  onConnectionRemoved(sipDialog.getCallId());
                  m_sipConnections.remove(pSipConnection);
                  // request codec renegotiation
                  pSipConnection->renegotiateCodecsConnection();
                  ptrLock->start();
                  result = OS_SUCCESS;
               }
               else // setConnection failed
               {
                  ptrLock->start();
                  ptrLock->dropConnection(); // destroy new call shell, cannot complete split
                  eventCause = CP_CONFERENCE_CAUSE_UNEXPECTED_ERROR;
               }
            }
            else // terminateMediaConnection failed
            {
               ptrLock->start();
               ptrLock->dropConnection(); // destroy new call shell, cannot complete split
               eventCause = CP_CONFERENCE_CAUSE_INVALID_STATE;
            }
         }
         else // findConnection failed
         {
            eventCause = CP_CONFERENCE_CAUSE_NOT_FOUND;
         }
      }
      else // findCall failed
      {
         eventCause = CP_CONFERENCE_CAUSE_NOT_FOUND;
      }
   }

   if (result == OS_SUCCESS)
   {
      fireConferenceEvent(CP_CONFERENCE_CALL_REMOVED, eventCause, &sipDialog);
   }
   else
   {
      fireConferenceEvent(CP_CONFERENCE_CALL_REMOVE_FAILURE, eventCause, &sipDialog);
   }
   return result;
}

// assumes external locking on m_memberMutex
XSipConnection* XCpConference::findConnection(const SipDialog& sipDialog) const
{
   UtlSListIterator itor(m_sipConnections);
   XSipConnection* pPartialMatchSipConnection = NULL;
   XSipConnection* pSipConnection = NULL;

   while (itor())
   {
      pSipConnection = dynamic_cast<XSipConnection*>(itor.item());
      if (pSipConnection)
      {
         SipDialog::DialogMatchEnum tmpResult = pSipConnection->compareSipDialog(sipDialog);
         if (tmpResult == SipDialog::DIALOG_ESTABLISHED_MATCH ||
            tmpResult == SipDialog::DIALOG_INITIAL_INITIAL_MATCH)
         {
            // return immediately if found perfect match for established dialog
            // initial match is also perfect if supplied dialog is initial
            return pSipConnection;
         }
         else if (tmpResult != SipDialog::DIALOG_MISMATCH)
         {
            // only override result if we found some match, as we could have some connection at the end of list that would not match
            pPartialMatchSipConnection = pSipConnection;
         }
      }
   }

   if (pPartialMatchSipConnection)
   {
      // return partial match at the end
      return pPartialMatchSipConnection;
   }

   return NULL;
}

void XCpConference::destroyAllSipConnections()
{
   UtlSList sipDialogList;

   // prepare list of all SipDialogs
   {
      OsLock lock(m_memberMutex);
      UtlSListIterator itor(m_sipConnections);
      XSipConnection* pSipConnection = NULL;

      while (itor())
      {
         pSipConnection = dynamic_cast<XSipConnection*>(itor.item());
         if (pSipConnection)
         {
            SipDialog *pSipDialog = new SipDialog();
            pSipConnection->getSipDialog(*pSipDialog);
            sipDialogList.append(pSipDialog);
         }
      }
   }

   // destroy connections one by one without holding global lock
   UtlSListIterator itor(sipDialogList);
   while (itor())
   {
      SipDialog *pSipDialog = dynamic_cast<SipDialog*>(itor.item());
      destroySipConnection(*pSipDialog);
   }

   sipDialogList.destroyAll();
}

void XCpConference::destroySipConnection(const SipDialog& sSipDialog, UtlBoolean bFireEvents)
{
   UtlString sSipCallId;
   {
      OsLock lock(m_memberMutex);
      XSipConnection* pSipConnection = findConnection(sSipDialog);
      if (pSipConnection)
      {
         pSipConnection->acquireExclusive();
         pSipConnection->getSipCallId(sSipCallId);
         m_sipConnections.remove(pSipConnection);
         if (bFireEvents)
         {
            // fire event that call was removed from conference
            fireConferenceEvent(CP_CONFERENCE_CALL_REMOVED, CP_CONFERENCE_CAUSE_NORMAL, &sSipDialog);
         }

         delete pSipConnection;
         pSipConnection = NULL;
      }
   }

   if (!sSipCallId.isNull())
   {
      // we destroyed some connection, notify call stack
      onConnectionRemoved(sSipCallId);
   }
}

void XCpConference::destroySipConnection(const SipDialog& sSipDialog)
{
   destroySipConnection(sSipDialog, TRUE);
}

void XCpConference::createSipConnection(const SipDialog& sipDialog, const UtlString& sFullLineUrl, UtlBoolean bFireEvents)
{
   XSipConnection *pSipConnection = new XSipConnection(m_sId,
      sipDialog,// used only temporarily until real dialog instance is created when connecting
      m_rSipUserAgent, m_rCallControl, sFullLineUrl, m_sBindIpAddress,
      m_sessionTimerExpiration, m_sessionTimerRefresh,
      m_updateSetting, m_100relSetting, m_sdpOfferingMode, m_inviteExpiresSeconds, this, this, m_natTraversalConfig,
      m_pCallEventListener, m_pInfoStatusEventListener, m_pInfoEventListener, m_pSecurityEventListener, m_pMediaEventListener,
      m_pRtpRedirectEventListener);

   {
      OsLock lock(m_memberMutex);
      m_sipConnections.append(pSipConnection);
   }

   if (bFireEvents)
   {
      // fire event that call was added to conference
      fireConferenceEvent(CP_CONFERENCE_CALL_ADDED, CP_CONFERENCE_CAUSE_NORMAL, &sipDialog);
   }

   onConnectionAddded(sipDialog.getCallId());
}

void XCpConference::fireSipXMediaConnectionEvent(CP_MEDIA_EVENT event,
                                                 CP_MEDIA_CAUSE cause,
                                                 CP_MEDIA_TYPE type,
                                                 int mediaConnectionId,
                                                 intptr_t pEventData1,
                                                 intptr_t pEventData2)
{
   OsLock lock(m_memberMutex);

   UtlSListIterator itor(m_sipConnections);
   XSipConnection* pSipConnection = NULL;

   while (itor())
   {
      pSipConnection = dynamic_cast<XSipConnection*>(itor.item());
      if (pSipConnection && pSipConnection->getMediaEventConnectionId() == mediaConnectionId)
      {
         pSipConnection->handleSipXMediaEvent(event, cause, type, pEventData1, pEventData2);
      }
   }
}

void XCpConference::fireSipXMediaInterfaceEvent(CP_MEDIA_EVENT event,
                                                CP_MEDIA_CAUSE cause,
                                                CP_MEDIA_TYPE type,
                                                intptr_t pEventData1,
                                                intptr_t pEventData2)
{
   OsLock lock(m_memberMutex);

   UtlSListIterator itor(m_sipConnections);
   XSipConnection* pSipConnection = NULL;

   while (itor())
   {
      pSipConnection = dynamic_cast<XSipConnection*>(itor.item());
      if (pSipConnection)
      {
         pSipConnection->handleSipXMediaEvent(event, cause, type, pEventData1, pEventData2);
      }
   }
}

void XCpConference::fireConferenceEvent(CP_CONFERENCE_EVENT event,
                                        CP_CONFERENCE_CAUSE cause,
                                        const SipDialog* pSipDialog)
{
   if (m_pConferenceEventListener)
   {
      CpConferenceEvent eventObject(cause, m_sId, pSipDialog);
      switch(event)
      {
      case CP_CONFERENCE_CREATED:
         m_pConferenceEventListener->OnConferenceCreated(eventObject);
         break;
      case CP_CONFERENCE_DESTROYED:
         m_pConferenceEventListener->OnConferenceDestroyed(eventObject);
         break;
      case CP_CONFERENCE_CALL_ADDED:
         m_pConferenceEventListener->OnConferenceCallAdded(eventObject);
         break;
      case CP_CONFERENCE_CALL_ADD_FAILURE:
         m_pConferenceEventListener->OnConferenceCallAddFailure(eventObject);
         break;
      case CP_CONFERENCE_CALL_REMOVED:
         m_pConferenceEventListener->OnConferenceCallRemoved(eventObject);
         break;
      case CP_CONFERENCE_CALL_REMOVE_FAILURE:
         m_pConferenceEventListener->OnConferenceCallRemoveFailure(eventObject);
         break;
      default:
         ;
      }
   }
}

void XCpConference::onFocusGained()
{
   OsLock lock(m_memberMutex);

   UtlSListIterator itor(m_sipConnections);
   XSipConnection* pSipConnection = NULL;

   while (itor())
   {
      pSipConnection = dynamic_cast<XSipConnection*>(itor.item());
      if (pSipConnection)
      {
         pSipConnection->onFocusGained();
      }
   }
}

void XCpConference::onFocusLost()
{
   OsLock lock(m_memberMutex);

   UtlSListIterator itor(m_sipConnections);
   XSipConnection* pSipConnection = NULL;

   while (itor())
   {
      pSipConnection = dynamic_cast<XSipConnection*>(itor.item());
      if (pSipConnection)
      {
         pSipConnection->onFocusLost();
      }
   }
}

void XCpConference::onStarted()
{
   // this gets called once thread is started
   fireConferenceEvent(CP_CONFERENCE_CREATED, CP_CONFERENCE_CAUSE_NORMAL);
}

void XCpConference::requestConferenceDestruction()
{
   releaseMediaInterface(); // release audio resources

   fireConferenceEvent(CP_CONFERENCE_DESTROYED, CP_CONFERENCE_CAUSE_NORMAL);

   CmDestroyAbstractCallMsg msg(m_sId);
   getGlobalQueue().send(msg); // instruct call manager to destroy this conference
}

/* ============================ FUNCTIONS ================================= */
