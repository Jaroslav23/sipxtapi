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
#include <os/OsSysLog.h>
#include <os/OsWriteLock.h>
#include <os/OsReadLock.h>
#include <os/OsTimer.h>
#include <os/OsTimerNotification.h>
#include <utl/UtlSList.h>
#include <utl/UtlSListIterator.h>
#include <sdp/SdpCodecList.h>
#include <net/SipMessage.h>
#include <net/SipUserAgent.h>
#include <net/SipContact.h>
#include <mi/CpMediaInterface.h>
#include <cp/CpMessageQueueProvider.h>
#include <cp/XSipConnectionContext.h>
#include <cp/state/BaseSipConnectionState.h>
#include <cp/state/SipConnectionStateTransition.h>
#include <cp/state/StateTransitionMemory.h>
#include <cp/state/SipResponseTransitionMemory.h>
#include <cp/state/SipEventTransitionMemory.h>
#include <cp/state/GeneralTransitionMemory.h>
#include <cp/CpMediaInterfaceProvider.h>
#include <cp/msg/ScTimerMsg.h>
#include <cp/msg/ScCommandMsg.h>
#include <cp/msg/ScNotificationMsg.h>
#include <cp/msg/Sc100RelTimerMsg.h>
#include <cp/msg/Sc2xxTimerMsg.h>
#include <cp/msg/ScDisconnectTimerMsg.h>
#include <cp/msg/ScReInviteTimerMsg.h>
#include <cp/msg/ScByeRetryTimerMsg.h>
#include <cp/msg/ScSessionTimeoutTimerMsg.h>
#include <cp/msg/ScInviteExpirationTimerMsg.h>
#include <cp/msg/ScDelayedAnswerTimerMsg.h>
#include <cp/msg/AcDestroyConnectionMsg.h>
#include <cp/msg/ScConnStateNotificationMsg.h>
#include <cp/XSipConnectionEventSink.h>
#include <cp/XCpCallControl.h>

// DEFINES
#define MAX_491_FAILURES 5

// CANCEL doesn't need transaction tracking
#define TRACKABLE_METHODS "INVITE UPDATE INFO NOTIFY REFER OPTIONS PRACK SUBSCRIBE BYE"
#define MAX_RENEGOTIATION_RETRY_COUNT 10
#define MAX_100REL_RETRANSMIT_COUNT 6
#define MAX_DELAYED_ANSWER_RETRY_COUNT MAX_100REL_RETRANSMIT_COUNT + 1

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
int BaseSipConnectionState::ms_iInfoTestResponseCode = 0;

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

BaseSipConnectionState::BaseSipConnectionState(SipConnectionStateContext& rStateContext,
                                               SipUserAgent& rSipUserAgent,
                                               XCpCallControl& rCallControl,
                                               CpMediaInterfaceProvider* pMediaInterfaceProvider,
                                               CpMessageQueueProvider* pMessageQueueProvider,
                                               XSipConnectionEventSink& rSipConnectionEventSink,
                                               const CpNatTraversalConfig& natTraversalConfig)
: m_rStateContext(rStateContext)
, m_rSipUserAgent(rSipUserAgent)
, m_rCallControl(rCallControl)
, m_pMediaInterfaceProvider(pMediaInterfaceProvider)
, m_pMessageQueueProvider(pMessageQueueProvider)
, m_rSipConnectionEventSink(rSipConnectionEventSink)
, m_natTraversalConfig(natTraversalConfig)
{

}

BaseSipConnectionState::BaseSipConnectionState(const BaseSipConnectionState& rhs)
: m_rStateContext(rhs.m_rStateContext)
, m_rSipUserAgent(rhs.m_rSipUserAgent)
, m_rCallControl(rhs.m_rCallControl)
, m_pMediaInterfaceProvider(rhs.m_pMediaInterfaceProvider)
, m_pMessageQueueProvider(rhs.m_pMessageQueueProvider)
, m_rSipConnectionEventSink(rhs.m_rSipConnectionEventSink)
, m_natTraversalConfig(rhs.m_natTraversalConfig)
{

}

BaseSipConnectionState::~BaseSipConnectionState()
{

}

/* ============================ MANIPULATORS ============================== */

void BaseSipConnectionState::handleStateEntry(StateEnum previousState, const StateTransitionMemory* pTransitionMemory)
{
}

void BaseSipConnectionState::handleStateExit(StateEnum nextState, const StateTransitionMemory* pTransitionMemory)
{
}

SipConnectionStateTransition* BaseSipConnectionState::handleSipMessageEvent(const SipMessageEvent& rEvent)
{
   const SipMessage* pSipMessage = rEvent.getMessage();
   int messageType = rEvent.getMessageStatus();

   switch(messageType)
   {
      case SipMessageEvent::TRANSPORT_ERROR:
         {
            // drop the whole dialog
            SipEventTransitionMemory memory(messageType);
            return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, &memory);
         }
      case SipMessageEvent::AUTHENTICATION_RETRY:
         {
            if (pSipMessage)
            {
               return handleAuthenticationRetryEvent(*pSipMessage);
            }
            break;
         }
      case SipMessageEvent::APPLICATION:
         {
            // normal sip message received
            if (pSipMessage)
            {
               return processSipMessage(*pSipMessage);
            }
            break;
         }
      default:
         // ignore, not interesting for us
         ;
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::connect(OsStatus& result,
                                                              const UtlString& sipCallId,
                                                              const UtlString& localTag,
                                                              const UtlString& toAddress,
                                                              const UtlString& fromAddress,
                                                              const UtlString& locationHeader,
                                                              CP_CONTACT_ID contactId,
                                                              SIP_TRANSPORT_TYPE transport,
                                                              const UtlString& replacesField)
{
   // we reject connect in all states except for Dialing
   result = OS_FAILED;
   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::startRtpRedirect(OsStatus& result,
                                                                       const UtlString& slaveAbstractCallId,
                                                                       const SipDialog& slaveSipDialog)
{
   result = OS_FAILED;
   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::stopRtpRedirect(OsStatus& result)
{
   result = OS_FAILED;
   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::acceptConnection(OsStatus& result,
                                                                       UtlBoolean bSendSDP,
                                                                       const UtlString& locationHeader,
                                                                       CP_CONTACT_ID contactId,
                                                                       SIP_TRANSPORT_TYPE transport)
{
   result = OS_FAILED;
   // here we send either 180 or 183, and may proceed to alerting state

   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState == ISipConnectionState::CONNECTION_OFFERING ||
      connectionState == ISipConnectionState::CONNECTION_QUEUED)
   {
      if (m_rStateContext.m_pLastReceivedInvite)
      {
         Url requestUri;
         m_rStateContext.m_pLastReceivedInvite->getRequestUri(requestUri);
         m_rStateContext.m_transportType = transport;
         initDialogContact(contactId, requestUri);
         m_rStateContext.m_locationHeader = locationHeader;

         SipMessage sipResponse;
         ERROR_RESPONSE_TYPE errorResponseType = ERROR_RESPONSE_488;
         UtlBoolean bProtocolError = TRUE;
         const SdpBody* pSdpBody = m_rStateContext.m_pLastReceivedInvite->getSdpBody(m_rStateContext.m_pSecurity);
         UtlBoolean bSend100rel = shouldSend100relResponse();

         if (pSdpBody)
         {
            // there was SDP in INVITE, handle it
            if (handleSdpOffer(*m_rStateContext.m_pLastReceivedInvite))
            {
               sipResponse.setResponseData(m_rStateContext.m_pLastReceivedInvite, SIP_RINGING_CODE,
                  SIP_RINGING_TEXT, getLocalContactUrl());
               if (bSend100rel)
               {
                  sipResponse.addRequireExtension(SIP_PRACK_EXTENSION);
               }
               // SDP offer was handled, we may add SDP answer
               if (bSendSDP)
               {
                  // SDP was in INVITE, and we also want it in 18x
                  if (prepareSdpAnswer(sipResponse))
                  {
                     if (commitMediaSessionChanges()) // commit media session changes, so that we can hear early audio
                     {
                        bProtocolError = FALSE;
                     }
                  }
               }
               else
               {
                  // SDP was in INVITE, but we don't want it in 18x
                  bProtocolError = FALSE;
               }
            }
         }
         else
         {
            sipResponse.setResponseData(m_rStateContext.m_pLastReceivedInvite, SIP_RINGING_CODE,
               SIP_RINGING_TEXT, getLocalContactUrl());
            // there is no SDP in INVITE
            if (bSend100rel || bSendSDP)
            {
               if (bSend100rel)
               {
                  sipResponse.addRequireExtension(SIP_PRACK_EXTENSION);
               }
               // if we are sending 18x reliably, we must include SDP offer
               if (prepareSdpOffer(sipResponse))
               {
                  bProtocolError = FALSE;
               }
            }
            else
            {
               // we send 18x unreliably without SDP
               bProtocolError = FALSE;
            }
         }

         if (bProtocolError)
         {
            // reject call
            SipMessage errorResponse;
            prepareErrorResponse(*m_rStateContext.m_pLastReceivedInvite, errorResponse, errorResponseType);
            sendMessage(errorResponse);

            GeneralTransitionMemory memory(CP_CALLSTATE_CAUSE_REQUEST_NOT_ACCEPTED);
            return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, &memory);
         }
         else
         {
            UtlBoolean res = FALSE;
            if (shouldSend100relResponse())
            {
               res = sendReliableResponse(sipResponse);
            }
            else
            {
               // send prepared message
               res = sendMessage(sipResponse);
            }
            if (res)
            {
               result = OS_SUCCESS;
               return getTransition(ISipConnectionState::CONNECTION_ALERTING, NULL);
            }
         }
      }
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::rejectConnection(OsStatus& result)
{
   result = OS_FAILED;

   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState == ISipConnectionState::CONNECTION_OFFERING ||
      connectionState == ISipConnectionState::CONNECTION_ALERTING ||
      connectionState == ISipConnectionState::CONNECTION_QUEUED)
   {
      if (m_rStateContext.m_pLastReceivedInvite)
      {
         SipMessage sipResponse;
         if (connectionState == ISipConnectionState::CONNECTION_OFFERING)
         {
            sipResponse.setInviteBusyData(m_rStateContext.m_pLastReceivedInvite);
         }
         else if (connectionState == ISipConnectionState::CONNECTION_ALERTING)
         {
            sipResponse.setRequestTerminatedResponseData(m_rStateContext.m_pLastReceivedInvite);
         }
         deleteInviteExpirationTimer(); // INVITE was answered

         if (sendMessage(sipResponse))
         {
            result = OS_SUCCESS;
         }

         GeneralTransitionMemory memory(CP_CALLSTATE_CAUSE_REJECTED);
         return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, &memory);
      }
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::redirectConnection(OsStatus& result,
                                                                         const UtlString& sRedirectSipUrl)
{
   result = OS_FAILED;

   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState == ISipConnectionState::CONNECTION_OFFERING ||
       connectionState == ISipConnectionState::CONNECTION_ALERTING ||
       connectionState == ISipConnectionState::CONNECTION_QUEUED)
   {
      if (m_rStateContext.m_pLastReceivedInvite)
      {
         SipMessage redirectResponse;
         redirectResponse.setForwardResponseData(m_rStateContext.m_pLastReceivedInvite, sRedirectSipUrl);

         deleteInviteExpirationTimer(); // INVITE was answered

         if (sendMessage(redirectResponse))
         {
            result = OS_SUCCESS;
         }

         GeneralTransitionMemory memory(CP_CALLSTATE_CAUSE_REDIRECTED);
         return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, &memory);
      }
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::answerConnection(OsStatus& result)
{
   result = OS_FAILED;
   // here we send 200 OK and proceed to connected state

   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if ((connectionState == ISipConnectionState::CONNECTION_ALERTING ||
      connectionState == ISipConnectionState::CONNECTION_QUEUED) &&
      m_rStateContext.m_pLastReceivedInvite)
   {
      if (m_rStateContext.m_100RelTracker.are1xxRelsAcknowledged())
      {
         ERROR_RESPONSE_TYPE errorResponseType = ERROR_RESPONSE_500;
         SipMessage sipResponse;
         UtlString sLocalContact(getLocalContactUrl());
         UtlBoolean bProtocolError = TRUE;
         const SdpBody* pSdpBody = m_rStateContext.m_pLastReceivedInvite->getSdpBody(m_rStateContext.m_pSecurity);
         sipResponse.setOkResponseData(m_rStateContext.m_pLastReceivedInvite, sLocalContact);

         if (pSdpBody)
         {
            if (m_rStateContext.m_sdpNegotiation.isInSdpNegotiation(*m_rStateContext.m_pLastReceivedInvite) &&
               m_rStateContext.m_sdpNegotiation.isSdpNegotiationInProgress())
            {
               // there was body in INVITE message, handle it
               if (handleSdpOffer(*m_rStateContext.m_pLastReceivedInvite))
               {
                  // SDP offer was handled, we may add SDP answer
                  if (prepareSdpAnswer(sipResponse))
                  {
                     if (commitMediaSessionChanges()) // commit media session changes, so that we can hear early audio
                     {
                        bProtocolError = FALSE;
                     }
                  }
               }
            }
            else
            {
               // SDP negotiation was already handled, no need to respond to SDP
               bProtocolError = FALSE;
            }

            if (bProtocolError)
            {
               errorResponseType = ERROR_RESPONSE_488;
            }
         }
         else
         {
            // there was no offer in INVITE, send offer in 200 OK, regardless if SDP negotiation is finished
            if (prepareSdpOffer(sipResponse))
            {
               bProtocolError = FALSE;
            }
         }

         deleteInviteExpirationTimer(); // INVITE was answered

         if (bProtocolError)
         {
            // reject call
            SipMessage errorResponse;
            prepareErrorResponse(*m_rStateContext.m_pLastReceivedInvite, errorResponse, errorResponseType);
            sendMessage(errorResponse);

            GeneralTransitionMemory memory(CP_CALLSTATE_CAUSE_REQUEST_NOT_ACCEPTED);
            return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, &memory);
         }
         else
         {
            m_rStateContext.m_bAckReceived = FALSE;
            m_rStateContext.m_i2xxInviteRetransmitCount = 0;
            setLastSent2xxToInvite(sipResponse);
            prepareSessionTimerResponse(*m_rStateContext.m_pLastReceivedInvite, sipResponse); // construct session timer response
            handleSessionTimerResponse(sipResponse); // handle our own response, start session timer..
            sendMessage(sipResponse);
            start2xxRetransmitTimer();
            result = OS_SUCCESS;
            return getTransition(ISipConnectionState::CONNECTION_ESTABLISHED, NULL);
         }
      }
      else
      {
         // some reliable responses have not been acknowledged yet, delay answering
         if (m_rStateContext.m_iDelayedAnswerCount < MAX_DELAYED_ANSWER_RETRY_COUNT)
         {
            startDelayedAnswerTimer(); // we will try to answer call again when timer fires
            m_rStateContext.m_iDelayedAnswerCount++;
         }
         else
         {
            // reject call
            SipMessage errorResponse;
            errorResponse.setResponseData(m_rStateContext.m_pLastReceivedInvite, SIP_DECLINE_CODE,
               SIP_DECLINE_TEXT, getLocalContactUrl());
            sendMessage(errorResponse);
            GeneralTransitionMemory memory(CP_CALLSTATE_CAUSE_NO_RESPONSE);
            return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, &memory);
         }
      }
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::acceptTransfer(OsStatus& result)
{
   result = OS_FAILED;
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState == ISipConnectionState::CONNECTION_ESTABLISHED &&
      m_rStateContext.m_localEntityType == SipConnectionStateContext::ENTITY_TRANSFEREE &&
      m_rStateContext.m_inboundReferResponse == SipConnectionStateContext::REFER_NO_RESPONSE &&
      m_rStateContext.m_pLastReceivedRefer)
   {
      if (followRefer(*m_rStateContext.m_pLastReceivedRefer))
      {
         // REFER is ok, we started dialing new call
         UtlBoolean bNoreferSubGranted = FALSE;
         m_rStateContext.m_referOutSubscriptionActive = TRUE;
         if (isMethodAllowed(SIP_NOTIFY_METHOD))
         {
            if (isExtensionSupported(SIP_NO_REFER_SUB_EXTENSION))
            {
               UtlBoolean referSubField = TRUE;
               if (m_rStateContext.m_pLastReceivedRefer->getReferSubField(referSubField) && !referSubField)
               {
                  // sender doesn't want implicit subscription
                  m_rStateContext.m_referOutSubscriptionActive = FALSE;
                  bNoreferSubGranted = TRUE;
               }
            }
         }
         else
         {
            m_rStateContext.m_referOutSubscriptionActive = FALSE;
         }

         // send 202 Accepted
         SipMessage sipResponse;
         sipResponse.setResponseData(m_rStateContext.m_pLastReceivedRefer, SIP_ACCEPTED_CODE, SIP_ACCEPTED_TEXT, getLocalContactUrl());
         if (bNoreferSubGranted)
         {
            sipResponse.setReferSubField(FALSE); // grant norefersub
         }
         sendMessage(sipResponse);
         m_rStateContext.m_inboundReferResponse = SipConnectionStateContext::REFER_ACCEPTED;
         result = OS_SUCCESS;
      }
      else
      {
         OsStatus rejectResult;
         return rejectTransfer(rejectResult);
      }
   } // else invalid state, ignore

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::rejectTransfer(OsStatus& result)
{
   result = OS_FAILED;
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState == ISipConnectionState::CONNECTION_ESTABLISHED &&
      m_rStateContext.m_localEntityType == SipConnectionStateContext::ENTITY_TRANSFEREE &&
      m_rStateContext.m_inboundReferResponse == SipConnectionStateContext::REFER_NO_RESPONSE &&
      m_rStateContext.m_pLastReceivedRefer)
   {
      // send reject response
      SipMessage errorResponse;
      prepareErrorResponse(*m_rStateContext.m_pLastReceivedRefer, errorResponse, ERROR_RESPONSE_603);
      sendMessage(errorResponse);
      m_rStateContext.m_localEntityType = SipConnectionStateContext::ENTITY_NORMAL;
      // fire connected event, since we are no longer in transfer
      m_rSipConnectionEventSink.fireSipXCallEvent(CP_CALLSTATE_CONNECTED, CP_CALLSTATE_CAUSE_NORMAL);
      delete m_rStateContext.m_pLastReceivedRefer;
      m_rStateContext.m_pLastReceivedRefer = NULL;
   } // else invalid state, ignore

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::dropConnection(OsStatus& result)
{
   // implemented in subclasses
   result = OS_FAILED;
   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::transferBlind(OsStatus& result,
                                                                    const UtlString& sTransferSipUrl)
{
   if (!isMethodAllowed(SIP_REFER_METHOD))
   {
      // remote party must support REFER
      result = OS_FAILED;
      return NULL;
   }

   Url transferSipUrl(sTransferSipUrl);
   transferSipUrl.removeFieldParameters(); // do not remove all parameters
   transferSipUrl.setDisplayName(NULL);
   transferSipUrl.setPassword(NULL);
   transferSipUrl.includeAngleBrackets();

   result = transferCall(transferSipUrl);
   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::transferConsultative(OsStatus& result,
                                                                           const SipDialog& targetSipDialog)
{
   if (!isMethodAllowed(SIP_REFER_METHOD))
   {
      // remote party must support REFER
      result = OS_FAILED;
      return NULL;
   }

   // targetSipDialog identifies call we want to transfer remote participant of our current call to
   // targetSipDialog is some dialog owned by our local call manager

   Url referToUrl;
   UtlString callId;
   UtlString fromTag;
   UtlString toTag;

   if (!targetSipDialog.isLocalInitiatedDialog())
   {
      // for inbound calls use remote field
      targetSipDialog.getRemoteField(referToUrl);
   }
   else
   {
      // for outbound calls use request uri
      targetSipDialog.getRemoteRequestUri(referToUrl);
   }
   referToUrl.removeParameters();
   referToUrl.setDisplayName(NULL);
   referToUrl.setPassword(NULL);
   referToUrl.includeAngleBrackets();
   // construct ?Replaces= ...
   targetSipDialog.getCallId(callId);
   // local tag is compared with to tag in replaces, but we have to swap them, since INVITE will go to remote party, not us
   targetSipDialog.getLocalTag(fromTag); 
   targetSipDialog.getRemoteTag(toTag);
   UtlString replacesParam;
   replacesParam.appendFormat("%s;to-tag=%s;from-tag=%s", callId.data(), toTag.data(), fromTag.data());
   referToUrl.setHeaderParameter("Replaces", replacesParam);

   // Example: Refer-To: <sip:bob@example.org?Replaces=12345%40192.168.118.3%3Bto-tag%3D12345%3Bfrom-tag%3D5FFE-3994>
   result = transferCall(referToUrl);
   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::holdConnection(OsStatus& result)
{
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState == ISipConnectionState::CONNECTION_ESTABLISHED)
   {
      if (mayRenegotiateMediaSession())
      {
         // no invite transaction, we may start hold immediately
         doHold();
         result = OS_SUCCESS;
         return NULL;
      }
      else // some invite is active
      {
         if (m_rStateContext.m_pSessionRenegotiationTimer == NULL)
         {
            // timer is not started yet, schedule hold
            startSessionRenegotiationTimer(ScReInviteTimerMsg::REASON_HOLD, TRUE);
            result = OS_SUCCESS;
            return NULL;
         } // else fail
      }
   }

   result = OS_FAILED;
   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::unholdConnection(OsStatus& result)
{
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState == ISipConnectionState::CONNECTION_ESTABLISHED)
   {
      if (mayRenegotiateMediaSession())
      {
         // no invite transaction, we may start unhold immediately
         doUnhold();
         result = OS_SUCCESS;
         return NULL;
      }
      else // some invite is active
      {
         if (m_rStateContext.m_pSessionRenegotiationTimer == NULL)
         {
            // timer is not started yet, schedule unhold
            startSessionRenegotiationTimer(ScReInviteTimerMsg::REASON_UNHOLD, TRUE);
            result = OS_SUCCESS;
            return NULL;
         } // else fail
      }
   }

   result = OS_FAILED;
   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::renegotiateCodecsConnection(OsStatus& result)
{
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState == ISipConnectionState::CONNECTION_ESTABLISHED)
   {
      if (mayRenegotiateMediaSession())
      {
         // no invite transaction, we may renegotiate immediately
         refreshSession(TRUE);
         result = OS_SUCCESS;
         return NULL;
      }
      else // some invite/update is active
      {
         if (m_rStateContext.m_pSessionRenegotiationTimer == NULL)
         {
            // timer is not started yet, schedule renegotiation
            startSessionRenegotiationTimer(ScReInviteTimerMsg::REASON_NORMAL, TRUE);
            result = OS_SUCCESS;
            return NULL;
         } // else fail
      }
   }

   result = OS_FAILED;
   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::sendInfo(OsStatus& result,
                                                               const UtlString& sContentType,
                                                               const char* pContent,
                                                               const size_t nContentLength,
                                                               void* pCookie)
{
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState != ISipConnectionState::CONNECTION_DISCONNECTED &&
      connectionState != ISipConnectionState::CONNECTION_UNKNOWN)
   {
      SipDialog::DialogState dialogState;
      {
         OsReadLock lock(m_rStateContext);
         dialogState = m_rStateContext.m_sipDialog.getDialogState();
      }
      if (dialogState == SipDialog::DIALOG_STATE_ESTABLISHED)
      {
         // dialog is established (early or confirmed), we may send info
         int iCSeq = getNextLocalCSeq();
         // start transaction manually because we need to store custom data in it
         getClientTransactionManager().startTransaction(SIP_INFO_METHOD, iCSeq);
         getClientTransactionManager().setTransactionData(SIP_INFO_METHOD, iCSeq, pCookie); // assign cookie to transaction
         SipMessage infoRequest;
         prepareSipRequest(infoRequest, SIP_INFO_METHOD, iCSeq);

         if (!sContentType.isNull())
         {
            infoRequest.setContentType(sContentType);
         }
         if (nContentLength > 0)
         {
            // set INFO payload
            infoRequest.setContentLength(nContentLength);
            HttpBody* pBody = new HttpBody(pContent, nContentLength);
            infoRequest.setBody(pBody);
         }

         // try to send
         if (sendMessage(infoRequest))
         {
            result = OS_SUCCESS;
            return NULL;
         }
         else
         {
            m_rSipConnectionEventSink.fireSipXInfoStatusEvent(CP_INFOSTATUS_NETWORK_ERROR, SIPXTACK_MESSAGE_FAILURE,
               NULL, 0, pCookie);
         }
      }
   }

   result = OS_FAILED;
   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::terminateMediaConnection(OsStatus& result)
{
   ISipConnectionState::StateEnum connectionState = getCurrentState();
   result = OS_FAILED;

   if (connectionState == ISipConnectionState::CONNECTION_ESTABLISHED)
   {
      if (mayRenegotiateMediaSession())
      {
         // only allow termination of media connection if there is no active UPDATE or INVITE
         deleteMediaConnection();
         result = OS_SUCCESS;
      }
      else
      {
         result = OS_BUSY;
      }
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::handleTimerMessage(const ScTimerMsg& timerMsg)
{
   switch (timerMsg.getPayloadType())
   {
   case ScTimerMsg::PAYLOAD_TYPE_100REL:
      return handle100RelTimerMessage((const Sc100RelTimerMsg&)timerMsg);
   case ScTimerMsg::PAYLOAD_TYPE_2XX:
      return handle2xxTimerMessage((const Sc2xxTimerMsg&)timerMsg);
   case ScTimerMsg::PAYLOAD_TYPE_DISCONNECT:
      return handleDisconnectTimerMessage((const ScDisconnectTimerMsg&)timerMsg);
   case ScTimerMsg::PAYLOAD_TYPE_REINVITE:
      return handleReInviteTimerMessage((const ScReInviteTimerMsg&)timerMsg);
   case ScTimerMsg::PAYLOAD_TYPE_BYE_RETRY:
      return handleByeRetryTimerMessage((const ScByeRetryTimerMsg&)timerMsg);
   case ScTimerMsg::PAYLOAD_TYPE_SESSION_TIMEOUT_CHECK:
      return handleSessionTimeoutCheckTimerMessage((const ScSessionTimeoutTimerMsg&)timerMsg);
   case ScTimerMsg::PAYLOAD_TYPE_INVITE_EXPIRATION:
      return handleInviteExpirationTimerMessage((const ScInviteExpirationTimerMsg&)timerMsg);
   case ScTimerMsg::PAYLOAD_TYPE_DELAYED_ANSWER:
      return handleDelayedAnswerTimerMessage((const ScDelayedAnswerTimerMsg&)timerMsg);
   default:
      ;
      OsSysLog::add(FAC_CP, PRI_WARNING, "Unknown timer message received - %d:%d\r\n",
         (int)timerMsg.getMsgSubType(), (int)timerMsg.getPayloadType());
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::handleCommandMessage(const ScCommandMsg& rMsg)
{
   // subclass ScCommandMsg and call specific handling function
   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::handleNotificationMessage(const ScNotificationMsg& rMsg)
{
   ScNotificationMsg::SubTypesEnum type = (ScNotificationMsg::SubTypesEnum)rMsg.getMsgSubType();

   switch (type)
   {
   case ScNotificationMsg::SCN_CONNECTION_STATE:
      return handleConnStateNotificationMessage((const ScConnStateNotificationMsg&)rMsg);
   default:
      ;
   }

   return NULL;
}

/* ============================ ACCESSORS ================================= */

void BaseSipConnectionState::setMessageQueueProvider(CpMessageQueueProvider* pMessageQueueProvider)
{
   m_pMessageQueueProvider = pMessageQueueProvider;
   // we also need to update timers with new queue
   updateAllTimers();
}

void BaseSipConnectionState::setMediaInterfaceProvider(CpMediaInterfaceProvider* pMediaInterfaceProvider)
{
   m_pMediaInterfaceProvider = pMediaInterfaceProvider;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

SipConnectionStateTransition* BaseSipConnectionState::getTransitionObject(BaseSipConnectionState* pDestination,
                                                                          const StateTransitionMemory* pTansitionMemory) const
{
   if (this != pDestination)
   {
      StateTransitionMemory* pTansitionMemoryCopy = NULL;
      if (pTansitionMemory)
      {
         pTansitionMemoryCopy = pTansitionMemory->clone();
      }
      // construct transition from this to destination state, containing copy of state transition memory object
      SipConnectionStateTransition* pTransition = new SipConnectionStateTransition(this, pDestination,
         pTansitionMemoryCopy);
      return pTransition;
   }
   else
   {
      return NULL;
   }
}

void BaseSipConnectionState::updateSipDialog(const SipMessage& sipMessage)
{
   OsWriteLock lock(m_rStateContext);
   m_rStateContext.m_sipDialog.updateDialog(sipMessage);
}

void BaseSipConnectionState::initializeSipDialog(const SipMessage& sipMessage)
{
   OsWriteLock lock(m_rStateContext);
   m_rStateContext.m_sipDialog = SipDialog(&sipMessage);
}

UtlBoolean BaseSipConnectionState::sendMessage(SipMessage& sipMessage)
{
   if (sipMessage.isResponse() && !isLocalInitiatedDialog())
   {
      // response & we are responsible for generating to tag
      int responseCode = sipMessage.getResponseStatusCode();
      if (responseCode > SIP_1XX_CLASS_CODE)
      {
         UtlString msgToTag;
         sipMessage.getToFieldTag(msgToTag);
         if (msgToTag.isNull())
         {
            // responses must have a tag. We add it here so that it is never forgotten
            UtlString toTag(getLocalTag());
            sipMessage.setToFieldTag(toTag);
         }
      }
   }

   if (sipMessage.isRequest())
   {
      configureRequestTransport(sipMessage);
      trackTransactionRequest(sipMessage);
   }
   else
   {
      trackTransactionResponse(sipMessage);
   }

   sipMessage.setSecurityAttributes(m_rStateContext.m_pSecurity);
   if (!m_rStateContext.m_locationHeader.isNull())
   {
      sipMessage.setLocationField(m_rStateContext.m_locationHeader);
   }

   if (m_rStateContext.m_sBindIpAddress.compareTo("0.0.0.0") != 0)
   {
      // if its not 0.0.0.0, then use it as local IP of sip message
      sipMessage.setLocalIp(m_rStateContext.m_sBindIpAddress);
   }

   if (m_rStateContext.m_contactId != AUTOMATIC_CONTACT_ID)
   {
      // we are not using automatic contact, disable contact override
      sipMessage.allowContactOverride(FALSE);
   }

   UtlBoolean res =  m_rSipUserAgent.send(sipMessage);
   if (!res)
   {
      // send failure, terminate transaction
      int seqNum;
      UtlString seqMethod;
      sipMessage.getCSeqField(&seqNum, &seqMethod);
      if (sipMessage.isRequest())
      {
         getClientTransactionManager().endTransaction(seqMethod, seqNum);
      }
      else
      {
         getServerTransactionManager().endTransaction(seqMethod, seqNum);
      }
   }
   else
   {
      // send success
      m_rStateContext.m_loopDetector.onMessageSent(sipMessage); // notify loop detector once Via might be known
   }

   return res;
}

UtlBoolean BaseSipConnectionState::sendReliableResponse(SipMessage& sipMessage)
{
   if (sipMessage.isResponse() && m_rStateContext.m_100RelTracker.canSend1xxRel())
   {
      if (!sipMessage.isRequireExtensionSet(SIP_PRACK_EXTENSION))
      {
         sipMessage.addRequireExtension(SIP_PRACK_EXTENSION);
      }

      int rseqNum = m_rStateContext.m_100RelTracker.getNextRSeq();
      sipMessage.setRSeqField(rseqNum);
      // start retransmit timer every T1 ms
      m_rStateContext.m_i100relRetransmitCount = 0;
      start100relRetransmitTimer(sipMessage);
      UtlString s100relId;
      m_rStateContext.m_100RelTracker.on100RelSent(sipMessage, s100relId);
      // send message
      return sendMessage(sipMessage);
   }

   return FALSE;
}

SipConnectionStateTransition* BaseSipConnectionState::processSipMessage(const SipMessage& sipMessage)
{
   {
      OsWriteLock lock(m_rStateContext);
      m_rStateContext.m_sipDialog.updateDialog(sipMessage); // update dialog with received message
   }

   if (sipMessage.isRequest())
   {
      return processRequest(sipMessage);
   }
   else
   {
      return processResponse(sipMessage);
   }
}

SipConnectionStateTransition* BaseSipConnectionState::processRequest(const SipMessage& sipMessage)
{
   trackTransactionRequest(sipMessage);

   if (!verifyInboundRequest(sipMessage))
   {
      // message didn't satisfy some checks
      return NULL;
   }

   updateRemoteCapabilities(sipMessage);

   // process inbound sip message request
   UtlString method;
   sipMessage.getRequestMethod(&method);

   if (method.compareTo(SIP_INVITE_METHOD) == 0)
   {
      return processInviteRequest(sipMessage);
   }
   else if (method.compareTo(SIP_UPDATE_METHOD) == 0)
   {
      return processUpdateRequest(sipMessage);
   }
   else if (method.compareTo(SIP_ACK_METHOD) == 0)
   {
      return processAckRequest(sipMessage);
   }
   else if (method.compareTo(SIP_BYE_METHOD) == 0)
   {
      return processByeRequest(sipMessage);
   }
   else if (method.compareTo(SIP_CANCEL_METHOD) == 0)
   {
      return processCancelRequest(sipMessage);
   }
   else if (method.compareTo(SIP_INFO_METHOD) == 0)
   {
      return processInfoRequest(sipMessage);
   }
   else if (method.compareTo(SIP_NOTIFY_METHOD) == 0)
   {
      return processNotifyRequest(sipMessage);
   }
   else if (method.compareTo(SIP_OPTIONS_METHOD) == 0)
   {
      return processOptionsRequest(sipMessage);
   }
   else if (method.compareTo(SIP_REFER_METHOD) == 0)
   {
      return processReferRequest(sipMessage);
   }
   else if (method.compareTo(SIP_SUBSCRIBE_METHOD) == 0)
   {
      return processSubscribeRequest(sipMessage);
   }
   else if (method.compareTo(SIP_PRACK_METHOD) == 0)
   {
      return processPrackRequest(sipMessage);
   }
   else
   {
      // unsupported seqMethod. This can only occur if in XCpCallManager we observe sip messages that aren't supported here
      SipMessage notImplemented;
      notImplemented.setRequestUnimplemented(&sipMessage);
      m_rSipUserAgent.send(notImplemented);
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::processInviteRequest(const SipMessage& sipMessage)
{
   // initial INVITE is handled in newcall state
   // re-INVITE is handled in established state
   ISipConnectionState::StateEnum connectionState = getCurrentState();
   CpSipTransactionManager::TransactionState transactionState = getServerTransactionManager().getTransactionState(sipMessage);

   if (connectionState != ISipConnectionState::CONNECTION_ESTABLISHED &&
       connectionState != ISipConnectionState::CONNECTION_DISCONNECTED &&
       connectionState != ISipConnectionState::CONNECTION_UNKNOWN)
   {
      if (transactionState == CpSipTransactionManager::TRANSACTION_NOT_FOUND)
      {
         // this is some unknown re-INVITE, but we have another INVITE transaction running
         if (!isLocalInitiatedDialog() && !m_rStateContext.m_pLastSent2xxToInvite)
         {
            // this is an inbound call and we haven't sent 200 OK yet
            SipMessage sipResponse;
            prepareErrorResponse(sipMessage, sipResponse, ERROR_RESPONSE_500);
            sipResponse.setRetryAfterField(2); // retry after 2 seconds
            sendMessage(sipResponse);
         }
         else
         {
            // there is an INVITE in progress
            SipMessage sipResponse;
            prepareErrorResponse(sipMessage, sipResponse, ERROR_RESPONSE_491);
            sendMessage(sipResponse);
         }
      } // if transaction is active or terminated then ignore INVITE, since we are resending 200 OK
      else if (transactionState == CpSipTransactionManager::TRANSACTION_ACTIVE)
      {
         // check if outbound INVITE transaction is also active
         if (getClientTransactionManager().isInviteTransactionActive())
         {
            // outbound invite transaction was started first, then inbound INVITE came
            SipMessage sipResponse;
            prepareErrorResponse(sipMessage, sipResponse, ERROR_RESPONSE_500);
            sipResponse.setRetryAfterField(2); // retry after 2 seconds
            sendMessage(sipResponse);
         }
      }
   } // for established there is special handler
   else if (connectionState == ISipConnectionState::CONNECTION_DISCONNECTED ||
            connectionState == ISipConnectionState::CONNECTION_UNKNOWN)
   {
      SipMessage errorResponse;
      prepareErrorResponse(sipMessage, errorResponse, ERROR_RESPONSE_481);
      sendMessage(errorResponse);
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::processUpdateRequest(const SipMessage& sipMessage)
{
   int iSeqNumber = 0;
   UtlString seqMethod;
   sipMessage.getCSeqField(iSeqNumber, seqMethod);
   SipMessage sipResponse;
   UtlBoolean bProtocolError = TRUE;
   ERROR_RESPONSE_TYPE errorResponseType = ERROR_RESPONSE_488;
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (m_rStateContext.m_updateSetting != CP_SIP_UPDATE_DISABLED &&
       connectionState != ISipConnectionState::CONNECTION_DISCONNECTED &&
       connectionState != ISipConnectionState::CONNECTION_UNKNOWN &&
       connectionState != ISipConnectionState::CONNECTION_IDLE &&
       connectionState != ISipConnectionState::CONNECTION_DIALING &&
       connectionState != ISipConnectionState::CONNECTION_NEWCALL)
   {
      // sip dialog must be established (early or confirmed)
      // no SDP negotiation must be taking place
      SipDialog::DialogState dialogState;
      {
         OsReadLock lock(m_rStateContext);
         dialogState = m_rStateContext.m_sipDialog.getDialogState();
      }
      CpSipTransactionManager::InviteTransactionState inviteState = getInviteTransactionState();
      CpSdpNegotiation::SdpNegotiationState sdpNegotiationState = m_rStateContext.m_sdpNegotiation.getNegotiationState();

      // UPDATE is supported only if there is no re-INVITE transaction, no SDP negotiation in progress (must be completed),
      // no other outbound update
      if (dialogState == SipDialog::DIALOG_STATE_ESTABLISHED &&
          inviteState != CpSipTransactionManager::REINVITE_ACTIVE &&
          !isOutboundUpdateActive() &&
          sdpNegotiationState != CpSdpNegotiation::SDP_NEGOTIATION_IN_PROGRESS)
      {
         const SdpBody* pSdpBody = sipMessage.getSdpBody(m_rStateContext.m_pSecurity);
         if (pSdpBody)
         {
            if (handleSdpOffer(sipMessage))
            {
               sipResponse.setOkResponseData(&sipMessage, getLocalContactUrl());
               if (prepareSdpAnswer(sipResponse))
               {
                  if (commitMediaSessionChanges())
                  {
                     bProtocolError = FALSE;
                  }
               }
            }
         }
         else
         {
            // UPDATE without SDP body, maybe just refreshing contact or something else
            bProtocolError = FALSE;
         }
      }
   }
   else if (connectionState == ISipConnectionState::CONNECTION_DISCONNECTED ||
            connectionState == ISipConnectionState::CONNECTION_UNKNOWN)
   {
      errorResponseType = ERROR_RESPONSE_481;
   }

   if (bProtocolError)
   {
      SipMessage errorResponse;
      prepareErrorResponse(sipMessage, errorResponse, errorResponseType);
      sendMessage(errorResponse);
   }
   else
   {
      prepareSessionTimerResponse(sipMessage, sipResponse); // construct session timer response
      handleSessionTimerResponse(sipResponse); // handle our own response, start session timer..
      sendMessage(sipResponse);

      // 200 OK was sent, update connected identity
      if (sipMessage.isInSupportedField(SIP_FROM_CHANGE_EXTENSION))
      {
         updateSipDialogRemoteField(sipMessage);
      }
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::processAckRequest(const SipMessage& sipMessage)
{
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState != ISipConnectionState::CONNECTION_DISCONNECTED &&
      connectionState != ISipConnectionState::CONNECTION_UNKNOWN)
   {
      UtlBoolean bProtocolError = FALSE;
      int cseqNumber = 0;
      UtlString cseqMethod;
      sipMessage.getCSeqField(cseqNumber, cseqMethod);

      if (m_rStateContext.m_pLastSent2xxToInvite)
      {
         int cseqNumber2xx = 0;
         m_rStateContext.m_pLastSent2xxToInvite->getCSeqField(&cseqNumber2xx, NULL);

         if (cseqNumber2xx == cseqNumber)
         {
            m_rStateContext.m_bAckReceived = TRUE;
            delete2xxRetransmitTimer();

            const SdpBody* pSdpBody = sipMessage.getSdpBody(m_rStateContext.m_pSecurity);
            if (pSdpBody)
            {
               if (m_rStateContext.m_sdpNegotiation.isSdpOfferFinished())
               {
                  // this must be SDP answer
                  if (handleSdpAnswer(sipMessage))
                  {
                     commitMediaSessionChanges();
                  }
                  else
                  {
                     bProtocolError = TRUE;
                  }
               }
               else
               {
                  bProtocolError = TRUE;
               }
            }
            else
            {
               // sdp bodyContent is missing, check if SDP negotiation is complete. Media session changes are already committed
               if (m_rStateContext.m_sdpNegotiation.getNegotiationState() == CpSdpNegotiation::SDP_NEGOTIATION_IN_PROGRESS)
               {
                  bProtocolError = TRUE;
               }
            }

            // inbound INVITE terminates, check if we need to update connected identity
            if (m_rStateContext.m_connectedIdentityState == SipConnectionStateContext::IDENTITY_NOT_YET_ANNOUNCED &&
               isExtensionSupported(SIP_FROM_CHANGE_EXTENSION))
            {
               m_rStateContext.m_connectedIdentityState = SipConnectionStateContext::IDENTITY_ANNOUNCING;
               refreshSession(FALSE); // refresh session by re-INVITE or UPDATE, to announce real identity
            }

         } // ignore bad acks
      } // ignore bad acks

      if (bProtocolError)
      {
         sendBye(487, "Request terminated due to protocol error");
      }
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::processByeRequest(const SipMessage& sipMessage)
{
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState == ISipConnectionState::CONNECTION_DISCONNECTED ||
      connectionState == ISipConnectionState::CONNECTION_UNKNOWN)
   {
      // transaction was not found
      SipMessage sipResponse;
      sipResponse.setBadTransactionData(&sipMessage);
      sendMessage(sipResponse);
      // we are already disconnected
   }
   else
   {
      // default handler for inbound BYE. We only allow BYE in established state.
      SipMessage sipResponse;
      // bad request, BYE is handled elsewhere
      sipResponse.setRequestBadRequest(&sipMessage);
      sendMessage(sipResponse);
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::processCancelRequest(const SipMessage& sipMessage)
{
   // inbound CANCEL
   int iSeqNumber = 0;
   sipMessage.getCSeqField(&iSeqNumber, NULL); // don't use seqMethod, as it is pretty useless for us (is CANCEL)
   SipMessage sipResponse;
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState != ISipConnectionState::CONNECTION_DISCONNECTED &&
      connectionState != ISipConnectionState::CONNECTION_UNKNOWN)
   {
      CpSipTransactionManager::TransactionState transactionState = getServerTransactionManager().getTransactionState(iSeqNumber);
      if (transactionState == CpSipTransactionManager::TRANSACTION_ACTIVE)
      {
         if (getServerTransactionManager().isInviteTransaction(iSeqNumber))
         {
            // send 200 OK
            sipResponse.setOkResponseData(&sipMessage);
            sendMessage(sipResponse);

            if (!m_rStateContext.m_pLastSent2xxToInvite && m_rStateContext.m_pLastReceivedInvite)
            {
               // 2xx was not yet sent, we can terminate INVITE transaction
               CpSipTransactionManager::InviteTransactionState inviteState = getServerTransactionManager().getInviteTransactionState();

               SipMessage sipInviteResponse;
               sipInviteResponse.setRequestTerminatedResponseData(m_rStateContext.m_pLastReceivedInvite);
               sendMessage(sipInviteResponse); // terminates INVITE transaction

               if (inviteState == CpSipTransactionManager::INITIAL_INVITE_ACTIVE)
               {
                  UtlString protocol;
                  int cause = 0;
                  UtlString text;
                  sipResponse.getReasonField(0, protocol, cause, text); // extract 1st reason field
                  // if we terminated initial INVITE transaction, we also need to disconnect. Otherwise no problem.
                  SipResponseTransitionMemory memory(cause, text);
                  return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, &memory);
               }
            }
         }
         else
         {
            // send 200 OK, but not possible to terminate. Has no effect, as we always respond immediately.
            sipResponse.setOkResponseData(&sipMessage, getLocalContactUrl());
            sendMessage(sipResponse);
         }
      }
      else if (transactionState == CpSipTransactionManager::TRANSACTION_TERMINATED)
      {
         // send 200 OK. Has no effect.
         sipResponse.setOkResponseData(&sipMessage, getLocalContactUrl());
         sendMessage(sipResponse);
      }
      else
      {
         // transaction was not found
         sipResponse.setBadTransactionData(&sipMessage);
         sendMessage(sipResponse);
         // drop connection
         OsStatus result;
         return dropConnection(result);
      }
   }
   else
   {
      // transaction was not found
      sipResponse.setBadTransactionData(&sipMessage);
      sendMessage(sipResponse);
      // we are already disconnected
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::processInfoRequest(const SipMessage& sipMessage)
{
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState != ISipConnectionState::CONNECTION_DISCONNECTED &&
      connectionState != ISipConnectionState::CONNECTION_UNKNOWN)
   {
      if (ms_iInfoTestResponseCode != 0) // used in unit tests
      {
         if (ms_iInfoTestResponseCode == SIP_REQUEST_TIMEOUT_CODE)
         {
            OsTask::delay(1000);
         }
         SipMessage sipResponse;
         sipResponse.setResponseData(&sipMessage, ms_iInfoTestResponseCode, "Timed out", getLocalContactUrl());
         sendMessage(sipResponse);
         return NULL; // do not fire event if custom response code was requested.
      }
      else
      {
         // send 200 OK response regardless of content
         SipMessage sipResponse;
         sipResponse.setOkResponseData(&sipMessage, getLocalContactUrl());
         sendMessage(sipResponse);
      }

      // we won't see duplicate INFO request here, SipUserAgent filters them
      UtlString contentType;
      sipMessage.getContentType(&contentType);
      UtlString bodyContent;
      int contentLength = sipMessage.getContentLength();
      const HttpBody* pBody = sipMessage.getBody();
      if (pBody)
      {
         pBody->getBytes(&bodyContent, &contentLength);
      }
      else
      {
         contentLength = 0;
      }

      m_rSipConnectionEventSink.fireSipXInfoEvent(contentType, bodyContent.data(), contentLength);
   }
   else
   {
      // respond with 481
      SipMessage sipResponse;
      sipResponse.setBadTransactionData(&sipMessage);
      sendMessage(sipResponse);
   }
   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::processNotifyRequest(const SipMessage& sipRequest)
{
   ISipConnectionState::StateEnum connectionState = getCurrentState();
   UtlBoolean bSendError = TRUE;
   SipMessage errorResponse;
   ERROR_RESPONSE_TYPE errorResponseType = ERROR_RESPONSE_488;

   if (connectionState == ISipConnectionState::CONNECTION_ESTABLISHED)
   {
      if (m_rStateContext.m_localEntityType == SipConnectionStateContext::ENTITY_TRANSFER_CONTROLLER)
      {
         // get additional details about NOTIFY content
         UtlString eventType;
         UtlString eventId;
         sipRequest.getEventField(&eventType, &eventId);

         if (eventType.compareTo(SIP_EVENT_REFER) == 0)
         {
            const HttpBody* pBody = sipRequest.getBody();
            UtlString contentType;
            sipRequest.getContentType(&contentType);
            // maybe deactivate subscription
            UtlString state;
            UtlString reason;
            int expireInSeconds;
            int retryAfter;
            sipRequest.getSubscriptionState(state, reason, expireInSeconds, retryAfter);
            if (state.compareTo(SIP_SUBSCRIPTION_TERMINATED) == 0 || expireInSeconds == 0)
            {
               m_rStateContext.m_referInSubscriptionActive = FALSE; // subscription has been terminated
            }
            else
            {
               m_rStateContext.m_referInSubscriptionActive = TRUE; // mark subscription as active, we will need to unsubscribe
            }

            if (pBody && contentType.compareTo(CONTENT_TYPE_MESSAGE_SIPFRAG) == 0)
            {
               SipMessage sipResponse;
               sipResponse.setResponseData(&sipRequest, SIP_OK_CODE, SIP_OK_TEXT, getLocalContactUrl());
               sendMessage(sipResponse);

               // we only like message/sipfrag content
               m_rStateContext.m_subscriptionId = eventId;
               // extract sipfrag body
               const char* bodyBytes; // copy of pointer to internal body
               int bodyLength;
               pBody->getBytes(&bodyBytes, &bodyLength);
               // construct sip message from body
               SipMessage notifyBody(bodyBytes, bodyLength);
               // handle inner message
               return handleReferNotifyBody(notifyBody);
            }
         }
      }
   }
   else if (connectionState == ISipConnectionState::CONNECTION_DISCONNECTED ||
         connectionState == ISipConnectionState::CONNECTION_UNKNOWN)
   {
      errorResponseType = ERROR_RESPONSE_481;
   }

   if (bSendError)
   {
      prepareErrorResponse(sipRequest, errorResponse, errorResponseType);
      sendMessage(errorResponse);
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::processReferRequest(const SipMessage& sipRequest)
{
   ISipConnectionState::StateEnum connectionState = getCurrentState();
   UtlBoolean bSendError = TRUE;
   SipMessage errorResponse;
   ERROR_RESPONSE_TYPE errorResponseType = ERROR_RESPONSE_603;

   if (connectionState == ISipConnectionState::CONNECTION_ESTABLISHED)
   {
      // retransmits are filtered out by SipUserAgent, we only get new requests
      // SipUserAgent will resend last response if available, if not then does nothing
      // but we won't see duplicate request in any case
      if (m_rStateContext.m_localEntityType == SipConnectionStateContext::ENTITY_NORMAL)
      {
         // verify the REFER
         if(sipRequest.getHeaderValue(1, SIP_REFERRED_BY_FIELD) ||
            sipRequest.getHeaderValue(1, SIP_REFER_TO_FIELD))
         {
            errorResponseType = ERROR_RESPONSE_400;
         }
         else
         {
            setLastReceivedRefer(sipRequest); // remember received REFER
            m_rStateContext.m_localEntityType = SipConnectionStateContext::ENTITY_TRANSFEREE;
            m_rStateContext.m_inboundReferResponse = SipConnectionStateContext::REFER_NO_RESPONSE;
            // don't send response, wait for user to either accept or reject refer
            UtlString referredBy;
            UtlString referTo;
            sipRequest.getReferredByField(referredBy);
            sipRequest.getReferToField(referTo);
            // fire transfer event, so that user can confirm or reject transfer
            m_rSipConnectionEventSink.fireSipXCallEvent(CP_CALLSTATE_TRANSFER_EVENT, CP_CALLSTATE_CAUSE_TRANSFER, NULL,
               0, NULL, referredBy, referredBy);
            bSendError = FALSE;
         }
      }
   }
   else if (connectionState == ISipConnectionState::CONNECTION_DISCONNECTED ||
      connectionState == ISipConnectionState::CONNECTION_UNKNOWN)
   {
      errorResponseType = ERROR_RESPONSE_481;
   }

   if (bSendError)
   {
      prepareErrorResponse(sipRequest, errorResponse, errorResponseType);
      sendMessage(errorResponse);
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::handleReferNotifyBody(const SipMessage& sipNotifyBody)
{
   int responseCode = sipNotifyBody.getResponseStatusCode();
   UtlString responseText;
   sipNotifyBody.getResponseStatusText(&responseText);
   UtlBoolean bTerminateCall = FALSE;

   if (responseCode == SIP_TRYING_CODE)
   {
      m_rSipConnectionEventSink.fireSipXCallEvent(CP_CALLSTATE_TRANSFER_EVENT, CP_CALLSTATE_CAUSE_TRANSFER_TRYING, NULL, responseCode, responseText);
   }
   else if (responseCode == SIP_RINGING_CODE || responseCode == SIP_SESSION_PROGRESS_CODE)
   {
      m_rSipConnectionEventSink.fireSipXCallEvent(CP_CALLSTATE_TRANSFER_EVENT, CP_CALLSTATE_CAUSE_TRANSFER_RINGING, NULL, responseCode, responseText);
   }
   else if (responseCode >= SIP_2XX_CLASS_CODE && responseCode < SIP_3XX_CLASS_CODE)
   {
      m_rSipConnectionEventSink.fireSipXCallEvent(CP_CALLSTATE_TRANSFER_EVENT, CP_CALLSTATE_CAUSE_TRANSFER_SUCCESS, NULL, responseCode, responseText);
      bTerminateCall = TRUE;
   }
   else if (responseCode >= SIP_3XX_CLASS_CODE)
   {
      // refer redirection should never occur, since we only send REFER in dialog
      m_rSipConnectionEventSink.fireSipXCallEvent(CP_CALLSTATE_TRANSFER_EVENT, CP_CALLSTATE_CAUSE_TRANSFER_FAILURE, NULL, responseCode, responseText);
      // reset to connected state, so that user can continue with this call
      m_rSipConnectionEventSink.fireSipXCallEvent(CP_CALLSTATE_CONNECTED, CP_CALLSTATE_CAUSE_NORMAL);
      m_rStateContext.m_localEntityType = SipConnectionStateContext::ENTITY_NORMAL;
      delete m_rStateContext.m_pLastSentRefer;
      m_rStateContext.m_pLastSentRefer = NULL;
   }

   if (bTerminateCall)
   {
      // call is established, terminate with BYE
      sendBye();
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::processPrackRequest(const SipMessage& sipRequest)
{
   ISipConnectionState::StateEnum connectionState = getCurrentState();
   int rseqNum;
   int cseqNum;
   UtlString rseqMethod;
   sipRequest.getRAckField(rseqNum, cseqNum, rseqMethod);
   UtlBoolean bProtocolError = TRUE;
   ERROR_RESPONSE_TYPE errorResponseType = ERROR_RESPONSE_481;
   SipMessage sipResponse;
   UtlBoolean bTerminateCall = FALSE;
   int inviteSeqNum = -1;

   if (m_rStateContext.m_pLastReceivedInvite)
   {
      m_rStateContext.m_pLastReceivedInvite->getCSeqField(&inviteSeqNum, NULL);
   }

   if (m_rStateContext.m_100RelTracker.onPrackReceived(sipRequest) &&
      m_rStateContext.m_pLastReceivedInvite &&
      inviteSeqNum == cseqNum)
   {
      CpSdpNegotiation::SdpBodyType prackBodyType = getPrackSdpBodyType(sipRequest);

      if (connectionState == ISipConnectionState::CONNECTION_OFFERING || // we never send 100rel response when established
         connectionState == ISipConnectionState::CONNECTION_ALERTING ||
         connectionState == ISipConnectionState::CONNECTION_QUEUED)
      {
         if (getServerTransactionManager().getTransactionState(cseqNum) == CpSipTransactionManager::TRANSACTION_ACTIVE &&
            rseqMethod.compareTo(SIP_INVITE_METHOD) == 0)
         {
            // 200 OK was not sent yet, 
            // PRACK is valid
            sipResponse.setOkResponseData(&sipRequest, getLocalContactUrl());

            if (prackBodyType == CpSdpNegotiation::SDP_BODY_OFFER)
            {
               if (handleSdpOffer(sipRequest))
               {
                  if (prepareSdpAnswer(sipResponse) && commitMediaSessionChanges())
                  {
                     bProtocolError = FALSE;
                  }
               }
            }
            else if (prackBodyType == CpSdpNegotiation::SDP_BODY_ANSWER)
            {
               if (handleSdpAnswer(sipRequest))
               {
                  if (commitMediaSessionChanges())
                  {
                     bProtocolError = FALSE;
                  }
               }
            }
            else if (prackBodyType == CpSdpNegotiation::SDP_BODY_NONE)
            {
               bProtocolError = FALSE;
            }
         }

         if (bProtocolError)
         {
            bTerminateCall = TRUE;
         }
      }
      else
      {
         if (connectionState != ISipConnectionState::CONNECTION_DISCONNECTED &&
            connectionState != ISipConnectionState::CONNECTION_UNKNOWN)
         {
            // PRACK is valid, 200 OK INVITE response was sent
            sipResponse.setOkResponseData(&sipRequest, getLocalContactUrl());

            if (prackBodyType == CpSdpNegotiation::SDP_BODY_OFFER)
            {
               if (handleSdpOffer(sipRequest))
               {
                  if (prepareSdpAnswer(sipResponse))
                  {
                     bProtocolError = FALSE;
                  }
               }
            }
            else if (prackBodyType == CpSdpNegotiation::SDP_BODY_ANSWER)
            {
               if (handleSdpAnswer(sipRequest))
               {
                  bProtocolError = FALSE;
               }
            }
            else if (prackBodyType == CpSdpNegotiation::SDP_BODY_NONE)
            {
               bProtocolError = FALSE;
            }
         }
      }
   }

   if (bProtocolError)
   {
      SipMessage errorResponse;
      prepareErrorResponse(sipRequest, errorResponse, errorResponseType);
      sendMessage(errorResponse);
   }
   else
   {
      sendMessage(sipResponse);
   }

   if (bTerminateCall && m_rStateContext.m_pLastReceivedInvite)
   {
      SipMessage errorResponse;
      prepareErrorResponse(*m_rStateContext.m_pLastReceivedInvite, errorResponse, ERROR_RESPONSE_488);
      sendMessage(errorResponse);

      SipResponseTransitionMemory memory(SIP_REQUEST_NOT_ACCEPTABLE_HERE_CODE, SIP_REQUEST_NOT_ACCEPTABLE_HERE_TEXT);
      return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, &memory);
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::processOptionsRequest(const SipMessage& sipMessage)
{
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState != ISipConnectionState::CONNECTION_DISCONNECTED &&
      connectionState != ISipConnectionState::CONNECTION_UNKNOWN)
   {
      // respond with 200 OK
      SipMessage sipResponse;
      sipResponse.setResponseData(&sipMessage, SIP_OK_CODE, SIP_OK_TEXT);
      sendMessage(sipResponse);
   }
   else
   {
      // respond with 481
      SipMessage sipResponse;
      sipResponse.setBadTransactionData(&sipMessage);
      sendMessage(sipResponse);
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::processSubscribeRequest(const SipMessage& sipMessage)
{
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState != ISipConnectionState::CONNECTION_DISCONNECTED &&
      connectionState != ISipConnectionState::CONNECTION_UNKNOWN)
   {
      UtlString eventField;
      int expires = -1;
      sipMessage.getEventField(eventField);
      sipMessage.getExpiresField(&expires);

      if (m_rStateContext.m_localEntityType == SipConnectionStateContext::ENTITY_TRANSFEREE &&
         m_rStateContext.m_pLastReceivedRefer &&
         eventField.compareTo(SIP_EVENT_REFER) == 0 &&
         expires == 0)
      {
         // allow to unsubscribe from implicit REFER subscription
         SipMessage sipResponse;
         sipResponse.setResponseData(&sipMessage, SIP_OK_CODE, SIP_OK_TEXT, getLocalContactUrl());
         sendMessage(sipResponse);
         m_rStateContext.m_referOutSubscriptionActive = FALSE;
      }
      else
      {
         // decline in dialog SUBSCRIBE, we do not support multiple dialog usages, except for REFER
         SipMessage sipResponse;
         sipResponse.setResponseData(&sipMessage, SIP_SERVICE_UNAVAILABLE_CODE, SIP_SERVICE_UNAVAILABLE_TEXT);
         sendMessage(sipResponse);
      }
   }
   else
   {
      // respond with 481
      SipMessage sipResponse;
      sipResponse.setBadTransactionData(&sipMessage);
      sendMessage(sipResponse);
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::processResponse(const SipMessage& sipMessage)
{
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   // INVITE transaction termination is handled during disconnected state entry
   if (connectionState != ISipConnectionState::CONNECTION_DISCONNECTED &&
       connectionState != ISipConnectionState::CONNECTION_UNKNOWN)
   {
      int iSeqNumber = 0;
      UtlString seqMethod;
      sipMessage.getCSeqField(iSeqNumber, seqMethod);
      int statusCode = sipMessage.getResponseStatusCode();
      UtlString statusText;
      sipMessage.getResponseStatusText(&statusText);

      CpSipTransactionManager::TransactionState transactionState = getClientTransactionManager().getTransactionState(seqMethod, iSeqNumber);
      // allow only responses for which we have an active transaction, or CANCEL response (don't have transaction tracking)
      // or INVITE 200 OK retransmissions (transaction will be terminated)
      if (transactionState == CpSipTransactionManager::TRANSACTION_ACTIVE ||
          seqMethod.compareTo(SIP_CANCEL_METHOD) == 0 ||
          (seqMethod.compareTo(SIP_INVITE_METHOD) == 0 && transactionState == CpSipTransactionManager::TRANSACTION_TERMINATED &&
           statusCode >= SIP_2XX_CLASS_CODE && statusCode < SIP_3XX_CLASS_CODE))
      {
         updateRemoteCapabilities(sipMessage);

         switch (statusCode)
         {
         // these destroy the whole dialog regardless of seqMethod
         case SIP_NOT_FOUND_CODE: // 404
         case SIP_GONE_CODE: // 410
         case SIP_UNSUPPORTED_URI_SCHEME_CODE: // 416
         case SIP_LOOP_DETECTED_CODE: // 482
         case SIP_TOO_MANY_HOPS_CODE: // 483
         case SIP_BAD_ADDRESS_CODE: // 484
         case SIP_AMBIGUOUS_CODE: // 485
         case SIP_BAD_GATEWAY_CODE: // 502
         case SIP_DOESNT_EXIST_ANYWHERE_CODE: // 604
            {
               trackTransactionResponse(sipMessage);
               SipResponseTransitionMemory memory(statusCode, statusText);
               return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, &memory);
            }
         // these destroy dialog usage - if message belongs to INVITE usage, destroy dialog
         case SIP_BAD_METHOD_CODE: // 405
         case SIP_TEMPORARILY_UNAVAILABLE_CODE: // 480
         case SIP_BAD_TRANSACTION_CODE: // 481
         case SIP_BAD_EVENT_CODE: // 489
         case SIP_UNIMPLEMENTED_METHOD_CODE: // 501
            if (sipMessage.isInviteDialogUsage())
            {
               trackTransactionResponse(sipMessage);
               SipResponseTransitionMemory memory(statusCode, statusText);
               return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, &memory);
            }
            break;
         case SIP_UNAUTHORIZED_CODE: // 401
         case SIP_PROXY_AUTH_REQUIRED_CODE: // 407
            {
               if (seqMethod.compareTo(SIP_BYE_METHOD) == 0 ||
                   seqMethod.compareTo(SIP_CANCEL_METHOD) == 0)
               {
                  trackTransactionResponse(sipMessage);
                  GeneralTransitionMemory memory(CP_CALLSTATE_CAUSE_NO_RESPONSE, statusCode, statusText);
                  return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, &memory);
               }
            }
         case SIP_REQUEST_TIMEOUT_CODE: // 408
            {
               if (seqMethod.compareTo(SIP_INVITE_METHOD) == 0 ||
                   seqMethod.compareTo(SIP_UPDATE_METHOD) == 0 ||
                   seqMethod.compareTo(SIP_BYE_METHOD) == 0 ||
                   seqMethod.compareTo(SIP_CANCEL_METHOD) == 0)
               {
                  trackTransactionResponse(sipMessage);
                  GeneralTransitionMemory memory(CP_CALLSTATE_CAUSE_NO_RESPONSE, statusCode, statusText);
                  return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, &memory);
               }
            }
            break;
         // these only affect transaction - if seqMethod was INVITE and media session doesn't exist, then destroy dialog
         // if media session exists, then no need to destroy the dialog - only INVITE failed for some reason, we can keep
         // the original media session
         default:
            // handle unknown 4XX, 5XX, 6XX for initial INVITE transaction (but not re-INVITE)
            CpSipTransactionManager::InviteTransactionState inviteState = getClientTransactionManager().getInviteTransactionState();
            if (statusCode >= SIP_4XX_CLASS_CODE && statusCode < SIP_7XX_CLASS_CODE &&
                statusCode != SIP_SMALL_SESSION_INTERVAL_CODE && // 422 means we can still retry
                seqMethod.compareTo(SIP_INVITE_METHOD) == 0 &&
                inviteState == CpSipTransactionManager::INITIAL_INVITE_ACTIVE)
            {
               trackTransactionResponse(sipMessage);
               if (connectionState == ISipConnectionState::CONNECTION_REMOTE_OFFERING && m_rStateContext.m_bRedirecting)
               {
                  // we are redirecting and redirect failed, try next one
                  return followNextRedirect();
               }
               else
               {
                  // failure during initial INVITE, terminate dialog
                  SipResponseTransitionMemory memory(statusCode, statusText);
                  return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, &memory);
               }
            }
            // otherwise re-INVITE failed
            ;
         }

         SipConnectionStateTransition* pTransition = processNonFatalResponse(sipMessage);
         trackTransactionResponse(sipMessage); // automatically ends transaction, after we process response
         return pTransition;
      }
      // if transaction was not found or is terminated then ignore response
   }

   trackTransactionResponse(sipMessage);
   return NULL; // no transition
}

SipConnectionStateTransition* BaseSipConnectionState::processNonFatalResponse(const SipMessage& sipMessage)
{
   // process inbound sip message request
   int seqNum;
   UtlString seqMethod;
   sipMessage.getCSeqField(seqNum, seqMethod);

   if (seqMethod.compareTo(SIP_INVITE_METHOD) == 0)
   {
      return processInviteResponse(sipMessage);
   }
   else if (seqMethod.compareTo(SIP_UPDATE_METHOD) == 0)
   {
      return processUpdateResponse(sipMessage);
   }
   else if (seqMethod.compareTo(SIP_BYE_METHOD) == 0)
   {
      return processByeResponse(sipMessage);
   }
   else if (seqMethod.compareTo(SIP_CANCEL_METHOD) == 0)
   {
      return processCancelResponse(sipMessage);
   }
   else if (seqMethod.compareTo(SIP_INFO_METHOD) == 0)
   {
      return processInfoResponse(sipMessage);
   }
   else if (seqMethod.compareTo(SIP_NOTIFY_METHOD) == 0)
   {
      return processNotifyResponse(sipMessage);
   }
   else if (seqMethod.compareTo(SIP_OPTIONS_METHOD) == 0)
   {
      return processOptionsResponse(sipMessage);
   }
   else if (seqMethod.compareTo(SIP_REFER_METHOD) == 0)
   {
      return processReferResponse(sipMessage);
   }
   else if (seqMethod.compareTo(SIP_SUBSCRIBE_METHOD) == 0)
   {
      return processSubscribeResponse(sipMessage);
   }
   else if (seqMethod.compareTo(SIP_PRACK_METHOD) == 0)
   {
      return processPrackResponse(sipMessage);
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::processInviteResponse(const SipMessage& sipMessage)
{
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   // INVITE transaction termination is handled during disconnected state entry
   if (connectionState != ISipConnectionState::CONNECTION_DISCONNECTED &&
      connectionState != ISipConnectionState::CONNECTION_UNKNOWN)
   {
      int responseCode = sipMessage.getResponseStatusCode();
      
      if (responseCode > SIP_1XX_CLASS_CODE)
      {
         deleteInviteExpirationTimer(); // INVITE was answered
      }
      // handle connected identity response
      if (m_rStateContext.m_connectedIdentityState == SipConnectionStateContext::IDENTITY_ANNOUNCING)
      {
         if (responseCode >= SIP_2XX_CLASS_CODE && responseCode < SIP_3XX_CLASS_CODE)
         {
            onConnectedIdentityAccepted();
         }
         else if (responseCode >= SIP_4XX_CLASS_CODE &&
            responseCode != SIP_SMALL_SESSION_INTERVAL_CODE &&
            responseCode != SIP_REQUEST_PENDING_CODE)
         {
            onConnectedIdentityRejected();
         }
      }

      // process 422 for both initial INVITE and re-INVITE
      if (responseCode == SIP_SMALL_SESSION_INTERVAL_CODE)
      {
         handleSmallSessionIntervalResponse(sipMessage); // session timeout is too small
      }
      else if (responseCode >= SIP_2XX_CLASS_CODE && responseCode < SIP_3XX_CLASS_CODE)
      {
         handleSessionTimerResponse(sipMessage);
      }
      else if (responseCode == SIP_REQUEST_PENDING_CODE &&
               connectionState == ISipConnectionState::CONNECTION_ESTABLISHED)
      {
         // we got 491 for outbound re-INVITE/UPDATE, retry it after some time, if timer is not already running
         if (!m_rStateContext.m_pSessionRenegotiationTimer &&
             m_rStateContext.m_491failureCounter < MAX_491_FAILURES)
         {
            m_rStateContext.m_491failureCounter++;
            startSessionRenegotiationTimer(ScReInviteTimerMsg::REASON_NORMAL, TRUE);
         }
      }
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::processProvisionalInviteResponse(const SipMessage& sipResponse)
{
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState == ISipConnectionState::CONNECTION_REMOTE_ALERTING ||
      connectionState == ISipConnectionState::CONNECTION_REMOTE_OFFERING ||
      connectionState == ISipConnectionState::CONNECTION_ESTABLISHED ||
      connectionState == ISipConnectionState::CONNECTION_REMOTE_QUEUED)
   {
      int responseCode = sipResponse.getResponseStatusCode();
      UtlString responseText;
      sipResponse.getResponseStatusText(&responseText);
      int cseqNum;
      UtlString cseqMethod;
      sipResponse.getCSeqField(&cseqNum, &cseqMethod);
      UtlBoolean bSendPrack = FALSE;
      UtlBoolean bProtocolError = FALSE;
      UtlBoolean bGenerateSdpAnswer = FALSE;

      if (sipResponse.is100RelResponse())
      {
         bSendPrack = m_rStateContext.m_100RelTracker.on100RelReceived(sipResponse);
      }

      const SdpBody* pSdpBody = sipResponse.getSdpBody(m_rStateContext.m_pSecurity);
      if (pSdpBody)
      {
         // 18x response with SDP body
         if (sipResponse.is100RelResponse())
         {
            // reliable 18x response
            if (bSendPrack)
            {
               if (m_rStateContext.m_sdpNegotiation.getNegotiationState() == CpSdpNegotiation::SDP_NEGOTIATION_IN_PROGRESS)
               {
                  // this must be SDP answer
                  if (handleSdpAnswer(sipResponse))
                  {
                     bProtocolError = !commitMediaSessionChanges();
                  }
                  else
                  {
                     bProtocolError = TRUE; // error while handling Sdp answer
                  }
               }
               else
               {
                  // this must be SDP offer
                  if (handleSdpOffer(sipResponse))
                  {
                     bGenerateSdpAnswer = TRUE;
                  }
                  else
                  {
                     bProtocolError = TRUE; // error while handling Sdp offer
                  }
               }
            } // else 18x retransmit, but we sent PRACK, ignore as PRACK will be resent automatically by sipXtackLib
         }
         else
         {
            // unreliable 18x response
            if (m_rStateContext.m_sdpNegotiation.getNegotiationState() == CpSdpNegotiation::SDP_NEGOTIATION_IN_PROGRESS)
            {
               // this must be SDP answer
               if (handleSdpAnswer(sipResponse))
               {
                  bProtocolError = !commitMediaSessionChanges();
               }
               else
               {
                  bProtocolError = TRUE; // error while handling Sdp answer
               }
            }
         }
      }

      if (bSendPrack)
      {
         bProtocolError |= !sendPrack(sipResponse, bGenerateSdpAnswer);
      } // else 18x retransmission

      if (bProtocolError)
      {
         // some protocol error (SDP), cancel call
         if (connectionState != ISipConnectionState::CONNECTION_ESTABLISHED)
         {
            sendInviteCancel(487, "Request terminated due to protocol error");
         }
         else
         {
            sendBye(487, "Request terminated due to protocol error");
         }
      }

      if (connectionState != ISipConnectionState::CONNECTION_ESTABLISHED)
      {
         if (responseCode == SIP_RINGING_CODE || responseCode == SIP_SESSION_PROGRESS_CODE)
         {
            // proceed to remote alerting state
            SipResponseTransitionMemory memory(responseCode, responseText);
            return getTransition(ISipConnectionState::CONNECTION_REMOTE_ALERTING, &memory);
         }
         else if (responseCode == SIP_QUEUED_CODE)
         {
            // proceed to remote queued state
            SipResponseTransitionMemory memory(responseCode, responseText);
            return getTransition(ISipConnectionState::CONNECTION_REMOTE_QUEUED, &memory);
         }
      }
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::processUpdateResponse(const SipMessage& sipResponse)
{
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   int responseCode = sipResponse.getResponseStatusCode();

   if (connectionState != ISipConnectionState::CONNECTION_DISCONNECTED &&
       connectionState != ISipConnectionState::CONNECTION_UNKNOWN &&
       connectionState != ISipConnectionState::CONNECTION_IDLE &&
       connectionState != ISipConnectionState::CONNECTION_DIALING &&
       connectionState != ISipConnectionState::CONNECTION_NEWCALL)
   {
      if (m_rStateContext.m_connectedIdentityState == SipConnectionStateContext::IDENTITY_ANNOUNCING)
      {
         if (responseCode >= SIP_2XX_CLASS_CODE && responseCode < SIP_3XX_CLASS_CODE)
         {
            onConnectedIdentityAccepted();
         }
         else if (responseCode >= SIP_4XX_CLASS_CODE &&
                  responseCode != SIP_SMALL_SESSION_INTERVAL_CODE &&
                  responseCode != SIP_REQUEST_PENDING_CODE)
         {
            onConnectedIdentityRejected();
         }
      }

      if (m_rStateContext.m_sdpNegotiation.isInSdpNegotiation(sipResponse))
      {
         if (responseCode >= SIP_2XX_CLASS_CODE && responseCode < SIP_3XX_CLASS_CODE)
         {
            m_rStateContext.m_491failureCounter = 0;
            handleSessionTimerResponse(sipResponse); // handle response, start session timer..

            // UPDATE transaction exists (checked in generic response handler), and we are in correct state
            const SdpBody* pSdpBody = sipResponse.getSdpBody(m_rStateContext.m_pSecurity);
            if (pSdpBody)
            {
               if (handleSdpAnswer(sipResponse))
               {
                  commitMediaSessionChanges();
               }
            }
            else if (m_rStateContext.m_sdpNegotiation.getNegotiationState() == CpSdpNegotiation::SDP_NEGOTIATION_IN_PROGRESS)
            {
               // we sent SDP offer, but didn't get SDP answer, this is not fatal error
               OsSysLog::add(FAC_CP, PRI_WARNING, "Missing SDP answer: SDP answer was not found as expected in UPDATE response. Call-id: %s.",
                  getCallId().data());
               m_rStateContext.m_sdpNegotiation.resetSdpNegotiation();
            }
         } // if error then nothing happens
         else if (responseCode == SIP_SMALL_SESSION_INTERVAL_CODE)
         {
            handleSmallSessionIntervalResponse(sipResponse);
         }
         else if (responseCode == SIP_REQUEST_PENDING_CODE &&
                  connectionState == ISipConnectionState::CONNECTION_ESTABLISHED)
         {
            // we got 491 for outbound re-INVITE/UPDATE, retry it after some time, if timer is not already running
            if (!m_rStateContext.m_pSessionRenegotiationTimer &&
                m_rStateContext.m_491failureCounter < MAX_491_FAILURES)
            {
               m_rStateContext.m_491failureCounter++;
               startSessionRenegotiationTimer(ScReInviteTimerMsg::REASON_NORMAL, TRUE);
            }
         }
      }
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::processByeResponse(const SipMessage& sipMessage)
{
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   // BYE is only valid if connection is established
   if (connectionState == ISipConnectionState::CONNECTION_ESTABLISHED)
   {
      int responseCode = sipMessage.getResponseStatusCode();
      UtlString responseText;
      sipMessage.getResponseStatusText(&responseText);

      if (responseCode == SIP_OK_CODE)
      {
         // progress to disconnected state
         SipResponseTransitionMemory memory(responseCode, responseText);
         return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, &memory);
      }
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::processCancelResponse(const SipMessage& sipMessage)
{
   // we don't really care if CANCEL request response is 200 OK or not
   // 481 and other fatal responses were already handled
   // if CANCEL succeeds, 487 Request Terminated will be sent on INVITE transaction (with cseq method INVITE)
   // and then we terminate INVITE transaction

   // OBS: We do care - some servers (SipXecs) does not send 487 if we are in remote offering
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   int responseCode = sipMessage.getResponseStatusCode();
   UtlString responseText;
   sipMessage.getResponseStatusText(&responseText);

   if (responseCode == SIP_OK_CODE)
   {
      // progress to disconnected state
      SipResponseTransitionMemory memory(responseCode, responseText);
      return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, &memory);
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::processInfoResponse(const SipMessage& sipMessage)
{
   int responseCode = sipMessage.getResponseStatusCode();
   UtlString responseText;
   int cseqNum;
   UtlString cseqMethod;

   sipMessage.getResponseStatusText(&responseText);
   sipMessage.getCSeqField(&cseqNum, &cseqMethod);

   void* pCookie = getClientTransactionManager().getTransactionData(cseqMethod, cseqNum);
   // ignore 100 Trying
   if (responseCode >= SIP_2XX_CLASS_CODE && responseCode < SIP_3XX_CLASS_CODE)
   {
      m_rSipConnectionEventSink.fireSipXInfoStatusEvent(CP_INFOSTATUS_RESPONSE, SIPXTACK_MESSAGE_OK, responseText, responseCode, pCookie);
   }
   else if (responseCode >= SIP_3XX_CLASS_CODE && responseCode < SIP_5XX_CLASS_CODE)
   {
      m_rSipConnectionEventSink.fireSipXInfoStatusEvent(CP_INFOSTATUS_RESPONSE, SIPXTACK_MESSAGE_FAILURE, responseText, responseCode, pCookie);
   }
   else if (responseCode >= SIP_5XX_CLASS_CODE && responseCode < SIP_6XX_CLASS_CODE)
   {
      m_rSipConnectionEventSink.fireSipXInfoStatusEvent(CP_INFOSTATUS_RESPONSE, SIPXTACK_MESSAGE_SERVER_FAILURE, responseText, responseCode, pCookie);
   }
   else if (responseCode >= SIP_6XX_CLASS_CODE)
   {
      m_rSipConnectionEventSink.fireSipXInfoStatusEvent(CP_INFOSTATUS_RESPONSE, SIPXTACK_MESSAGE_GLOBAL_FAILURE, responseText, responseCode, pCookie);
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::processOptionsResponse(const SipMessage& sipMessage)
{
   // OPTIONS responses are handled in updateRemoteCapabilities
   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::processNotifyResponse(const SipMessage& sipMessage)
{
   // TODO: Implement
   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::processReferResponse(const SipMessage& sipMessage)
{
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState == ISipConnectionState::CONNECTION_ESTABLISHED)
   {
      int responseCode = sipMessage.getResponseStatusCode();
      UtlString responseText;
      sipMessage.getResponseStatusText(&responseText);

      if (responseCode >= SIP_2XX_CLASS_CODE && responseCode < SIP_3XX_CLASS_CODE)
      {
         // refer success, further events will be fired when we get NOTIFYs
         // this call will be terminated when we get final NOTIFY, local user drops the call, or we get BYE
         m_rSipConnectionEventSink.fireSipXCallEvent(CP_CALLSTATE_TRANSFER_EVENT, CP_CALLSTATE_CAUSE_TRANSFER_ACCEPTED, NULL, responseCode, responseText);
      }
      else if (responseCode >= SIP_3XX_CLASS_CODE)
      {
         // refer failure
         m_rStateContext.m_localEntityType = SipConnectionStateContext::ENTITY_NORMAL;
         m_rStateContext.m_referInSubscriptionActive = FALSE;
         delete m_rStateContext.m_pLastSentRefer;
         m_rStateContext.m_pLastSentRefer = NULL;
         m_rSipConnectionEventSink.fireSipXCallEvent(CP_CALLSTATE_TRANSFER_EVENT, CP_CALLSTATE_CAUSE_TRANSFER_FAILURE, NULL, responseCode, responseText);
         // fire connected event, we keep control of the call
         m_rSipConnectionEventSink.fireSipXCallEvent(CP_CALLSTATE_CONNECTED, CP_CALLSTATE_CAUSE_NORMAL);
      }
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::processPrackResponse(const SipMessage& sipMessage)
{
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState != ISipConnectionState::CONNECTION_DISCONNECTED &&
      connectionState != ISipConnectionState::CONNECTION_UNKNOWN)
   {
      int responseCode = sipMessage.getResponseStatusCode();

      if (responseCode >= SIP_2XX_CLASS_CODE && responseCode < SIP_3XX_CLASS_CODE)
      {
         // nothing to do. SDP answer cannot be present in PRACK response, because we never send SDP offer
         // in PRACK request. SDP offer can't be present in PRACK response either.
      }
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::processSubscribeResponse(const SipMessage& sipMessage)
{
   // TODO: implement for REFER
   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::handleAuthenticationRetryEvent(const SipMessage& sipMessage)
{

   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState != ISipConnectionState::CONNECTION_DISCONNECTED &&
      connectionState != ISipConnectionState::CONNECTION_UNKNOWN)
   {
      if (sipMessage.isResponse())
      {
         int oldSeqNum;
         UtlString seqMethod;
         sipMessage.getCSeqField(oldSeqNum, seqMethod);
         int newSeqNum = oldSeqNum + 1;

         if (needsTransactionTracking(seqMethod))
         {
            // start next transaction for authentication retry request we never see here
            if (seqMethod.compareTo(SIP_INVITE_METHOD) == 0)
            {
               getClientTransactionManager().updateInviteTransaction(newSeqNum);
               if (m_rStateContext.m_pInviteExpiresTimer)
               {
                  // restart INVITE expiration check timer
                  startInviteExpirationTimer(m_rStateContext.m_inviteExpiresSeconds, newSeqNum, TRUE);
               }
            }
            else
            {
               // non INVITE transaction
               void* pToken = getClientTransactionManager().getTransactionData(seqMethod, oldSeqNum);
               getClientTransactionManager().endTransaction(oldSeqNum); // end current transaction
               // SipUserAgent uses seqNum+1 for authentication retransmission. That works only
               // if we don't send 2 messages quickly that both need to be authenticated
               getNextLocalCSeq(); // skip 1 cseq number
               UtlBoolean res = getClientTransactionManager().startTransaction(seqMethod, newSeqNum); // start new transaction with the same method
               if (res)
               {
                  getClientTransactionManager().setTransactionData(seqMethod, newSeqNum, pToken);
               }
               else
               {
                  OsSysLog::add(FAC_CP, PRI_WARNING, "Starting transaction after auth retry failed for call-id %s. seqMethod=%s,seqNum=%i.",
                     getCallId().data(), seqMethod.data(), newSeqNum);
               }
            }

            // also update sdp negotiation for INVITE & UPDATE
            if (seqMethod.compareTo(SIP_INVITE_METHOD) == 0 ||
                seqMethod.compareTo(SIP_UPDATE_METHOD) == 0)
            {
               if (m_rStateContext.m_sdpNegotiation.isInSdpNegotiation(sipMessage))
               {
                  // original message was in SDP negotiation
                  m_rStateContext.m_sdpNegotiation.notifyAuthRetry(newSeqNum);
               }
            }
         }
      }
   }

   // no state transition
   return NULL;
}

UtlBoolean BaseSipConnectionState::isMethodAllowed(const UtlString& sMethod) const
{
   if (m_rStateContext.m_allowedRemote.index(sMethod) != UtlString::UTLSTRING_NOT_FOUND ||
       m_rStateContext.m_implicitAllowedRemote.index(sMethod) != UtlString::UTLSTRING_NOT_FOUND)
   {
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}

UtlBoolean BaseSipConnectionState::isExtensionSupported(const UtlString& sExtension) const
{
   if (m_rStateContext.m_supportedRemote.index(sExtension) != UtlString::UTLSTRING_NOT_FOUND)
   {
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}

void BaseSipConnectionState::deleteMediaConnection()
{
   CpMediaInterface* pInterface = m_pMediaInterfaceProvider->getMediaInterface(FALSE);

   if (pInterface)
   {
      int mediaConnectionId = getMediaConnectionId();

      if (mediaConnectionId != CpMediaInterface::INVALID_CONNECTION_ID)
      {
         setMediaConnectionId(CpMediaInterface::INVALID_CONNECTION_ID);

         // media interface exists, shut down media connection
         pInterface->stopRtpSend(mediaConnectionId);
         pInterface->stopRtpReceive(mediaConnectionId);
         pInterface->deleteConnection(mediaConnectionId);

         setRemoteMediaConnectionState(SipConnectionStateContext::MEDIA_CONNECTION_NONE, FALSE);
         setLocalMediaConnectionState(SipConnectionStateContext::MEDIA_CONNECTION_NONE);
      }
   }
}

void BaseSipConnectionState::terminateSipDialog()
{
   {
      OsWriteLock lock(m_rStateContext);
      m_rStateContext.m_sipDialog.terminateDialog();
   }
}

UtlBoolean BaseSipConnectionState::isLocalInitiatedDialog()
{
   OsWriteLock lock(m_rStateContext);
   return m_rStateContext.m_sipDialog.isLocalInitiatedDialog();
}

CpSipTransactionManager& BaseSipConnectionState::getClientTransactionManager() const
{
   return m_rStateContext.m_sipClientTransactionMgr;
}

CpSipTransactionManager& BaseSipConnectionState::getServerTransactionManager() const
{
   return m_rStateContext.m_sipServerTransactionMgr;
}

int BaseSipConnectionState::getMediaConnectionId() const
{
   // no need to lock atomic
   return m_rStateContext.m_mediaConnectionId;
}

void BaseSipConnectionState::setMediaConnectionId(int mediaConnectionId)
{
   // no need to lock atomic
   m_rStateContext.m_mediaConnectionId = mediaConnectionId;

   if (mediaConnectionId != CpMediaInterface::INVALID_CONNECTION_ID)
   {
      // this one is only used for routing media events after media connection destruction
      m_rStateContext.m_mediaEventConnectionId = mediaConnectionId;
   }
}

SipConnectionStateTransition* BaseSipConnectionState::handle100RelTimerMessage(const Sc100RelTimerMsg& timerMsg)
{
   delete100relRetransmitTimer();

   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState != ISipConnectionState::CONNECTION_ESTABLISHED && // we never send 100rel when established
      connectionState != ISipConnectionState::CONNECTION_DISCONNECTED &&
      connectionState != ISipConnectionState::CONNECTION_UNKNOWN)
   {
      UtlString s100relId = timerMsg.get100RelId();

      if (!m_rStateContext.m_100RelTracker.wasPrackReceived(s100relId) &&
         m_rStateContext.m_i100relRetransmitCount < MAX_100REL_RETRANSMIT_COUNT)
      {
         // PRACK was not received, resend 100rel response
         SipMessage c100relResponse;
         timerMsg.get100relResponse(c100relResponse);
         // increase counter
         m_rStateContext.m_i100relRetransmitCount++;
         start100relRetransmitTimer(c100relResponse);
         // resend
         sendMessage(c100relResponse);
      }
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::handle2xxTimerMessage(const Sc2xxTimerMsg& timerMsg)
{
   if (!m_rStateContext.m_bAckReceived && m_rStateContext.m_pLastSent2xxToInvite)
   {
      if (m_rStateContext.m_i2xxInviteRetransmitCount < 8)
      {
         // resend message
         sendMessage(*m_rStateContext.m_pLastSent2xxToInvite);
         start2xxRetransmitTimer();
         return NULL;
      }
      else
      {
         delete2xxRetransmitTimer();
         sendBye(487, "Request terminated due to missing ACK");
         // we sent 2xx too many times, fail
         return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, NULL);
      }
   }

   delete2xxRetransmitTimer();
   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::handleDisconnectTimerMessage(const ScDisconnectTimerMsg& timerMsg)
{
   GeneralTransitionMemory memory(CP_CALLSTATE_CAUSE_NO_RESPONSE);
   return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, &memory);
}

SipConnectionStateTransition* BaseSipConnectionState::handleReInviteTimerMessage(const ScReInviteTimerMsg& timerMsg)
{
   switch (timerMsg.getReason())
   {
   case ScReInviteTimerMsg::REASON_NORMAL:
   case ScReInviteTimerMsg::REASON_HOLD:
   case ScReInviteTimerMsg::REASON_UNHOLD:
      return handleRenegotiateTimerMessage(timerMsg);
   case ScReInviteTimerMsg::REASON_SESSION_EXTENSION:
      return handleRefreshSessionTimerMessage(timerMsg);
   default:
      break;
   }
   
   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::handleRenegotiateTimerMessage(const ScReInviteTimerMsg& timerMsg)
{
   deleteSessionRenegotiationTimer();

   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState == ISipConnectionState::CONNECTION_ESTABLISHED)
   {
      // we are in a state where we can do hold
      if (mayRenegotiateMediaSession())
      {
         switch (timerMsg.getReason())
         {
         case ScReInviteTimerMsg::REASON_NORMAL:
            refreshSession(TRUE);
            break;
         case ScReInviteTimerMsg::REASON_HOLD:
            doHold();
            break;
         case ScReInviteTimerMsg::REASON_UNHOLD:
            doUnhold();
            break;
         default:
            ;
         }
      }
      else
      {
         if (m_rStateContext.m_iRenegotiationRetryCount < MAX_RENEGOTIATION_RETRY_COUNT)
         {
            // we may try again
            startSessionRenegotiationTimer(timerMsg.getReason(), FALSE);
         }
         else
         {
            OsSysLog::add(FAC_CP, PRI_WARNING, "Giving up renegotiate (%i) operation, not ready to renegotiate session after %i tries\n",
               (int)timerMsg.getReason(), MAX_RENEGOTIATION_RETRY_COUNT);
         }
      }
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::handleRefreshSessionTimerMessage(const ScReInviteTimerMsg& timerMsg)
{
   deleteSessionRefreshTimer();

   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState == ISipConnectionState::CONNECTION_ESTABLISHED)
   {
      // we are in a state where we can do hold
      if (mayRenegotiateMediaSession())
      {
         OsSysLog::add(FAC_CP, PRI_DEBUG, "About to renegotiate session due to session timer for sip call-id %s.\n",
            getSipCallId().data());
         refreshSession(FALSE);
      } // else some renegotiation is in progress, just restart timer, no need to refresh

      startSessionRefreshTimer();
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::handleSessionTimeoutCheckTimerMessage(const ScSessionTimeoutTimerMsg& timerMsg)
{
   deleteSessionTimeoutCheckTimer();

   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState == ISipConnectionState::CONNECTION_ESTABLISHED)
   {
      if (m_rStateContext.m_sessionTimerProperties.isSessionStale())
      {
         OsSysLog::add(FAC_CP, PRI_WARNING, "Session timeout: session with call-id %s is too old, no session refresh occurred within session expiration time, dropping session.\n",
            getSipCallId().data());

         // session is stale (too old, not refreshed), terminate it with BYE
         if (!m_rStateContext.m_bCancelSent)
         {
            sendBye(487, "Session Timeout"); // this kicks off bye timeout timer
         }
      }
      else
      {
         // session is not stale, restart timeout timer
         startSessionTimeoutCheckTimer();
      }
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::handleInviteExpirationTimerMessage(const ScInviteExpirationTimerMsg& timerMsg)
{
   deleteInviteExpirationTimer();

   int cseqNum = timerMsg.getCseqNum();
   UtlBoolean bIsOutbound = timerMsg.getIsOutbound();
   CpSipTransactionManager& rTransactionMgr = bIsOutbound ? getClientTransactionManager() : getServerTransactionManager();

   if (cseqNum == rTransactionMgr.getInviteCSeqNum() && rTransactionMgr.isInviteTransactionActive())
   {
      // we haven't received final response for INVITE request, or haven't sent final response to initial INVITE
      ISipConnectionState::StateEnum connectionState = getCurrentState();

      if (bIsOutbound)
      {
         if (connectionState == ISipConnectionState::CONNECTION_REMOTE_OFFERING ||
            connectionState == ISipConnectionState::CONNECTION_REMOTE_ALERTING ||
            connectionState == ISipConnectionState::CONNECTION_REMOTE_QUEUED)
         {
            // this is outbound initial INVITE
            sendInviteCancel(487, "INVITE request expired");

            // this is not a problem
            OsSysLog::add(FAC_CP, PRI_DEBUG, "INVITE expired: remote party didn't answer with final response. Sending CANCEL. call-id: %s",
               getCallId().data());
         }
         else if (connectionState == ISipConnectionState::CONNECTION_ESTABLISHED)
         {
            // this is re-INVITE. Remote party is not responding with final response
            rTransactionMgr.endInviteTransaction();
            m_rStateContext.m_sdpNegotiation.resetSdpNegotiation();

            // this is a problem but not fatal
            OsSysLog::add(FAC_CP, PRI_WARNING, "re-INVITE expired: remote party didn't answer with final response. Ending transaction. call-id: %s",
               getCallId().data());
         }
      }
      else if (m_rStateContext.m_pLastReceivedInvite)
      {
         // is inbound INVITE
         if (connectionState == ISipConnectionState::CONNECTION_ALERTING ||
            connectionState == ISipConnectionState::CONNECTION_OFFERING ||
            connectionState == ISipConnectionState::CONNECTION_QUEUED)
         {
            SipMessage sipResponse;
            sipResponse.setResponseData(m_rStateContext.m_pLastReceivedInvite, SIP_TEMPORARILY_UNAVAILABLE_CODE,
               SIP_TEMPORARILY_UNAVAILABLE_TEXT);
            sendMessage(sipResponse);

            OsSysLog::add(FAC_CP, PRI_DEBUG, "INVITE expired: local user didn't answer call within expiration time. Rejecting call. call-id: %s",
               getCallId().data());

            GeneralTransitionMemory memory(CP_CALLSTATE_CAUSE_REJECTED);
            return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, &memory);
         } // else something is wrong
      }
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::handleByeRetryTimerMessage(const ScByeRetryTimerMsg& timerMsg)
{
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   deleteByeRetryTimer();

   // INVITE transaction termination is handled during disconnected state entry
   if (connectionState == ISipConnectionState::CONNECTION_ESTABLISHED)
   {
      if (m_rStateContext.m_bAckReceived)
      {
         // we may now send bye
         OsStatus result;
         return doByeConnection(result);
      }
      else
      {
         m_rStateContext.m_iByeRetryCount++;
         if (m_rStateContext.m_iByeRetryCount < 5)
         {
            // retry again
            startByeRetryTimer();
         }
         else
         {
            // too many tries
            return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, NULL);
         }
      }
   }

   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::handleDelayedAnswerTimerMessage(const ScDelayedAnswerTimerMsg& timerMsg)
{
   deleteDelayedAnswerTimer();

   OsStatus result;
   return answerConnection(result); // try to answer again, if still cannot new timer will be started
}

SipConnectionStateTransition* BaseSipConnectionState::handleConnStateNotificationMessage(const ScConnStateNotificationMsg& rMsg)
{
   SipDialog sourceSipDialog;
   rMsg.getSourceSipDialog(sourceSipDialog);
   ISipConnectionState::StateEnum sourceState = rMsg.getState();
   ISipConnectionState::StateEnum currentState = getCurrentState();

   if (currentState == ISipConnectionState::CONNECTION_ESTABLISHED)
   {
      if (m_rStateContext.m_localEntityType == SipConnectionStateContext::ENTITY_TRANSFEREE &&
         m_rStateContext.m_transferSipDialog.compareDialogs(sourceSipDialog) != SipDialog::DIALOG_MISMATCH)
      {
         // send NOTIFY if needed
         if (m_rStateContext.m_referOutSubscriptionActive && isMethodAllowed(SIP_NOTIFY_METHOD))
         {
            sendReferNotify(sourceState);
         }

         // we are transferee and dialogs match
         if (sourceState == ISipConnectionState::CONNECTION_ESTABLISHED)
         {
            // transfer successful, drop our current call
            sendBye();
         }
         else if (sourceState == ISipConnectionState::CONNECTION_DISCONNECTED)
         {
            // transfer unsuccessful, reset our substate, we can continue
            m_rStateContext.m_localEntityType = SipConnectionStateContext::ENTITY_NORMAL;
            m_rStateContext.m_inboundReferResponse = SipConnectionStateContext::REFER_NO_RESPONSE;
            // fire connected event, so that call is restored to normal state
            m_rSipConnectionEventSink.fireSipXCallEvent(CP_CALLSTATE_CONNECTED, CP_CALLSTATE_CAUSE_NORMAL);
            delete m_rStateContext.m_pLastReceivedRefer;
            m_rStateContext.m_pLastReceivedRefer = NULL;
         }
      }
   }

   return NULL;
}

UtlString BaseSipConnectionState::getCallId() const
{
   OsReadLock lock(m_rStateContext);
   return m_rStateContext.m_sipDialog.getCallId();
}

UtlBoolean BaseSipConnectionState::needsTransactionTracking(const UtlString& sipMethod) const
{
   UtlString trackable(TRACKABLE_METHODS);

   if (trackable.index(sipMethod) != UtlString::UTLSTRING_NOT_FOUND)
   {
      return TRUE;
   }

   return FALSE;
}

UtlString BaseSipConnectionState::buildContactUrl(const Url& fromUrl) const
{
   UtlString sContact;
   if (m_rStateContext.m_contactId != AUTOMATIC_CONTACT_ID)
   {
      // contact id is known
      SipContact* pContact = m_rSipUserAgent.getContactDb().find(m_rStateContext.m_contactId);
      if (pContact)
      {
         // Get display name and user id from from Url
         UtlString displayName;
         UtlString userId;
         fromUrl.getDisplayName(displayName);
         fromUrl.getUserId(userId);

         Url contactUrl;
         pContact->buildContactUri(displayName, userId, contactUrl);
         contactUrl.toString(sContact);

         delete pContact;
         pContact = NULL;
         return sContact;
      }
   }

   return buildDefaultContactUrl(fromUrl);
}

void BaseSipConnectionState::initDialogContact(CP_CONTACT_ID contactId, const Url& localField)
{
   m_rStateContext.m_contactId = contactId;

   UtlString sContactUrl = buildContactUrl(localField);
   {
      OsWriteLock lock(m_rStateContext);
      m_rStateContext.m_sipDialog.setLocalContact(sContactUrl.data());
   }
}

void BaseSipConnectionState::getLocalContactUrl(Url& contactUrl) const
{
   Url localField;
   {
      OsReadLock lock(m_rStateContext);
      m_rStateContext.m_sipDialog.getLocalContact(contactUrl);
   }
}

UtlString BaseSipConnectionState::getLocalContactUrl() const
{
   Url localContactUrl;
   getLocalContactUrl(localContactUrl);
   return localContactUrl.toString();
}

UtlString BaseSipConnectionState::buildDefaultContactUrl(const Url& fromUrl) const
{
   Url contactUrl;
   // Get display name and user id from from Url
   UtlString displayName;
   UtlString userId;
   fromUrl.getDisplayName(displayName);
   fromUrl.getUserId(userId);

   SipContact* pContact = NULL;
   if (m_rStateContext.m_sBindIpAddress.compareTo("0.0.0.0") != 0)
   {
      // we have specific IP address
      pContact = m_rSipUserAgent.getContactDb().find(m_rStateContext.m_sBindIpAddress,
         SIP_CONTACT_AUTO, m_rStateContext.m_transportType);
   }
   else
   {
      // we are bound to all IP addresses, we can't really select a good contact now
      // it will be overridden later in SipUserAgent
      pContact = m_rSipUserAgent.getContactDb().find(SIP_CONTACT_AUTO, m_rStateContext.m_transportType);
   }

   if (pContact)
   {
      pContact->buildContactUri(displayName, userId, contactUrl);
      delete pContact;
      pContact = NULL;
      return contactUrl.toString();
   }

   // we should always be able to build some contact
   OsSysLog::add(FAC_CP, PRI_ERR, "Failed to build automatic contact for %s\n",
      fromUrl.toString().data());

   return NULL;
}

UtlString BaseSipConnectionState::getLocalTag() const
{
   UtlString localTag;
   {
      OsReadLock lock(m_rStateContext);
      m_rStateContext.m_sipDialog.getLocalTag(localTag);
   }
   return localTag;
}

void BaseSipConnectionState::secureUrl(Url& url) const
{
   SipContact* pContact = m_rSipUserAgent.getContactDb().find(m_rStateContext.m_contactId);
   if (pContact)
   {
      if (pContact->getTransportType() == SIP_TRANSPORT_TLS)
      {
         url.setScheme(Url::SipsUrlScheme);
      }
      delete pContact;
      pContact = NULL;
   }
}

UtlBoolean BaseSipConnectionState::setupMediaConnection(RTP_TRANSPORT rtpTransportOptions, int& mediaConnectionId)
{
   OsStatus res = m_pMediaInterfaceProvider->getMediaInterface()->createConnection(mediaConnectionId,
               m_rStateContext.m_sBindIpAddress,
               0,
               NULL, // no display settings
               (void*)m_rStateContext.m_pSecurity,
               &m_pMessageQueueProvider->getLocalQueue(),
               rtpTransportOptions);
   
   if (res == OS_SUCCESS)
   {
      setMediaConnectionId(mediaConnectionId);
      return TRUE;
   }

   return FALSE;
}

UtlBoolean BaseSipConnectionState::prepareSdpOffer(SipMessage& sipMessage)
{
   m_rStateContext.m_sdpNegotiation.startSdpNegotiation(sipMessage, m_rStateContext.m_bUseLocalHoldSDP);
   CpMediaInterface* pMediaInterface = m_pMediaInterfaceProvider->getMediaInterface();

   int mediaConnectionId = getMediaConnectionId();
   if (mediaConnectionId == CpMediaInterface::INVALID_CONNECTION_ID)
   {
      if (!setupMediaConnection(m_rStateContext.m_rtpTransport, mediaConnectionId))
      {
         return FALSE;
      }
   }

   SipContact* pContact = m_rSipUserAgent.getContactDb().find(m_rStateContext.m_contactId);
   if (pContact)
   {
      pMediaInterface->setContactType(mediaConnectionId, pContact->getContactType(),
         m_rStateContext.m_contactId);
      delete pContact;
      pContact = NULL;
   }
   else
   {
      pMediaInterface->setContactType(mediaConnectionId, SIP_CONTACT_AUTO, AUTOMATIC_CONTACT_ID);
   }

   UtlString hostAddresses[MAX_ADDRESS_CANDIDATES];
   int receiveRtpPorts[MAX_ADDRESS_CANDIDATES];
   int receiveRtcpPorts[MAX_ADDRESS_CANDIDATES];
   int receiveVideoRtpPorts[MAX_ADDRESS_CANDIDATES];
   int receiveVideoRtcpPorts[MAX_ADDRESS_CANDIDATES];
   RTP_TRANSPORT transportTypes[MAX_ADDRESS_CANDIDATES];
   int nRtpContacts;
   int totalBandwidth = 0;
   SdpSrtpParameters srtpParams;
   SdpCodecList supportedCodecs;
   int videoFramerate = 0;
   OsStatus res = pMediaInterface->getCapabilitiesEx(mediaConnectionId,
         MAX_ADDRESS_CANDIDATES,
         hostAddresses,
         receiveRtpPorts,
         receiveRtcpPorts,
         receiveVideoRtpPorts,
         receiveVideoRtcpPorts,
         transportTypes,
         nRtpContacts,
         supportedCodecs,
         srtpParams,
         totalBandwidth,
         videoFramerate);

   if (res == OS_SUCCESS)
   {
      if (!m_natTraversalConfig.m_bEnableICE)
      {
         nRtpContacts = 1;
      }

      m_rStateContext.m_sdpNegotiation.addSdpOffer(sipMessage,
         nRtpContacts, hostAddresses, receiveRtpPorts, receiveRtcpPorts, receiveVideoRtpPorts, receiveVideoRtcpPorts,
         transportTypes, supportedCodecs, srtpParams, totalBandwidth, videoFramerate, m_rStateContext.m_rtpTransport);
      return TRUE;
   }

   // creation of SDP offer failed for some reason, reset SDP negotiation
   m_rStateContext.m_sdpNegotiation.resetSdpNegotiation();
   return FALSE;
}

UtlBoolean BaseSipConnectionState::handleSdpOffer(const SipMessage& sipMessage)
{
   const SdpBody* pSdpBody = sipMessage.getSdpBody(m_rStateContext.m_pSecurity);
   if (pSdpBody)
   {
      m_rStateContext.m_sdpNegotiation.startSdpNegotiation(sipMessage, m_rStateContext.m_bUseLocalHoldSDP);

      if (!m_rStateContext.m_bRTPRedirectActive)
      {
         // if we are not redirecting RTP, initialize media connection if not already initialized
         CpMediaInterface* pMediaInterface = m_pMediaInterfaceProvider->getMediaInterface();
         int mediaConnectionId = getMediaConnectionId();

         if (mediaConnectionId == CpMediaInterface::INVALID_CONNECTION_ID)
         {
            if (!setupMediaConnection(m_rStateContext.m_rtpTransport, mediaConnectionId))
            {
               m_rStateContext.m_sdpNegotiation.resetSdpNegotiation();
               return FALSE;
            }
         }
      }

      if (m_rStateContext.m_sdpNegotiation.handleInboundSdpOffer(sipMessage))
      {
         if (handleRemoteSdpBody(*pSdpBody))
         {
            return TRUE;
         }
      }
   }

   // handling of SDP offer failed for some reason, reset SDP negotiation
   m_rStateContext.m_sdpNegotiation.resetSdpNegotiation();
   return FALSE;
}

UtlBoolean BaseSipConnectionState::prepareSdpAnswer(SipMessage& sipMessage)
{
   CpMediaInterface* pMediaInterface = m_pMediaInterfaceProvider->getMediaInterface();

   int mediaConnectionId = getMediaConnectionId();
   if (mediaConnectionId == CpMediaInterface::INVALID_CONNECTION_ID)
   {
      m_rStateContext.m_sdpNegotiation.resetSdpNegotiation();
      return FALSE;
   }

   if (m_rStateContext.m_sdpNegotiation.isSdpOfferFinished())
   {
      SipContact* pContact = m_rSipUserAgent.getContactDb().find(m_rStateContext.m_contactId);
      if (pContact)
      {
         pMediaInterface->setContactType(mediaConnectionId, pContact->getContactType(),
            m_rStateContext.m_contactId);
         delete pContact;
         pContact = NULL;
      }
      else
      {
         pMediaInterface->setContactType(mediaConnectionId, SIP_CONTACT_AUTO, AUTOMATIC_CONTACT_ID);
      }

      UtlString hostAddresses[MAX_ADDRESS_CANDIDATES];
      int receiveRtpPorts[MAX_ADDRESS_CANDIDATES];
      int receiveRtcpPorts[MAX_ADDRESS_CANDIDATES];
      int receiveVideoRtpPorts[MAX_ADDRESS_CANDIDATES];
      int receiveVideoRtcpPorts[MAX_ADDRESS_CANDIDATES];
      RTP_TRANSPORT transportTypes[MAX_ADDRESS_CANDIDATES];
      int nRtpContacts;
      int totalBandwidth = 0;
      SdpSrtpParameters srtpParams;
      SdpCodecList supportedCodecs;
      int videoFramerate = 0;
      OsStatus res = pMediaInterface->getCapabilitiesEx(mediaConnectionId,
         MAX_ADDRESS_CANDIDATES,
         hostAddresses,
         receiveRtpPorts,
         receiveRtcpPorts,
         receiveVideoRtpPorts,
         receiveVideoRtcpPorts,
         transportTypes,
         nRtpContacts,
         supportedCodecs,
         srtpParams,
         totalBandwidth,
         videoFramerate);

      if (res == OS_SUCCESS)
      {
         if (!m_natTraversalConfig.m_bEnableICE)
         {
            nRtpContacts = 1;
         }

         m_rStateContext.m_sdpNegotiation.addSdpAnswer(sipMessage,
            nRtpContacts, hostAddresses, receiveRtpPorts, receiveRtcpPorts, receiveVideoRtpPorts, receiveVideoRtcpPorts,
            transportTypes, supportedCodecs, srtpParams, totalBandwidth, videoFramerate, m_rStateContext.m_rtpTransport);
         return TRUE;
      }
   }

   // creation of SDP answer failed for some reason, reset SDP negotiation
   m_rStateContext.m_sdpNegotiation.resetSdpNegotiation();

   return FALSE;
}

UtlBoolean BaseSipConnectionState::handleSdpAnswer(const SipMessage& sipMessage)
{
   const SdpBody* pSdpBody = sipMessage.getSdpBody(m_rStateContext.m_pSecurity);

   if (pSdpBody)
   {
      if (m_rStateContext.m_sdpNegotiation.handleInboundSdpAnswer(sipMessage))
      {
         if (handleRemoteSdpBody(*pSdpBody))
         {
            return TRUE;
         }
      }
   }

   m_rStateContext.m_sdpNegotiation.resetSdpNegotiation();
   return FALSE;
}

UtlBoolean BaseSipConnectionState::handleRemoteSdpBody(const SdpBody& sdpBody)
{
   UtlString rtpAddress;
   int videoFramerate = 0;
   int matchingVideoFramerate = 0;
   SdpCodecList supportedCodecs;
   SdpCodecList commonCodecsForEncoder;
   SdpCodecList commonCodecsForDecoder;
   SdpSrtpParameters srtpParams;
   memset(&srtpParams, 0, sizeof(srtpParams));
   int numMatchingCodecs = 0;
   SdpSrtpParameters matchingSrtpParams;
   memset(&matchingSrtpParams, 0, sizeof(matchingSrtpParams));
   UtlString remoteRtpAddress; // only used if ICE is disabled
   int remoteRtpPort = -1; // only used if ICE is disabled
   int remoteRtcpPort = -1; // only used if ICE is disabled
   int remoteVideoRtpPort = -1; // only used if ICE is disabled
   int remoteVideoRtcpPort = -1; // only used if ICE is disabled

   int mediaConnectionId = getMediaConnectionId();
   CpMediaInterface* pMediaInterface = m_pMediaInterfaceProvider->getMediaInterface();
   pMediaInterface->getCodecList(mediaConnectionId, supportedCodecs);

   CpSdpNegotiation::getCommonSdpCodecs(sdpBody,
      supportedCodecs, numMatchingCodecs,
      commonCodecsForEncoder, commonCodecsForDecoder,
      remoteRtpAddress, remoteRtpPort, remoteRtcpPort, remoteVideoRtpPort, remoteVideoRtcpPort,
      srtpParams, matchingSrtpParams,
      videoFramerate, matchingVideoFramerate);

   if (numMatchingCodecs > 0 &&
      commonCodecsForEncoder.hasNonSignallingCodec(MIME_TYPE_AUDIO) &&
      commonCodecsForDecoder.hasNonSignallingCodec(MIME_TYPE_AUDIO))
   {
      return TRUE;
   }
   return FALSE;
}

UtlBoolean BaseSipConnectionState::commitMediaSessionChanges()
{
   if (m_rStateContext.m_sdpNegotiation.isSdpNegotiationComplete() ||
      m_rStateContext.m_sdpNegotiation.isSdpNegotiationUnreliablyComplete())
   {
      SdpCodecList localSdpCodecList;
      m_rStateContext.m_sdpNegotiation.getLocalSdpCodecList(localSdpCodecList);
      SdpBody sdpBody;
      UtlBoolean bodyFound = m_rStateContext.m_sdpNegotiation.getRemoteSdpBody(sdpBody);

      if (bodyFound)
      {
         UtlString rtpAddress;
         int videoFramerate = 0;
         int matchingVideoFramerate = 0;
         SdpCodecList supportedCodecs;
         SdpCodecList commonCodecsForEncoder;
         SdpCodecList commonCodecsForDecoder;
         SdpSrtpParameters srtpParams;
         memset(&srtpParams, 0, sizeof(srtpParams));
         int numMatchingCodecs = 0;
         SdpSrtpParameters matchingSrtpParams;
         memset(&matchingSrtpParams, 0, sizeof(matchingSrtpParams));
         UtlString remoteRtpAddress; // only used if ICE is disabled
         int remoteRtpPort = -1; // only used if ICE is disabled
         int remoteRtcpPort = -1; // only used if ICE is disabled
         int remoteVideoRtpPort = -1; // only used if ICE is disabled
         int remoteVideoRtcpPort = -1; // only used if ICE is disabled

         int mediaConnectionId = getMediaConnectionId();
         CpMediaInterface* pMediaInterface = m_pMediaInterfaceProvider->getMediaInterface();
         pMediaInterface->getCodecList(mediaConnectionId, supportedCodecs);

         CpSdpNegotiation::getCommonSdpCodecs(sdpBody,
            supportedCodecs, numMatchingCodecs,
            commonCodecsForEncoder, commonCodecsForDecoder,
            remoteRtpAddress, remoteRtpPort, remoteRtcpPort, remoteVideoRtpPort, remoteVideoRtcpPort,
            srtpParams, matchingSrtpParams,
            videoFramerate, matchingVideoFramerate);

         if (numMatchingCodecs > 0 &&
            commonCodecsForEncoder.hasNonSignallingCodec(MIME_TYPE_AUDIO) &&
            commonCodecsForDecoder.hasNonSignallingCodec(MIME_TYPE_AUDIO))
         {
            if (matchingVideoFramerate != 0)
            {
               pMediaInterface->setConnectionFramerate(mediaConnectionId, matchingVideoFramerate);
            }
            // Set up the remote RTP sockets
            setMediaDestination(remoteRtpAddress.data(),
               remoteRtpPort,
               remoteRtcpPort,
               remoteVideoRtpPort,
               remoteVideoRtcpPort,
               &sdpBody);

            if (remoteRtpAddress.compareTo("0.0.0.0") == 0) // hold address
            {
               pMediaInterface->stopRtpSend(mediaConnectionId);
               setRemoteMediaConnectionState(SipConnectionStateContext::MEDIA_CONNECTION_HELD, FALSE);
            }
            else
            {
               pMediaInterface->startRtpSend(mediaConnectionId, commonCodecsForEncoder);
               setRemoteMediaConnectionState(SipConnectionStateContext::MEDIA_CONNECTION_ACTIVE, FALSE);
            }

            if (!m_rStateContext.m_bUseLocalHoldSDP)
            {
               pMediaInterface->startRtpReceive(mediaConnectionId, commonCodecsForDecoder);
               setLocalMediaConnectionState(SipConnectionStateContext::MEDIA_CONNECTION_ACTIVE);
            }
            else
            {
               // local hold was negotiated
               pMediaInterface->stopRtpReceive(mediaConnectionId);
               setLocalMediaConnectionState(SipConnectionStateContext::MEDIA_CONNECTION_HELD);
            }

            return TRUE;
         }
         else
         {
            setRemoteMediaConnectionState(SipConnectionStateContext::MEDIA_CONNECTION_NONE, FALSE);
            setLocalMediaConnectionState(SipConnectionStateContext::MEDIA_CONNECTION_NONE);
         }
      }
   }

   return FALSE;
}

void BaseSipConnectionState::setLastReceivedInvite(const SipMessage& sipMessage)
{
   if (m_rStateContext.m_pLastReceivedInvite)
   {
      delete m_rStateContext.m_pLastReceivedInvite;
      m_rStateContext.m_pLastReceivedInvite = NULL;
   }

   m_rStateContext.m_pLastReceivedInvite = new SipMessage(sipMessage);
}

void BaseSipConnectionState::setLastSent2xxToInvite(const SipMessage& sipMessage)
{
   if (m_rStateContext.m_pLastSent2xxToInvite)
   {
      delete m_rStateContext.m_pLastSent2xxToInvite;
      m_rStateContext.m_pLastSent2xxToInvite = NULL;
   }

   m_rStateContext.m_pLastSent2xxToInvite = new SipMessage(sipMessage);
}

void BaseSipConnectionState::setLastSentRefer(const SipMessage& sipMessage)
{
   if (m_rStateContext.m_pLastSentRefer)
   {
      delete m_rStateContext.m_pLastSentRefer;
      m_rStateContext.m_pLastSentRefer = NULL;
   }

   m_rStateContext.m_pLastSentRefer = new SipMessage(sipMessage);
}

void BaseSipConnectionState::setLastReceivedRefer(const SipMessage& sipMessage)
{
   if (m_rStateContext.m_pLastReceivedRefer)
   {
      delete m_rStateContext.m_pLastReceivedRefer;
      m_rStateContext.m_pLastReceivedRefer = NULL;
   }

   m_rStateContext.m_pLastReceivedRefer = new SipMessage(sipMessage);
}

CpSessionTimerProperties& BaseSipConnectionState::getSessionTimerProperties()
{
   return m_rStateContext.m_sessionTimerProperties;
}

void BaseSipConnectionState::handleSmallSessionIntervalResponse(const SipMessage& sipResponse)
{
   int minSe = 90;
   int seqNumber = 0;
   UtlString seqMethod;

   sipResponse.getCSeqField(seqNumber, seqMethod);
   CpSipTransactionManager::TransactionState inviteState = getClientTransactionManager().getTransactionState(seqMethod, seqNumber);
   getClientTransactionManager().endTransaction(seqMethod, seqNumber);

   if (inviteState == CpSipTransactionManager::TRANSACTION_ACTIVE &&
       sipResponse.getMinSe(minSe))
   {
      // update session timer properties from response
      m_rStateContext.m_sessionTimerProperties.setMinSessionExpires(minSe);
      m_rStateContext.m_sessionTimerProperties.setSessionExpires(minSe);

      if (mayRenegotiateMediaSession())
      {
         if (seqMethod.compareTo(SIP_INVITE_METHOD) == 0)
         {
            sendInvite();
         }
         else if (seqMethod.compareTo(SIP_UPDATE_METHOD) == 0)
         {
            sendUpdate();
         }
      }
   }
}

void BaseSipConnectionState::sendOptionsRequest()
{
   // allow was not set in response, send OPTIONS
   int iCSeq = getNextLocalCSeq();
   SipMessage optionsRequest;
   prepareSipRequest(optionsRequest, SIP_OPTIONS_METHOD, iCSeq);
   sendMessage(optionsRequest);
}

void BaseSipConnectionState::discoverRemoteCapabilities()
{
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState != ISipConnectionState::CONNECTION_DISCONNECTED &&
      connectionState != ISipConnectionState::CONNECTION_UNKNOWN)
   {
      UtlBoolean bIsDialogEstablished = FALSE;
      {
         OsReadLock lock(m_rStateContext);
         bIsDialogEstablished = m_rStateContext.m_sipDialog.isEstablishedDialog();
      }

      if (bIsDialogEstablished && !m_rStateContext.m_bCancelSent && !m_rStateContext.m_bByeSent)
      {
         // allow was not set in response, send in dialog OPTIONS
         sendOptionsRequest();
      }
   }
}

SipConnectionStateTransition* BaseSipConnectionState::handleInvite2xxResponse(const SipMessage& sipResponse)
{
   int responseCode = sipResponse.getResponseStatusCode();
   UtlString responseText;
   sipResponse.getResponseStatusText(&responseText);
   int seqNum;
   UtlString seqMethod;
   sipResponse.getCSeqField(&seqNum, &seqMethod);
   UtlBoolean bProtocolError = FALSE;
   m_rStateContext.m_491failureCounter = 0;

   // get pointer to internal sdp bodyContent
   const SdpBody* pSdpBody = sipResponse.getSdpBody(m_rStateContext.m_pSecurity);
   SipMessage sipRequest;
   prepareSipRequest(sipRequest, SIP_ACK_METHOD, seqNum);

   CpSipTransactionManager::TransactionState transactionState = getClientTransactionManager().getTransactionState(seqMethod, seqNum);
   CpSdpNegotiation::SdpNegotiationState negotiationState = m_rStateContext.m_sdpNegotiation.getNegotiationState();

   if (pSdpBody)
   {
      if (transactionState == CpSipTransactionManager::TRANSACTION_ACTIVE ||
          m_rStateContext.m_sdpNegotiation.isInSdpNegotiation(sipResponse))
      {
         // response has SDP bodyContent, we must provide SDP answer
         if (transactionState == CpSipTransactionManager::TRANSACTION_TERMINATED &&
            negotiationState == CpSdpNegotiation::SDP_NEGOTIATION_COMPLETE)
         {
            // this 200 OK must be a retransmission
            CpSdpNegotiation::SdpBodyType sdpBodyType = m_rStateContext.m_sdpNegotiation.getSdpBodyType(sipResponse);
            if (sdpBodyType == CpSdpNegotiation::SDP_BODY_OFFER)
            {
               bProtocolError = !prepareSdpAnswer(sipRequest); // add sdp answer
            }
            else if (sdpBodyType == CpSdpNegotiation::SDP_BODY_UNKNOWN)
            {
               prepareSdpAnswer(sipRequest);
               bProtocolError = TRUE; // this is protocol error
            } // if sdp answer, then we already handled it before
         }
         else if (negotiationState == CpSdpNegotiation::SDP_NEGOTIATION_IN_PROGRESS)
         {
            // message has sdp bodyContent, and we are in negotiation (SDP was in INVITE)
            bProtocolError = !handleSdpAnswer(sipResponse);
         }
         else if (negotiationState == CpSdpNegotiation::SDP_NOT_NEGOTIATED)
         {
            // delayed SDP negotiation
            bProtocolError = !handleSdpOffer(sipResponse); // if we get false, we need to terminate call
            if (!bProtocolError)
            {
               bProtocolError = !prepareSdpAnswer(sipRequest);
            } // if protocol error, then sdp negotiation was reset already, and we cannot send SDP answer in ACK
         }
         //prepareSdpAnswer
      }
      else if (transactionState == CpSipTransactionManager::TRANSACTION_TERMINATED)
      {
         // transaction is terminated and is not part of SDP negotiation
         bProtocolError = TRUE;
      }
   }

   negotiationState = m_rStateContext.m_sdpNegotiation.getNegotiationState();
   if (negotiationState != CpSdpNegotiation::SDP_NEGOTIATION_COMPLETE)
   {
      bProtocolError = TRUE;
   }
   else
   {
      if (!bProtocolError)
      {
         // SDP negotiation is complete, commit changes
         commitMediaSessionChanges();
      }
   }

   if (!bProtocolError)
   {
      handleSessionTimerResponse(sipResponse);
   }

   if (!sendMessage(sipRequest)) // send ACK
   {
      GeneralTransitionMemory memory(CP_CALLSTATE_CAUSE_NETWORK);
      return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, &memory);
   }

   if (bProtocolError)
   {
      sendBye(487, "Request terminated due to protocol error"); // also send BYE
   }
   else if (m_rStateContext.m_bCancelSent)
   {
      sendBye(487, "Request Terminated"); // also send BYE
   }
   else if (getCurrentState() != ISipConnectionState::CONNECTION_ESTABLISHED)
   {
      // progress to established
      SipResponseTransitionMemory memory(responseCode, responseText);
      return getTransition(ISipConnectionState::CONNECTION_ESTABLISHED, NULL);
   }

   return NULL;
}

void BaseSipConnectionState::sendBye(int cause, const UtlString& text)
{
   if (!m_rStateContext.m_bByeSent)
   {
      // send BYE
      m_rStateContext.m_bByeSent = TRUE;
      SipMessage byeRequest;
      int seqNum = getNextLocalCSeq();
      prepareSipRequest(byeRequest, SIP_BYE_METHOD, seqNum);
      if (cause && !text.isNull())
      {
         byeRequest.setReasonField("SIP", cause, text);
      }
      sendMessage(byeRequest);
      deleteInviteExpirationTimer(); // INVITE was answered
      startByeTimeoutTimer(); // start bye timer to force destroy connection after some timeout
   }
}

void BaseSipConnectionState::sendInviteCancel(int cause, const UtlString& text)
{
   // send CANCEL
   if (!m_rStateContext.m_bCancelSent && // send only 1 CANCEL at time
       getClientTransactionManager().isInviteTransactionActive())
   {
      m_rStateContext.m_bCancelSent = TRUE;
      int seqNum = getClientTransactionManager().getInviteCSeqNum();

      SipMessage cancelRequest;
      prepareSipRequest(cancelRequest, SIP_CANCEL_METHOD, seqNum);
      if (cause && !text.isNull())
      {
         cancelRequest.setReasonField("SIP", cause, text);
      }
      sendMessage(cancelRequest);
      deleteInviteExpirationTimer(); // INVITE was answered
      startCancelTimeoutTimer(); // start cancel timer to force destroy connection after some timeout
   }
}

void BaseSipConnectionState::sendInvite()
{
   SipMessage sipInvite;
   int seqNum = getNextLocalCSeq();

   delete m_rStateContext.m_pLastSent2xxToInvite;
   m_rStateContext.m_pLastSent2xxToInvite = NULL;

   prepareSipRequest(sipInvite, SIP_INVITE_METHOD, seqNum);
   m_rStateContext.m_sessionTimerProperties.reset(FALSE); // reset refresher, so that it is negotiated again
   prepareSessionTimerRequest(sipInvite); // add session timer parameters
   maybeRequire100rel(sipInvite); // optionally require 100rel
   sipInvite.setExpiresField(m_rStateContext.m_inviteExpiresSeconds);
   startInviteExpirationTimer(m_rStateContext.m_inviteExpiresSeconds, seqNum, TRUE); // it will check again in m_inviteExpiresSeconds, if INVITE is finished
   announceConnectedIdentity(sipInvite);

   // add SDP if negotiation mode is immediate, otherwise don't add it
   if (m_rStateContext.m_sdpNegotiation.getSdpOfferingMode() == CpSdpNegotiation::SDP_OFFERING_IMMEDIATE)
   {
      if (!prepareSdpOffer(sipInvite))
      {
         // SDP negotiation start failed
         OsSysLog::add(FAC_CP, PRI_ERR, "SDP preparation for re-INVITE failed.\n");
         return;
      }
   }
   else
   {
      m_rStateContext.m_sdpNegotiation.resetSdpNegotiation();
   }

   // try to send sip message
   UtlBoolean sendSuccess = sendMessage(sipInvite);

   if (!sendSuccess)
   {
      OsSysLog::add(FAC_CP, PRI_ERR, "Sending re-INVITE failed.\n");
   }
}

void BaseSipConnectionState::sendUpdate(UtlBoolean bRenegotiateCodecs)
{
   SipMessage sipUpdate;
   int seqNum = getNextLocalCSeq();
   prepareSipRequest(sipUpdate, SIP_UPDATE_METHOD, seqNum);
   m_rStateContext.m_sessionTimerProperties.reset(FALSE); // reset refresher
   prepareSessionTimerRequest(sipUpdate);
   announceConnectedIdentity(sipUpdate);

   if (bRenegotiateCodecs)
   {
      if (!prepareSdpOffer(sipUpdate))
      {
         // SDP negotiation start failed
         OsSysLog::add(FAC_CP, PRI_ERR, "SDP preparation for UPDATE failed.\n");
         return;
      }
   }
   else
   {
      // reset SDP negotiation, so that we can detect that we are not expecting SDP in answer
      m_rStateContext.m_sdpNegotiation.resetSdpNegotiation();
   }

   // try to send sip message (maybe without SDP)
   UtlBoolean sendSuccess = sendMessage(sipUpdate);

   if (!sendSuccess)
   {
      OsSysLog::add(FAC_CP, PRI_ERR, "Sending UPDATE failed.\n");
   }
}

void BaseSipConnectionState::sendReferNotify(ISipConnectionState::StateEnum connectionState)
{
   int code;
   UtlString text;

   if (getReferNotifyCode(connectionState, code, text))
   {
      SipMessage sipNotify;
      int seqNum = getNextLocalCSeq();
      prepareSipRequest(sipNotify, SIP_NOTIFY_METHOD, seqNum);
      sipNotify.setEventField(SIP_EVENT_REFER);

      // construct sipfrag body
      SipMessage sipfragMessage;
      sipfragMessage.setResponseFirstHeaderLine(SIP_PROTOCOL_VERSION, code, text);
      UtlString messageBody;
      int len;
      sipfragMessage.getBytes(&messageBody,&len);
      HttpBody* body = new HttpBody(messageBody.data(), -1, CONTENT_TYPE_MESSAGE_SIPFRAG);

      sipNotify.setBody(body);
      sipNotify.setContentType(CONTENT_TYPE_MESSAGE_SIPFRAG);

      if (connectionState == ISipConnectionState::CONNECTION_ESTABLISHED ||
         connectionState == ISipConnectionState::CONNECTION_DISCONNECTED)
      {
         // final NOTIFY
         sipNotify.setSubscriptionState(SIP_SUBSCRIPTION_TERMINATED, "noresource");
         m_rStateContext.m_referOutSubscriptionActive = FALSE;
      }
      else
      {
         // non final NOTIFY
         sipNotify.setSubscriptionState(SIP_SUBSCRIPTION_ACTIVE, NULL, &m_rStateContext.m_inviteExpiresSeconds);
      }

      sendMessage(sipNotify);
   }
}

UtlBoolean BaseSipConnectionState::sendPrack(const SipMessage& sipResponse, UtlBoolean bSendSDPAnswer)
{
   UtlBoolean bSuccess = FALSE;

   if (sipResponse.is100RelResponse())
   {
      int seqNum = 0;
      int rseqNum = 0;
      UtlString seqMethod;
      sipResponse.getRSeqField(rseqNum);
      sipResponse.getCSeqField(&seqNum, &seqMethod);
      int prackSeqNum = getNextLocalCSeq(); // PRACK has new cseq number
      SipMessage sipPrack;
      prepareSipRequest(sipPrack, SIP_PRACK_METHOD, prackSeqNum);
      sipPrack.setRAckField(rseqNum, seqNum, seqMethod); // set properties of 1xx message we are confirming

      if (bSendSDPAnswer)
      {
         // we must send SDP answer
         if (prepareSdpAnswer(sipPrack))
         {
            bSuccess = TRUE;
         } // we never send SDP offer in PRACK, we prefer sending it in INVITE or ACK for outbound calls
      }
      else
      {
         bSuccess = TRUE;
      }

      bSuccess &= sendMessage(sipPrack);
   }

   return bSuccess;
}

void BaseSipConnectionState::setMediaDestination(const char* hostAddress,
                                                 int audioRtpPort,
                                                 int audioRtcpPort,
                                                 int videoRtpPort,
                                                 int videoRtcpPort,
                                                 const SdpBody* pRemoteBody)
{
   UtlBoolean bSetDestination = FALSE;

   int mediaConnectionId = getMediaConnectionId();
   CpMediaInterface* pMediaInterface = m_pMediaInterfaceProvider->getMediaInterface();

   /*
   * Assumption: that ICE is either enabled for both audio and video or not
   * at all.  If you attempt to mix, this won't work correctly.  To fix
   * this, we need to break setConnectionDestination(...) into two methods
   * -- one for audio and one for video.
   */
   if (pMediaInterface && hostAddress && (strcasecmp(hostAddress, "0.0.0.0") != 0))
   {
      if (pRemoteBody && m_natTraversalConfig.m_bEnableICE)
      {
         int         candidateIds[MAX_ADDRESS_CANDIDATES];
         UtlString   transportIds[MAX_ADDRESS_CANDIDATES];
         UtlString   transportTypes[MAX_ADDRESS_CANDIDATES];
         double      qValues[MAX_ADDRESS_CANDIDATES];
         UtlString   candidateIps[MAX_ADDRESS_CANDIDATES];
         int         candidatePorts[MAX_ADDRESS_CANDIDATES];
         int         nCandidates = 0;

         // Check for / add audio candidate addresses
         if (pRemoteBody->getCandidateAttributes(SDP_AUDIO_MEDIA_TYPE,
            MAX_ADDRESS_CANDIDATES,
            candidateIds,
            transportIds,
            transportTypes,
            qValues,
            candidateIps,
            candidatePorts,
            nCandidates))
         {
            bSetDestination = TRUE;

            int lastId = -1;
            for (int i=0; i<nCandidates; i++)
            {
               if (transportTypes[i].compareTo("UDP") == 0)
               {
                  if (candidateIds[i] != lastId)
                  {
                     if (pMediaInterface->addAudioRtpConnectionDestination(
                        mediaConnectionId,
                        (int) (qValues[i] * 100),
                        candidateIps[i],
                        candidatePorts[i]) != OS_SUCCESS)
                     {
                        OsSysLog::add(FAC_NET, PRI_ERR,
                           "Failed to set audio rtp media destination (%d %s %s:%d)",
                           candidateIds[i], transportIds[i].data(),
                           candidateIps[i].data(), candidatePorts[i]);

                     }
                  }
                  else
                  {
                     if (pMediaInterface->addAudioRtcpConnectionDestination(
                        mediaConnectionId,
                        (int) (qValues[i] * 100),
                        candidateIps[i],
                        candidatePorts[i]) != OS_SUCCESS)
                     {
                        OsSysLog::add(FAC_NET, PRI_ERR,
                           "Failed to set audio rtcp media destination (%d %s %s:%d)",
                           candidateIds[i], transportIds[i].data(),
                           candidateIps[i].data(), candidatePorts[i]);
                     }
                  }
                  lastId = candidateIds[i];
               }
            }
         }

         // Check for / add video candidate addresses
         if (pRemoteBody->getCandidateAttributes(SDP_VIDEO_MEDIA_TYPE,
            MAX_ADDRESS_CANDIDATES,
            candidateIds,
            transportIds,
            transportTypes,
            qValues,
            candidateIps,
            candidatePorts,
            nCandidates))
         {
            bSetDestination = TRUE;

            int lastId = -1;
            for (int i=0; i<nCandidates; i++)
            {
               if (transportTypes[i].compareTo("UDP") == 0)
               {
                  if (candidateIds[i] != lastId)
                  {
                     if (pMediaInterface->addVideoRtpConnectionDestination(
                        mediaConnectionId,
                        (int) (qValues[i] * 100),
                        candidateIps[i],
                        candidatePorts[i]) != OS_SUCCESS)
                     {
                        OsSysLog::add(FAC_NET, PRI_ERR,
                           "Failed to set video rtp media destination (%d %s %s:%d)",
                           candidateIds[i], transportIds[i].data(),
                           candidateIps[i].data(), candidatePorts[i]);

                     }
                  }
                  else
                  {
                     if (pMediaInterface->addVideoRtcpConnectionDestination(
                        mediaConnectionId,
                        (int) (qValues[i] * 100),
                        candidateIps[i],
                        candidatePorts[i]) != OS_SUCCESS)
                     {
                        OsSysLog::add(FAC_NET, PRI_ERR,
                           "Failed to set video rtcp media destination (%d %s %s:%d)",
                           candidateIds[i], transportIds[i].data(),
                           candidateIps[i].data(), candidatePorts[i]);
                     }
                  }
                  lastId = candidateIds[i];
               }
            }
         }
      }

      if (!bSetDestination)
      {
         pMediaInterface->setConnectionDestination(mediaConnectionId,
            hostAddress,
            audioRtpPort,
            audioRtcpPort,
            videoRtpPort,
            videoRtcpPort);
      }
   }
}

SipConnectionStateTransition* BaseSipConnectionState::doByeConnection(OsStatus& result)
{
   // if we are inbound call, sent 200 OK but haven't received ACK yet, we may not do BYE
   if (!m_rStateContext.m_bCancelSent)
   {
      sendBye();
   }
   result = OS_SUCCESS;
   return NULL;
}

SipConnectionStateTransition* BaseSipConnectionState::doCancelConnection(OsStatus& result)
{
   sendInviteCancel();
   result = OS_SUCCESS;
   return NULL;
}

void BaseSipConnectionState::requestConnectionDestruction()
{
   // send message to call that we want to destroy the XSipConnection
   SipDialog sipDialog;
   {
      OsReadLock lock(m_rStateContext);
      sipDialog = m_rStateContext.m_sipDialog;
   }
   AcDestroyConnectionMsg msg(sipDialog);
   m_pMessageQueueProvider->getLocalQueue().send(msg);
}

void BaseSipConnectionState::startCancelTimeoutTimer()
{
   deleteCancelTimeoutTimer();

   UtlString sipCallId;
   UtlString localTag;
   UtlString remoteTag;
   UtlBoolean isFromLocal;

   getSipDialogId(sipCallId, localTag, remoteTag, isFromLocal);
   ScDisconnectTimerMsg msg(ScDisconnectTimerMsg::REASON_CANCEL_TIMEOUT, sipCallId, localTag, remoteTag, isFromLocal);
   OsTimerNotification* pNotification = new OsTimerNotification(m_pMessageQueueProvider->getLocalQueue(), msg);
   m_rStateContext.m_pCancelTimeoutTimer = new OsTimer(pNotification);
   int sipTransactionTimeout = m_rSipUserAgent.getSipStateTransactionTimeout();
   OsTime timerTime(T1_PERIOD_MSEC * 64);
   m_rStateContext.m_pCancelTimeoutTimer->oneshotAfter(timerTime); // start timer
}

void BaseSipConnectionState::deleteCancelTimeoutTimer()
{
   if (m_rStateContext.m_pCancelTimeoutTimer)
   {
      delete m_rStateContext.m_pCancelTimeoutTimer;
      m_rStateContext.m_pCancelTimeoutTimer = NULL;
   }
}

void BaseSipConnectionState::startByeTimeoutTimer()
{
   deleteByeTimeoutTimer();

   UtlString sipCallId;
   UtlString localTag;
   UtlString remoteTag;
   UtlBoolean isFromLocal;

   getSipDialogId(sipCallId, localTag, remoteTag, isFromLocal);
   ScDisconnectTimerMsg msg(ScDisconnectTimerMsg::REASON_BYE_TIMEOUT, sipCallId, localTag, remoteTag, isFromLocal);
   OsTimerNotification* pNotification = new OsTimerNotification(m_pMessageQueueProvider->getLocalQueue(), msg);
   m_rStateContext.m_pByeTimeoutTimer = new OsTimer(pNotification);
   int sipTransactionTimeoutMs = m_rSipUserAgent.getSipStateTransactionTimeout();
   OsTime timerTime(sipTransactionTimeoutMs);
   m_rStateContext.m_pByeTimeoutTimer->oneshotAfter(timerTime); // start timer
}

void BaseSipConnectionState::deleteByeTimeoutTimer()
{
   if (m_rStateContext.m_pByeTimeoutTimer)
   {
      delete m_rStateContext.m_pByeTimeoutTimer;
      m_rStateContext.m_pByeTimeoutTimer = NULL;
   }
}

void BaseSipConnectionState::startByeRetryTimer()
{
   deleteByeRetryTimer();

   UtlString sipCallId;
   UtlString localTag;
   UtlString remoteTag;
   UtlBoolean isFromLocal;

   getSipDialogId(sipCallId, localTag, remoteTag, isFromLocal);
   ScByeRetryTimerMsg msg(sipCallId, localTag, remoteTag, isFromLocal);
   OsTimerNotification* pNotification = new OsTimerNotification(m_pMessageQueueProvider->getLocalQueue(), msg);
   m_rStateContext.m_pByeRetryTimer = new OsTimer(pNotification);
   OsTime timerTime(1, 0); // try BYE in 1 second again
   m_rStateContext.m_pByeRetryTimer->oneshotAfter(timerTime); // start timer
}

void BaseSipConnectionState::deleteByeRetryTimer()
{
   if (m_rStateContext.m_pByeRetryTimer)
   {
      delete m_rStateContext.m_pByeRetryTimer;
      m_rStateContext.m_pByeRetryTimer = NULL;
   }
}

void BaseSipConnectionState::start2xxRetransmitTimer()
{
   delete2xxRetransmitTimer();

   UtlString sipCallId;
   UtlString localTag;
   UtlString remoteTag;
   UtlBoolean isFromLocal;

   getSipDialogId(sipCallId, localTag, remoteTag, isFromLocal);
   Sc2xxTimerMsg msg(sipCallId, localTag, remoteTag, isFromLocal);
   OsTimerNotification* pNotification = new OsTimerNotification(m_pMessageQueueProvider->getLocalQueue(), msg);
   m_rStateContext.m_p2xxInviteRetransmitTimer = new OsTimer(pNotification);
   OsTime timerTime(max(T1_PERIOD_MSEC * (m_rStateContext.m_i2xxInviteRetransmitCount + 1), T2_PERIOD_MSEC));
   m_rStateContext.m_p2xxInviteRetransmitTimer->oneshotAfter(timerTime); // start timer
}

void BaseSipConnectionState::delete2xxRetransmitTimer()
{
   if (m_rStateContext.m_p2xxInviteRetransmitTimer)
   {
      delete m_rStateContext.m_p2xxInviteRetransmitTimer;
      m_rStateContext.m_p2xxInviteRetransmitTimer = NULL;
   }
}

void BaseSipConnectionState::startSessionTimeoutCheckTimer()
{
   deleteSessionTimeoutCheckTimer();

   UtlString sipCallId;
   UtlString localTag;
   UtlString remoteTag;
   UtlBoolean isFromLocal;

   getSipDialogId(sipCallId, localTag, remoteTag, isFromLocal);
   ScSessionTimeoutTimerMsg msg(sipCallId, localTag, remoteTag, isFromLocal);
   OsTimerNotification* pNotification = new OsTimerNotification(m_pMessageQueueProvider->getLocalQueue(), msg);
   m_rStateContext.m_pSessionTimeoutCheckTimer = new OsTimer(pNotification);
   OsTime timerTime(m_rStateContext.m_sessionTimerProperties.getSessionExpires() / 2, 0);
   m_rStateContext.m_pSessionTimeoutCheckTimer->oneshotAfter(timerTime); // start timer
}

void BaseSipConnectionState::deleteSessionTimeoutCheckTimer()
{
   if (m_rStateContext.m_pSessionTimeoutCheckTimer)
   {
      delete m_rStateContext.m_pSessionTimeoutCheckTimer;
      m_rStateContext.m_pSessionTimeoutCheckTimer = NULL;
   }
}

void BaseSipConnectionState::startSessionRefreshTimer()
{
   deleteSessionRefreshTimer();

   UtlString sipCallId;
   UtlString localTag;
   UtlString remoteTag;
   UtlBoolean isFromLocal;

   getSipDialogId(sipCallId, localTag, remoteTag, isFromLocal);
   ScReInviteTimerMsg msg(ScReInviteTimerMsg::REASON_SESSION_EXTENSION, sipCallId, localTag, remoteTag, isFromLocal);
   OsTimerNotification* pNotification = new OsTimerNotification(m_pMessageQueueProvider->getLocalQueue(), msg);
   m_rStateContext.m_pSessionRefreshTimer = new OsTimer(pNotification);
   OsTime timerTime(m_rStateContext.m_sessionTimerProperties.getSessionExpires() / 2, 0); // refresh in the middle
   m_rStateContext.m_pSessionRefreshTimer->oneshotAfter(timerTime); // start timer
}

void BaseSipConnectionState::deleteSessionRefreshTimer()
{
   if (m_rStateContext.m_pSessionRefreshTimer)
   {
      delete m_rStateContext.m_pSessionRefreshTimer;
      m_rStateContext.m_pSessionRefreshTimer = NULL;
   }
}

void BaseSipConnectionState::startInviteExpirationTimer(int timeoutSec, int cseqNum, UtlBoolean bIsOutbound)
{
   deleteInviteExpirationTimer();

   UtlString sipCallId;
   UtlString localTag;
   UtlString remoteTag;
   UtlBoolean isFromLocal;

   getSipDialogId(sipCallId, localTag, remoteTag, isFromLocal);
   ScInviteExpirationTimerMsg msg(cseqNum, bIsOutbound, sipCallId, localTag, remoteTag, isFromLocal);
   OsTimerNotification* pNotification = new OsTimerNotification(m_pMessageQueueProvider->getLocalQueue(), msg);
   m_rStateContext.m_pInviteExpiresTimer = new OsTimer(pNotification);
   OsTime timerTime(timeoutSec, 0);
   m_rStateContext.m_pInviteExpiresTimer->oneshotAfter(timerTime); // start timer
}

void BaseSipConnectionState::deleteInviteExpirationTimer()
{
   if (m_rStateContext.m_pInviteExpiresTimer)
   {
      delete m_rStateContext.m_pInviteExpiresTimer;
      m_rStateContext.m_pInviteExpiresTimer = NULL;
   }
}

void BaseSipConnectionState::start100relRetransmitTimer(const SipMessage& c100relResponse)
{
   delete100relRetransmitTimer();

   UtlString sipCallId;
   UtlString localTag;
   UtlString remoteTag;
   UtlBoolean isFromLocal;

   getSipDialogId(sipCallId, localTag, remoteTag, isFromLocal);
   Sc100RelTimerMsg msg(c100relResponse, sipCallId, localTag, remoteTag, isFromLocal);
   OsTimerNotification* pNotification = new OsTimerNotification(m_pMessageQueueProvider->getLocalQueue(), msg);
   m_rStateContext.m_p100relRetransmitTimer = new OsTimer(pNotification);
   OsTime timerTime(T1_PERIOD_MSEC, 0);
   m_rStateContext.m_p100relRetransmitTimer->oneshotAfter(timerTime); // start timer
}

void BaseSipConnectionState::delete100relRetransmitTimer()
{
   if (m_rStateContext.m_p100relRetransmitTimer)
   {
      delete m_rStateContext.m_p100relRetransmitTimer;
      m_rStateContext.m_p100relRetransmitTimer = NULL;
   }
}

void BaseSipConnectionState::startDelayedAnswerTimer()
{
   deleteDelayedAnswerTimer();

   UtlString sipCallId;
   UtlString localTag;
   UtlString remoteTag;
   UtlBoolean isFromLocal;

   getSipDialogId(sipCallId, localTag, remoteTag, isFromLocal);
   ScDelayedAnswerTimerMsg msg(sipCallId, localTag, remoteTag, isFromLocal);
   OsTimerNotification* pNotification = new OsTimerNotification(m_pMessageQueueProvider->getLocalQueue(), msg);
   m_rStateContext.m_pDelayedAnswerTimer = new OsTimer(pNotification);
   OsTime timerTime(T1_PERIOD_MSEC);
   m_rStateContext.m_pDelayedAnswerTimer->oneshotAfter(timerTime); // start timer
}

void BaseSipConnectionState::deleteDelayedAnswerTimer()
{
   if (m_rStateContext.m_pDelayedAnswerTimer)
   {
      delete m_rStateContext.m_pDelayedAnswerTimer;
      m_rStateContext.m_pDelayedAnswerTimer = NULL;
   }
}

void BaseSipConnectionState::getSipDialogId(UtlString& sipCallId,
                                            UtlString& localTag,
                                            UtlString& remoteTag,
                                            UtlBoolean& isFromLocal)
{
   {
      OsReadLock lock(m_rStateContext);
      m_rStateContext.m_sipDialog.getCallId(sipCallId);
      m_rStateContext.m_sipDialog.getLocalTag(localTag);
      m_rStateContext.m_sipDialog.getRemoteTag(remoteTag);
      isFromLocal = m_rStateContext.m_sipDialog.isLocalInitiatedDialog();
   }
}

int BaseSipConnectionState::getNextLocalCSeq()
{
   OsWriteLock lock(m_rStateContext);
   return m_rStateContext.m_sipDialog.getNextLocalCseq();
}

int BaseSipConnectionState::getRandomCSeq() const
{
   UtlRandom randomGenerator;
   return (abs(randomGenerator.rand()) % 65535);
}

void BaseSipConnectionState::trackTransactionRequest(const SipMessage& sipMessage)
{
   int cseqNum;
   UtlString cseqMethod;
   sipMessage.getCSeqField(cseqNum, cseqMethod);

   CpSipTransactionManager& transactionManager = sipMessage.isFromThisSide() ? getClientTransactionManager() : getServerTransactionManager();

   if (needsTransactionTracking(cseqMethod))
   {
      CpSipTransactionManager::TransactionState transactionState = transactionManager.getTransactionState(cseqNum);

      if (transactionState == CpSipTransactionManager::TRANSACTION_NOT_FOUND)
      {
         // not found, start new transaction
         if (cseqMethod.compareTo(SIP_INVITE_METHOD) == 0)
         {
            ISipConnectionState::StateEnum connectionState = getCurrentState();
            if (connectionState == ISipConnectionState::CONNECTION_ESTABLISHED)
            {
               // this must be a re-INVITE
               transactionManager.startReInviteTransaction(cseqNum);
            }
            else
            {
               // this must be the initial INVITE
               transactionManager.startInitialInviteTransaction(cseqNum);
            }
         }
         else
         {
            transactionManager.startTransaction(cseqMethod, cseqNum);
         }
      }
   }

   if (cseqMethod.compareTo(SIP_ACK_METHOD) == 0)
   {
      // inbound ACK terminates some inbound INVITE transaction, outbound ACK terminates outbound INVITE transaction
      // We deliberately end INVITE transaction with 2xx response with ACK, and not 2xx response
      // This has an advantage for inbound INVITE transactions, because we terminate only once we receive ACK for 2xx
      // that we sent. That means we can be sure our 2xx was received and not lost. Then we can proceed sending our
      // INVITE, and shouldn't get 491 Request Pending. If we terminated transaction after sending 2xx, then we could
      // try send new re-INVITE, but 2xx could have been lost, and we would get 491.

      transactionManager.endTransaction(SIP_INVITE_METHOD, cseqNum);
   }

   updateSipDialog(sipMessage);
}

void BaseSipConnectionState::trackTransactionResponse(const SipMessage& sipMessage)
{
   int cseqNum;
   UtlString cseqMethod;
   sipMessage.getCSeqField(cseqNum, cseqMethod);
   int statusCode = sipMessage.getResponseStatusCode();

   if (needsTransactionTracking(cseqMethod))
   {
      CpSipTransactionManager& transactionManager = sipMessage.isFromThisSide() ? getServerTransactionManager() : getClientTransactionManager();

      if (cseqMethod.compareTo(SIP_INVITE_METHOD) == 0 &&
          statusCode >= SIP_2XX_CLASS_CODE && statusCode < SIP_3XX_CLASS_CODE)
      {
         // INVITE transaction ends with ACK for 2xx code, since it is us who sends the ACK
         // therefore do nothing here. This is deliberate, read explanation in trackTransactionRequest.
      }
      else if (statusCode >= SIP_2XX_CLASS_CODE)
      {
         // also handles INVITE 3xx and greater responses - terminate transaction now, 
         // since ACK is sent automatically by transaction layer and we will never see it
         UtlBoolean res = transactionManager.endTransaction(cseqMethod, cseqNum);

         if (cseqMethod.compareTo(SIP_INVITE_METHOD) == 0 &&
             statusCode >= SIP_3XX_CLASS_CODE &&
             m_rStateContext.m_sdpNegotiation.isInSdpNegotiation(sipMessage))
         {
            // INVITE transaction error response, that is in SDP negotiation, reset SDP negotiation
            m_rStateContext.m_sdpNegotiation.resetSdpNegotiation();
         }
      }
      // 1xx don't end transaction
   }

   updateSipDialog(sipMessage);
}

UtlBoolean BaseSipConnectionState::verifyInboundRequest(const SipMessage& sipRequest)
{
   // sipMessage is inbound request

   int lastRemoteCseqNum = -1;
   {
      OsReadLock lock(m_rStateContext);
      lastRemoteCseqNum = m_rStateContext.m_sipDialog.getLastRemoteCseq();
   }
   int seqNum = -1;
   UtlString seqMethod;
   sipRequest.getCSeqField(&seqNum, &seqMethod);

   if (seqMethod.compareTo(SIP_ACK_METHOD) != 0 && seqMethod.compareTo(SIP_CANCEL_METHOD) != 0)
   {
      // method is not ACK or CANCEL, seqNum must be monotonous
      if (seqNum < lastRemoteCseqNum)
      {
         CpSipTransactionManager::TransactionState transactionState = getServerTransactionManager().getTransactionState(sipRequest);
         if (transactionState != CpSipTransactionManager::TRANSACTION_TERMINATED)
         {
            // this is an error
            SipMessage sipResponse;
            sipResponse.setResponseData(&sipRequest, SIP_SERVER_INTERNAL_ERROR_CODE, SIP_SERVER_INTERNAL_ERROR_TEXT);
            sendMessage(sipResponse);
            return FALSE;
         } // if transaction is terminated, then this is a retransmit (response was lost), allow it
      }
   }

   if (m_rStateContext.m_loopDetector.isInboundMessageLoop(sipRequest))
   {
      if (seqMethod.compareTo(SIP_ACK_METHOD) != 0)
      {
         SipMessage sipResponse;
         sipResponse.setResponseData(&sipRequest, SIP_LOOP_DETECTED_CODE, SIP_LOOP_DETECTED_TEXT);
         sendMessage(sipResponse);
         return FALSE;
      }
      else
      {
         return FALSE;
      }
   }

   // add additional checks here if needed
   return TRUE;
}

CpSipTransactionManager::InviteTransactionState BaseSipConnectionState::getInviteTransactionState() const
{
   CpSipTransactionManager::InviteTransactionState inviteState = getClientInviteTransactionState();
   if (inviteState == CpSipTransactionManager::INVITE_INACTIVE)
   {
      inviteState = getServerInviteTransactionState();
   }

   return inviteState;
}

CpSipTransactionManager::InviteTransactionState BaseSipConnectionState::getServerInviteTransactionState() const
{
   CpSipTransactionManager::InviteTransactionState inviteState = getServerTransactionManager().getInviteTransactionState();
   return inviteState;
}

CpSipTransactionManager::InviteTransactionState BaseSipConnectionState::getClientInviteTransactionState() const
{
   CpSipTransactionManager::InviteTransactionState inviteState = getClientTransactionManager().getInviteTransactionState();
   return inviteState;
}

void BaseSipConnectionState::startSessionRenegotiationTimer(ScReInviteTimerMsg::ReInviteReason reason,
                                                            UtlBoolean bCleanStart)
{
   deleteSessionRenegotiationTimer();

   UtlString sipCallId;
   UtlString localTag;
   UtlString remoteTag;
   UtlBoolean isFromLocal;

   getSipDialogId(sipCallId, localTag, remoteTag, isFromLocal);
   ScReInviteTimerMsg msg(reason, sipCallId, localTag, remoteTag, isFromLocal);
   OsTimerNotification* pNotification = new OsTimerNotification(m_pMessageQueueProvider->getLocalQueue(), msg);
   m_rStateContext.m_pSessionRenegotiationTimer = new OsTimer(pNotification);
   OsTime timerTime(T1_PERIOD_MSEC); // try again in 500ms
   m_rStateContext.m_pSessionRenegotiationTimer->oneshotAfter(timerTime); // start timer

   if (bCleanStart)
   {
      m_rStateContext.m_iRenegotiationRetryCount = 0;
   }
   else
   {
      m_rStateContext.m_iRenegotiationRetryCount++;
   }
}

void BaseSipConnectionState::deleteSessionRenegotiationTimer()
{
   if (m_rStateContext.m_pSessionRenegotiationTimer)
   {
      delete m_rStateContext.m_pSessionRenegotiationTimer;
      m_rStateContext.m_pSessionRenegotiationTimer = NULL;
   }
}

UtlBoolean BaseSipConnectionState::isUpdateActive()
{
   return isOutboundUpdateActive() || isInboundUpdateActive();
}

UtlBoolean BaseSipConnectionState::isOutboundUpdateActive()
{
   int updateCount = getClientTransactionManager().getTransactionCount(SIP_UPDATE_METHOD);
   return updateCount != 0;
}

UtlBoolean BaseSipConnectionState::isInboundUpdateActive()
{
   int updateCount = getServerTransactionManager().getTransactionCount(SIP_UPDATE_METHOD);
   return updateCount != 0;
}

UtlBoolean BaseSipConnectionState::mayRenegotiateMediaSession()
{
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState != ISipConnectionState::CONNECTION_DISCONNECTED &&
       connectionState != ISipConnectionState::CONNECTION_UNKNOWN)
   {
      CpSipTransactionManager::InviteTransactionState inviteState = getInviteTransactionState();
      if (inviteState == CpSipTransactionManager::INVITE_INACTIVE)
      {
         // no invite is running, check for UPDATE
         if (!isUpdateActive())
         {
            // also no UPDATE, we may renegotiate
            return TRUE;
         }
      }
   }

   return FALSE;
}

void BaseSipConnectionState::doHold()
{
   // start hold via re-INVITE
   m_rStateContext.m_bUseLocalHoldSDP = TRUE;
   refreshSession(TRUE);
}

void BaseSipConnectionState::doUnhold()
{
   // start unhold via re-INVITE
   m_rStateContext.m_bUseLocalHoldSDP = FALSE;
   refreshSession(TRUE);
}

void BaseSipConnectionState::refreshSession(UtlBoolean bRenegotiateCodecs)
{
   if (m_rStateContext.m_updateSetting == CP_SIP_UPDATE_BOTH &&
       isMethodAllowed(SIP_UPDATE_METHOD))
   {
      sendUpdate(bRenegotiateCodecs);
   }
   else
   {
      // re-INVITE always renegotiates codecs
      sendInvite();
   }
}

void BaseSipConnectionState::setLocalMediaConnectionState(SipConnectionStateContext::MediaConnectionState state,
                                                          UtlBoolean bRefreshMediaSessionState)
{
   m_rStateContext.m_localMediaConnectionState = state;
   if (bRefreshMediaSessionState)
   {
      refreshMediaSessionState();
   }
}

void BaseSipConnectionState::setRemoteMediaConnectionState(SipConnectionStateContext::MediaConnectionState state,
                                                           UtlBoolean bRefreshMediaSessionState)
{
   m_rStateContext.m_remoteMediaConnectionState = state;
   if (bRefreshMediaSessionState)
   {
      refreshMediaSessionState();
   }
}

void BaseSipConnectionState::refreshMediaSessionState()
{
   SipConnectionStateContext::MediaConnectionState localState = m_rStateContext.m_localMediaConnectionState;
   SipConnectionStateContext::MediaConnectionState remoteState = m_rStateContext.m_remoteMediaConnectionState;

   if (localState == SipConnectionStateContext::MEDIA_CONNECTION_NONE &&
       remoteState == SipConnectionStateContext::MEDIA_CONNECTION_NONE)
   {
      // session is not active
      m_rStateContext.m_mediaSessionState = SipConnectionStateContext::MEDIA_SESSION_NONE;
   }
   else if (localState == SipConnectionStateContext::MEDIA_CONNECTION_HELD &&
            remoteState == SipConnectionStateContext::MEDIA_CONNECTION_ACTIVE)
   {
      // local held, remote active -> local hold
      m_rStateContext.m_mediaSessionState = SipConnectionStateContext::MEDIA_SESSION_LOCALLY_HELD;
   }
   else if (localState == SipConnectionStateContext::MEDIA_CONNECTION_ACTIVE &&
            remoteState == SipConnectionStateContext::MEDIA_CONNECTION_HELD)
   {
      // local active, remote held -> remote hold
      m_rStateContext.m_mediaSessionState = SipConnectionStateContext::MEDIA_SESSION_REMOTELY_HELD;
   }
   else if (localState == SipConnectionStateContext::MEDIA_CONNECTION_HELD &&
            remoteState == SipConnectionStateContext::MEDIA_CONNECTION_HELD)
   {
      // local held, remote held -> full hold
      m_rStateContext.m_mediaSessionState = SipConnectionStateContext::MEDIA_SESSION_FULLY_HELD;
   }
   else if (localState == SipConnectionStateContext::MEDIA_CONNECTION_ACTIVE ||
            remoteState == SipConnectionStateContext::MEDIA_CONNECTION_ACTIVE)
   {
      // one of connections is active -> session is active
      m_rStateContext.m_mediaSessionState = SipConnectionStateContext::MEDIA_SESSION_ACTIVE;
   }
   else
   {
      m_rStateContext.m_mediaSessionState = SipConnectionStateContext::MEDIA_SESSION_NONE;
   }

   ISipConnectionState::StateEnum connectionState = getCurrentState();
   if (connectionState == ISipConnectionState::CONNECTION_ESTABLISHED)
   {
      fireMediaSessionEvents(FALSE, FALSE);
   }

   m_rStateContext.m_previousMediaSessionState = m_rStateContext.m_mediaSessionState;
}

void BaseSipConnectionState::fireMediaSessionEvents(UtlBoolean bForce, UtlBoolean bSupressConnected)
{
   if ((m_rStateContext.m_previousMediaSessionState != m_rStateContext.m_mediaSessionState) ||
       bForce)
   {
      // if established then also fire event for hold/remote hold
      switch (m_rStateContext.m_mediaSessionState)
      {
      case SipConnectionStateContext::MEDIA_SESSION_ACTIVE:
         if (!bSupressConnected)
         {
            m_rSipConnectionEventSink.fireSipXCallEvent(CP_CALLSTATE_CONNECTED, CP_CALLSTATE_CAUSE_NORMAL);
         }
         break;
      case SipConnectionStateContext::MEDIA_SESSION_REMOTELY_HELD:
         m_rSipConnectionEventSink.fireSipXCallEvent(CP_CALLSTATE_REMOTE_HELD, CP_CALLSTATE_CAUSE_NORMAL);
         break;
      case SipConnectionStateContext::MEDIA_SESSION_LOCALLY_HELD:
      case SipConnectionStateContext::MEDIA_SESSION_FULLY_HELD:
         m_rSipConnectionEventSink.fireSipXCallEvent(CP_CALLSTATE_HELD, CP_CALLSTATE_CAUSE_NORMAL);
         break;
      case SipConnectionStateContext::MEDIA_SESSION_NONE:
      default:
         // don't fire events
         break;
      }
   }
}

void BaseSipConnectionState::prepareSipRequest(SipMessage& sipRequest, const UtlString& method, int cseqNum)
{
   OsWriteLock lock(m_rStateContext);
   m_rStateContext.m_sipDialog.setRequestData(sipRequest, method, cseqNum);
}

SipConnectionStateTransition* BaseSipConnectionState::handleInviteRedirectResponse(const SipMessage& sipMessage)
{
   int responseCode = sipMessage.getResponseStatusCode();
   UtlString responseText;
   sipMessage.getResponseStatusText(&responseText);

   UtlString contactUri;
   int index = 0;

   if (!m_rStateContext.m_bRedirecting)
   {
      // go through all contacts only if we are not already redirected - to prevent infinite loops
      while (sipMessage.getContactUri(index++, &contactUri))
      {
         m_rStateContext.m_redirectContactList.append(contactUri.clone()); // append duplicate uri
      }
   }

   if (m_rStateContext.m_redirectContactList.entries() > 0)
   {
      // we are redirecting
      m_rStateContext.m_bRedirecting = TRUE;
      return followNextRedirect();
   }
   else
   {
      // no contacts present, give up
      SipResponseTransitionMemory memory(responseCode, responseText);
      return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, &memory);
   }
}

SipConnectionStateTransition* BaseSipConnectionState::followNextRedirect()
{
   if (m_rStateContext.m_redirectContactList.entries() > 0)
   {
      Url fromField;
      Url toField;
      UtlString sipCallId;
      UtlString contactUrl(getLocalContactUrl());
      // get next contact, and construct new INVITE
      {
         OsReadLock lock(m_rStateContext);
         m_rStateContext.m_sipDialog.getLocalField(fromField);
         m_rStateContext.m_sipDialog.getRemoteField(toField);
         m_rStateContext.m_sipDialog.getCallId(sipCallId);
      }
      // keep tag of from field, but remove it from toField
      toField.removeFieldParameters();
      int cseqNum = getRandomCSeq();

      resetRemoteCapabilities();
      m_rStateContext.m_sessionTimerProperties.reset(TRUE); // reset refresher and session expiration

      SipMessage sipInvite;
      sipInvite.setInviteData(fromField.toString(), toField.toString(),
         NULL, contactUrl, sipCallId,
         cseqNum);
      prepareSessionTimerRequest(sipInvite);

      UtlString* pRedirectContactUri = dynamic_cast<UtlString*>(m_rStateContext.m_redirectContactList.get());
      if (pRedirectContactUri)
      {
         sipInvite.changeUri(*pRedirectContactUri); // for redirect, change just the URI, not toField

         initializeSipDialog(sipInvite); // restart sip dialog

         // add SDP if negotiation mode is immediate, otherwise don't add it
         if (m_rStateContext.m_sdpNegotiation.getSdpOfferingMode() == CpSdpNegotiation::SDP_OFFERING_IMMEDIATE)
         {
            if (!prepareSdpOffer(sipInvite))
            {
               // SDP negotiation start failed
               // media connection creation failed
               GeneralTransitionMemory memory(CP_CALLSTATE_CAUSE_RESOURCE_LIMIT);
               return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, &memory);
            }
         }

         // try to send sip message
         UtlBoolean sendSuccess = sendMessage(sipInvite);
         if (!sendSuccess)
         {
            GeneralTransitionMemory memory(CP_CALLSTATE_CAUSE_NETWORK);
            return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, &memory);
         }
      }
   }

   // no contacts present, give up
   return getTransition(ISipConnectionState::CONNECTION_DISCONNECTED, NULL);
}

UtlString BaseSipConnectionState::getSipCallId() const
{
   OsReadLock lock(m_rStateContext);
   return m_rStateContext.m_sipDialog.getCallId();
}

void BaseSipConnectionState::handleSessionTimerResponse(const SipMessage& sipResponse)
{
   // handles INVITE or UPDATE 2xx response, which may have session timer negotiation response
   int responseCode = sipResponse.getResponseStatusCode();
   int sessionExpiresSeconds;
   UtlString refresher;
   int minSe;

   if (responseCode >= SIP_2XX_CLASS_CODE && responseCode < SIP_3XX_CLASS_CODE)
   {
      if (sipResponse.getMinSe(minSe))
      {
         m_rStateContext.m_sessionTimerProperties.setMinSessionExpires(minSe);
      }
      if (sipResponse.getSessionExpires(&sessionExpiresSeconds, &refresher))
      {
         // Session-Expires field was present
         m_rStateContext.m_sessionTimerProperties.setSessionExpires(sessionExpiresSeconds);
         m_rStateContext.m_sessionTimerProperties.configureRefresher(refresher, !sipResponse.isFromThisSide());
      }

      m_rStateContext.m_sessionTimerProperties.onSessionRefreshed();

      CP_SESSION_TIMER_REFRESH refresher = m_rStateContext.m_sessionTimerProperties.getRefresher();

      startSessionTimeoutCheckTimer();
      if (refresher != CP_SESSION_REFRESH_REMOTE)
      {
         // start session timeout timer, and session refresh timer
         startSessionRefreshTimer();
      }
      else
      {
         // remote side is refreshing
         deleteSessionRefreshTimer(); // refresher role changed, get rid of refresh timer
      }
   }
}

void BaseSipConnectionState::prepareSessionTimerResponse(const SipMessage& sipRequest, SipMessage& sipResponse)
{
   int sessionExpiresSeconds;
   UtlString refresher;
   int minSe;
   UtlBoolean bSenderSupportsTimer = FALSE;

   // take original request, find out if it has refresher & refresh timeout set
   if (sipRequest.getMinSe(minSe))
   {
      // update our minSe
      m_rStateContext.m_sessionTimerProperties.setMinSessionExpires(minSe);
   }

   sipResponse.setMinSe(m_rStateContext.m_sessionTimerProperties.getMinSessionExpires());

   // maybe add Require: timer
   if (sipRequest.isInSupportedField(SIP_SESSION_TIMER_EXTENSION))
   {
      bSenderSupportsTimer = TRUE;
      sipResponse.addRequireExtension(SIP_SESSION_TIMER_EXTENSION);
   }

   // add Session-Expires header
   if (sipRequest.getSessionExpires(&sessionExpiresSeconds, &refresher))
   {
      m_rStateContext.m_sessionTimerProperties.setSessionExpires(sessionExpiresSeconds);
      m_rStateContext.m_sessionTimerProperties.configureRefresher(refresher, FALSE); // save refresher from request
   }
   else
   {
      if (bSenderSupportsTimer)
      {
         // save empty refresher. This allows us to select prefered configured refresher
         m_rStateContext.m_sessionTimerProperties.configureRefresher(NULL, FALSE);
      }
      else
      {
         // sender doesn't support session timer, so we choose ourselves (we don't want "uac", since
         // remote side would never refresh session)
         m_rStateContext.m_sessionTimerProperties.configureRefresher("uas", FALSE);
      }
   }
   sipResponse.setSessionExpires(m_rStateContext.m_sessionTimerProperties.getSessionExpires(),
      m_rStateContext.m_sessionTimerProperties.getRefresher(FALSE));
}

void BaseSipConnectionState::prepareSessionTimerRequest(SipMessage& sipRequest)
{
   sipRequest.setSessionExpires(m_rStateContext.m_sessionTimerProperties.getSessionExpires(),
      m_rStateContext.m_sessionTimerProperties.getRefresher(TRUE));
   sipRequest.setMinExpiresField(m_rStateContext.m_sessionTimerProperties.getMinSessionExpires());
}

void BaseSipConnectionState::prepareErrorResponse(const SipMessage& sipRequest, SipMessage& sipResponse, ERROR_RESPONSE_TYPE responseType) const
{
   UtlString contactUrl(getLocalContactUrl());

   switch (responseType)
   {
   case ERROR_RESPONSE_400:
      sipResponse.setRequestBadRequest(&sipRequest);
      break;
   case ERROR_RESPONSE_481:
      sipResponse.setBadTransactionData(&sipRequest);
      break;
   case ERROR_RESPONSE_487:
      sipResponse.setRequestTerminatedResponseData(&sipRequest);
      break;
   case ERROR_RESPONSE_488:
      sipResponse.setResponseData(&sipRequest, SIP_REQUEST_NOT_ACCEPTABLE_HERE_CODE,
         SIP_REQUEST_NOT_ACCEPTABLE_HERE_TEXT, contactUrl);
      break;
   case ERROR_RESPONSE_491:
      sipResponse.setRequestPendingData(&sipRequest);
      break;
   case ERROR_RESPONSE_500:
      sipResponse.setResponseData(&sipRequest, SIP_SERVER_INTERNAL_ERROR_CODE,
         SIP_SERVER_INTERNAL_ERROR_TEXT, contactUrl);
      break;
   case ERROR_RESPONSE_603:
      sipResponse.setResponseData(&sipRequest, SIP_DECLINE_CODE, SIP_DECLINE_TEXT, contactUrl);
      break;
   default:
      sipResponse.setResponseData(&sipRequest, SIP_SERVER_INTERNAL_ERROR_CODE,
         SIP_SERVER_INTERNAL_ERROR_TEXT, contactUrl);
   }
}

void BaseSipConnectionState::deleteAllTimers()
{
   delete m_rStateContext.m_pByeRetryTimer;
   m_rStateContext.m_pByeRetryTimer = NULL;
   delete m_rStateContext.m_pCancelTimeoutTimer;
   m_rStateContext.m_pCancelTimeoutTimer = NULL;
   delete m_rStateContext.m_pByeTimeoutTimer;
   m_rStateContext.m_pByeTimeoutTimer = NULL;
   delete m_rStateContext.m_pSessionRenegotiationTimer;
   m_rStateContext.m_pSessionRenegotiationTimer = NULL;
   delete m_rStateContext.m_p2xxInviteRetransmitTimer;
   m_rStateContext.m_p2xxInviteRetransmitTimer = NULL;
   delete m_rStateContext.m_pSessionTimeoutCheckTimer;
   m_rStateContext.m_pSessionTimeoutCheckTimer = NULL;
   delete m_rStateContext.m_pSessionRefreshTimer;
   m_rStateContext.m_pSessionRefreshTimer = NULL;
   delete m_rStateContext.m_pInviteExpiresTimer;
   m_rStateContext.m_pInviteExpiresTimer = NULL;
   delete m_rStateContext.m_p100relRetransmitTimer;
   m_rStateContext.m_p100relRetransmitTimer = NULL;
   delete m_rStateContext.m_pDelayedAnswerTimer;
   m_rStateContext.m_pDelayedAnswerTimer = NULL;
}

void BaseSipConnectionState::updateTimer(OsTimer **pTimer)
{
   if (pTimer && *pTimer)
   {
      (*pTimer)->stop();
      if (!(*pTimer)->getWasFired())
      {
         // timer has not fired yet, it needs to be restarted
         OsTime expiresAt;
         (*pTimer)->getExpiresAt(expiresAt);
         OsTimerNotification *pOldNotification = dynamic_cast<OsTimerNotification*>((*pTimer)->getNotifier());
         if (pOldNotification)
         {
            OsTimerNotification *pNewNotification = new OsTimerNotification(m_pMessageQueueProvider->getLocalQueue(),
               *pOldNotification->getOsTimerMsg());
            delete *pTimer;
            *pTimer = new OsTimer(pNewNotification);
            (*pTimer)->oneshotAt(expiresAt);
         }
         else
         {
            delete *pTimer;
            *pTimer = NULL;
         }
      } // else timer fired, cannot update it with new queue
   }
}

void BaseSipConnectionState::updateAllTimers()
{
   updateTimer(&m_rStateContext.m_pByeRetryTimer);
   updateTimer(&m_rStateContext.m_pCancelTimeoutTimer);
   updateTimer(&m_rStateContext.m_pByeTimeoutTimer);
   updateTimer(&m_rStateContext.m_pSessionRenegotiationTimer);
   updateTimer(&m_rStateContext.m_p2xxInviteRetransmitTimer);
   updateTimer(&m_rStateContext.m_pSessionTimeoutCheckTimer);
   updateTimer(&m_rStateContext.m_pSessionRefreshTimer);
   updateTimer(&m_rStateContext.m_pInviteExpiresTimer);
   updateTimer(&m_rStateContext.m_p100relRetransmitTimer);
   updateTimer(&m_rStateContext.m_pDelayedAnswerTimer);
}

void BaseSipConnectionState::updateRemoteCapabilities(const SipMessage& sipMessage)
{
   UtlString allowField;
   UtlBoolean allowPresent = sipMessage.getAllowField(allowField);
   UtlString supportedField;
   UtlBoolean supportedPresent = sipMessage.getSupportedField(supportedField);
   UtlBoolean updateCapabilities = FALSE;
   int seqNum;
   UtlString seqMethod;
   sipMessage.getCSeqField(seqNum, seqMethod);

   if (sipMessage.isResponse())
   {
      int responseCode = sipMessage.getResponseStatusCode();
      if (responseCode > SIP_1XX_CLASS_CODE && responseCode < SIP_3XX_CLASS_CODE)
      {
         updateCapabilities = TRUE;
      }
      if (seqMethod.compareTo(SIP_OPTIONS_METHOD) == 0)
      {
         // if we get OPTIONS response, assume we know all that we can discover.
         m_rStateContext.m_allowedRemoteDiscovered = TRUE;
         m_rStateContext.m_supportedRemoteDiscovered = TRUE;
      }
   }
   else
   {
      // sip request
      updateCapabilities = TRUE;
   }

   if (updateCapabilities)
   {
      if (allowPresent)
      {
         m_rStateContext.m_allowedRemote = allowField; // update Allow:
         m_rStateContext.m_allowedRemoteDiscovered = TRUE;
      }
      if (supportedPresent)
      {
         m_rStateContext.m_supportedRemote = supportedField; // update Supported:
         m_rStateContext.m_supportedRemoteDiscovered = TRUE;
      }
      if (!m_rStateContext.m_allowedRemoteDiscovered || !m_rStateContext.m_supportedRemoteDiscovered)
      {
         discoverRemoteCapabilities(); // check if we know remote allow
      }
   }
}

void BaseSipConnectionState::resetRemoteCapabilities()
{
   m_rStateContext.m_allowedRemote.remove(0);
   m_rStateContext.m_supportedRemote.remove(0);
   m_rStateContext.m_allowedRemoteDiscovered = FALSE;
   m_rStateContext.m_supportedRemoteDiscovered = FALSE;
}

void BaseSipConnectionState::maybeRequire100rel(SipMessage& sipRequest) const
{
   if (m_rStateContext.m_100relSetting == CP_100REL_REQUIRE_RELIABLE)
   {
      sipRequest.addRequireExtension(SIP_PRACK_EXTENSION);
   }
}

UtlBoolean BaseSipConnectionState::shouldSend100relResponse() const
{
   if (m_rStateContext.m_pLastReceivedInvite)
   {
      if (m_rStateContext.m_100relSetting == CP_100REL_REQUIRE_RELIABLE ||
         (m_rStateContext.m_100relSetting == CP_100REL_PREFER_RELIABLE && isExtensionSupported(SIP_PRACK_EXTENSION)) ||
         m_rStateContext.m_pLastReceivedInvite->isRequireExtensionSet(SIP_PRACK_EXTENSION))
      {
         return TRUE;
      }
   }

   return FALSE;
}

CpSdpNegotiation::SdpBodyType BaseSipConnectionState::getPrackSdpBodyType(const SipMessage& sipMessage) const
{
   CpSdpNegotiation::SdpBodyType bodyType = CpSdpNegotiation::SDP_BODY_NONE;
   const SdpBody* pSdpBody = sipMessage.getSdpBody(m_rStateContext.m_pSecurity);
   if (pSdpBody)
   {
      bodyType = CpSdpNegotiation::SDP_BODY_UNKNOWN; // there is some unknown SDP body type

      UtlString s100relId = Cp100RelTracker::get100RelId(sipMessage);
      if (m_rStateContext.m_100RelTracker.is100RelIdValid(s100relId))
      {
         UtlBoolean bSdpIn100rel = m_rStateContext.m_100RelTracker.wasSdpBodyIn100rel(s100relId);
         // try to find out what type of body it is
         if (m_rStateContext.m_pLastReceivedInvite &&
             m_rStateContext.m_pLastReceivedInvite->getSdpBody() != NULL)
         {
            // SDP offer was in INVITE
            if (bSdpIn100rel)
            {
               // SDP answer was in 100rel response
               // SDP body in PRACK must be an offer
               bodyType = CpSdpNegotiation::SDP_BODY_OFFER;
            } // else protocol error. Prack can only have SDP body if 1st SDP negotiation was finished
         }
         else
         {
            // no SDP offer in INVITE
            if (bSdpIn100rel)
            {
               // SDP offer was in 100rel
               bodyType = CpSdpNegotiation::SDP_BODY_ANSWER;
            }
            else
            {
               bodyType = CpSdpNegotiation::SDP_BODY_OFFER;
            }
         }
      }
   }

   return bodyType;
}

void BaseSipConnectionState::announceConnectedIdentity(SipMessage& sipRequest) const
{
   Url fromUrl;
   sipRequest.getFromUrl(fromUrl);

   if (m_rStateContext.m_connectedIdentityState == SipConnectionStateContext::IDENTITY_ANNOUNCING)
   {
      UtlString userId;
      UtlString hostAddress;
      int hostPort;
      UtlString realDisplayName;
      m_rStateContext.m_realLineIdentity.getUserId(userId);
      m_rStateContext.m_realLineIdentity.getHostAddress(hostAddress);
      hostPort = m_rStateContext.m_realLineIdentity.getHostPort();
      m_rStateContext.m_realLineIdentity.getDisplayName(realDisplayName);
      // update from Url
      fromUrl.setUserId(userId);
      fromUrl.setHostAddress(hostAddress);
      fromUrl.setHostPort(hostPort);
      fromUrl.setDisplayName(realDisplayName);
      sipRequest.setRawFromField(fromUrl.toString());
   }
}

void BaseSipConnectionState::onConnectedIdentityAccepted()
{
   m_rStateContext.m_connectedIdentityState = SipConnectionStateContext::IDENTITY_UP_TO_DATE;

   Url localUrl;
   {
      OsReadLock lock(m_rStateContext);
      m_rStateContext.m_sipDialog.getLocalField(localUrl);
   }

   UtlString userId;
   UtlString hostAddress;
   int hostPort;
   UtlString displayName;
   m_rStateContext.m_realLineIdentity.getUserId(userId);
   m_rStateContext.m_realLineIdentity.getHostAddress(hostAddress);
   hostPort = m_rStateContext.m_realLineIdentity.getHostPort();
   m_rStateContext.m_realLineIdentity.getDisplayName(displayName);
   // update local Url
   localUrl.setUserId(userId);
   localUrl.setHostAddress(hostAddress);
   localUrl.setHostPort(hostPort);
   localUrl.setDisplayName(displayName);
   // update sip dialog
   {
      OsWriteLock lock(m_rStateContext);
      m_rStateContext.m_sipDialog.setLocalField(localUrl);
   }
}

void BaseSipConnectionState::onConnectedIdentityRejected()
{
   m_rStateContext.m_connectedIdentityState = SipConnectionStateContext::IDENTITY_REJECTED;
}

void BaseSipConnectionState::updateSipDialogRemoteField(const SipMessage& sipRequest)
{
   Url fromUrl;
   sipRequest.getFromUrl(fromUrl);

   {
      OsWriteLock lock(m_rStateContext);
      m_rStateContext.m_sipDialog.setRemoteField(fromUrl);
   }
}

OsStatus BaseSipConnectionState::transferCall(const Url& sReferToSipUrl)
{
   OsStatus result = OS_FAILED;
   ISipConnectionState::StateEnum connectionState = getCurrentState();

   if (connectionState == ISipConnectionState::CONNECTION_ESTABLISHED &&
      m_rStateContext.m_localEntityType == SipConnectionStateContext::ENTITY_NORMAL)
   {
      // call is connected and there is no call transfer in progress
      SipMessage sipRefer;
      int seqNum = getNextLocalCSeq();
      prepareSipRequest(sipRefer, SIP_REFER_METHOD, seqNum);
      setLastSentRefer(sipRefer);
      m_rStateContext.m_localEntityType = SipConnectionStateContext::ENTITY_TRANSFER_CONTROLLER;
      m_rStateContext.m_subscriptionId.remove(0);
      Url referredByUrl;
      {
         OsReadLock lock(m_rStateContext);
         if (m_rStateContext.m_sipDialog.isLocalInitiatedDialog())
         {
            // outbound call
            m_rStateContext.m_sipDialog.getLocalField(referredByUrl);
         }
         else
         {
            // inbound call
            m_rStateContext.m_sipDialog.getLocalRequestUri(referredByUrl);
         }
      }
      referredByUrl.removeParameters(); // remove tag and any other field parameters
      referredByUrl.setDisplayName(NULL);
      referredByUrl.setPassword(NULL);
      referredByUrl.includeAngleBrackets();
      sipRefer.setReferredByField(referredByUrl.toString());
      sipRefer.setReferToField(sReferToSipUrl.toString());
      sendMessage(sipRefer);
      // fire event that we are transferring call
      m_rSipConnectionEventSink.fireSipXCallEvent(CP_CALLSTATE_TRANSFER_EVENT, CP_CALLSTATE_CAUSE_TRANSFER_INITIATED);
      result = OS_SUCCESS;
   }

   return result;
}

void BaseSipConnectionState::notifyConnectionStateObservers()
{
   const UtlSList* pDialogList = m_rStateContext.m_notificationRegister.getSubscribedDialogs(CP_NOTIFICATION_CONNECTION_STATE);
   if (pDialogList)
   {
      UtlSListIterator itor(*pDialogList);
      while (itor())
      {
         SipDialog* pDialog = dynamic_cast<SipDialog*>(itor.item());
         if (pDialog)
         {
            SipDialog currentSipDialog;
            {
               OsReadLock lock(m_rStateContext);
               currentSipDialog = m_rStateContext.m_sipDialog; // copy current sip dialog
            }
            ScConnStateNotificationMsg msg(getCurrentState(), *pDialog, currentSipDialog);
            if (m_rCallControl.sendMessage(msg, *pDialog) == OS_NOT_FOUND) // send message to subscriber
            {
               // call has terminated, unsubscribe it
               m_rStateContext.m_notificationRegister.unsubscribe(CP_NOTIFICATION_CONNECTION_STATE, *pDialog);
               pDialog = NULL;
            }
         }
      }
   }
}

UtlBoolean BaseSipConnectionState::followRefer(const SipMessage& sipRequest)
{
   // we have received REFER, verified it, and are ready to create new call to referenced sip uri
   OsStatus connectStatus = OS_FAILED;
   CP_FOCUS_CONFIG focusConfig = CP_FOCUS_IF_AVAILABLE;

   UtlString referTo;
   UtlString referredBy;
   sipRequest.getReferredByField(referredBy);
   sipRequest.getReferToField(referTo);
   Url referToUrl(referTo);
   UtlString replacesField;
   referToUrl.getHeaderParameter("Replaces", replacesField);
   referToUrl.setScheme(Url::SipUrlScheme); // override scheme
   referToUrl.removeHeaderParameters();
   referToUrl.removeFieldParameters(); // remove some parameters

   CpMediaInterface* pMediaInterface = m_pMediaInterfaceProvider->getMediaInterface(FALSE);
   if (pMediaInterface && pMediaInterface->hasFocus())
   {
      focusConfig = CP_FOCUS_ALWAYS; // if this call is in focus, then we also want new call to take focus
   }

   UtlString fromField;
   SipDialog currentSipDialog;

   if (isLocalInitiatedDialog())
   {
      OsReadLock lock(m_rStateContext);
      m_rStateContext.m_sipDialog.getLocalField(fromField);
      currentSipDialog = m_rStateContext.m_sipDialog;
   }
   else
   {
      Url localRequestUri;
      {
         OsReadLock lock(m_rStateContext);
         m_rStateContext.m_sipDialog.getLocalRequestUri(localRequestUri);
         currentSipDialog = m_rStateContext.m_sipDialog;
      }
      localRequestUri.toString(fromField);
   }

   connectStatus = m_rCallControl.createConnectedCall(m_rStateContext.m_transferSipDialog, referToUrl.toString(), fromField,
      NULL, m_rStateContext.m_locationHeader, m_rStateContext.m_contactId, m_rStateContext.m_transportType,
      focusConfig, replacesField, CP_CALLSTATE_CAUSE_TRANSFER,
      &currentSipDialog);

   if (connectStatus == OS_SUCCESS)
   {
      // we cease being transferee once created call disconnects (for example rejected), to allow another transfer
      // this call will be disconnected when created call is established
      return TRUE;
   }

   return FALSE;
}

UtlBoolean BaseSipConnectionState::getReferNotifyCode(ISipConnectionState::StateEnum connectionState,
                                                      int& code,
                                                      UtlString& text) const
{
   if (connectionState == ISipConnectionState::CONNECTION_REMOTE_OFFERING)
   {
      code = SIP_TRYING_CODE;
      text = SIP_TRYING_TEXT;
      return TRUE;
   }
   else if (connectionState == ISipConnectionState::CONNECTION_REMOTE_ALERTING)
   {
      code = SIP_RINGING_CODE;
      text = SIP_RINGING_TEXT;
      return TRUE;
   }
   else if (connectionState == ISipConnectionState::CONNECTION_ESTABLISHED)
   {
      code = SIP_OK_CODE;
      text = SIP_OK_TEXT;
      return TRUE;
   }
   else if (connectionState == ISipConnectionState::CONNECTION_DISCONNECTED)
   {
      code = SIP_DECLINE_CODE;
      text = SIP_DECLINE_TEXT;
      return TRUE;
   }

   return FALSE;
}

void BaseSipConnectionState::configureRequestTransport(SipMessage& sipRequest) const
{
   Url remoteContact;
   {
      OsReadLock lock(m_rStateContext);
      m_rStateContext.m_sipDialog.getRemoteContact(remoteContact);
   }

   if (!remoteContact.isNull())
   {
      // if remote contact is known, use TCP transport only
      // if contact contains transport=tcp and preferred transport is either tcp or auto
      UtlString transportString;
      if (remoteContact.getUrlParameter(SIP_TRANSPORT, transportString))
      {
         SIP_TRANSPORT_TYPE transportType = SipTransport::getSipTransport(transportString);
         if (transportType == SIP_TRANSPORT_TCP &&
            (m_rStateContext.m_transportType == SIP_TRANSPORT_TCP || m_rStateContext.m_transportType == SIP_TRANSPORT_AUTO))
         {
            sipRequest.setPreferredTransport(SipTransport::getSipTransport(transportType));
         } // else request transport is automatic
      } // else no transport parameter, request transport is automatic
   }
   else // remote contact is not known yet, use configured transport
   {
      sipRequest.setPreferredTransport(SipTransport::getSipTransport(m_rStateContext.m_transportType));
   }
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
