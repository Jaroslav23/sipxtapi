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

#ifndef BaseSipConnectionState_h__
#define BaseSipConnectionState_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <utl/UtlString.h>
#include <cp/CpDefs.h>
#include <cp/CpNatTraversalConfig.h>
#include <cp/state/ISipConnectionState.h>
#include <cp/state/SipConnectionStateContext.h>
#include <cp/msg/ScReInviteTimerMsg.h>

// DEFINES
#define MAX_ADDRESS_CANDIDATES      12

// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef enum ERROR_RESPONSE_TYPE
{
   ERROR_RESPONSE_400, // Bad Request
   ERROR_RESPONSE_481, // Call/Transaction Does Not Exist
   ERROR_RESPONSE_487, // Request Terminated
   ERROR_RESPONSE_488, // Not Acceptable Here
   ERROR_RESPONSE_491, // Request Pending
   ERROR_RESPONSE_500, // Internal Server Error
   ERROR_RESPONSE_603, // Declined
} ERROR_RESPONSE_TYPE;

// MACROS
// FORWARD DECLARATIONS
class SipUserAgent;
class CpMediaInterfaceProvider;
class CpMessageQueueProvider;
class XSipConnectionEventSink;
class XCpCallControl;
class SipConnectionStateTransition;
class StateTransitionMemory;
class SipMessage;
class ScTimerMsg;
class ScCommandMsg;
class ScNotificationMsg;
class Sc100RelTimerMsg;
class Sc2xxTimerMsg;
class ScDisconnectTimerMsg;
class ScByeRetryTimerMsg;
class ScSessionTimeoutTimerMsg;
class ScInviteExpirationTimerMsg;
class ScDelayedAnswerTimerMsg;
class ScConnStateNotificationMsg;

/**
 * Parent to all concrete sip connection states. Should be used for handling
 * common to all states. This should be used as the last resort handler, usually
 * responding with errors to requests.
 *
 * Specific response handlers are only called for SIP response messages, for which
 * an active transaction exists, and not for retransmissions (except for INVITE 200 OK).
 * There is one common fatal response handler, processResponse() which processes responses
 * according to rfc5057. Any 4xx, 5xx, 6xx to initial INVITE transaction is also treated
 * as fatal (with exception 422).
 * Responses which only affect transaction may be seen in specific response handlers.
 *
 * Some additional response codes are treated as fatal responses, and will result in immediate termination
 * of call for INVITE/UPDATE/BYE/CANCEL methods.
 * - 401 Unauthorized (will only be received for bad credentials)
 * - 407 Proxy Authentication Required (will only be received for bad credentials) - for BYE and CANCEL
 * - 408 Request Timeout (received from SipUserAgent) - for BYE and CANCEL
 */
class BaseSipConnectionState : public ISipConnectionState
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /** Constructor. */
   BaseSipConnectionState(SipConnectionStateContext& rStateContext,
                          SipUserAgent& rSipUserAgent,
                          XCpCallControl& rCallControl,
                          CpMediaInterfaceProvider* pMediaInterfaceProvider,
                          CpMessageQueueProvider* pMessageQueueProvider,
                          XSipConnectionEventSink& rSipConnectionEventSink,
                          const CpNatTraversalConfig& natTraversalConfig);

   /** Copy constructor. */
   BaseSipConnectionState(const BaseSipConnectionState& rhs);

   /** Destructor. */
   virtual ~BaseSipConnectionState();

   /* ============================ MANIPULATORS ============================== */

   /**
    * Default state entry handler.
    */
   virtual void handleStateEntry(StateEnum previousState, const StateTransitionMemory* pTransitionMemory);

   /**
    * Default state exit handler.
    */
   virtual void handleStateExit(StateEnum nextState, const StateTransitionMemory* pTransitionMemory);

   /**
   * Handles SipMessageEvent, which can be inbound sip request/response or error
   * sending status.
   * If instance cannot handle this event, it must pass it to parent as the last resort.
   *
   * @param rEvent Instance of SipMessageEvent that needs to be handled.
   * @return Instance of SipConnectionStateTransition if a transition should occur. NULL if no transition should occur.
   */
   virtual SipConnectionStateTransition* handleSipMessageEvent(const SipMessageEvent& rEvent);

   /** Connects call to given address. Uses supplied sip call-id. */
   virtual SipConnectionStateTransition* connect(OsStatus& result,
                                                 const UtlString& sipCallId,
                                                 const UtlString& localTag,
                                                 const UtlString& toAddress,
                                                 const UtlString& fromAddress,
                                                 const UtlString& locationHeader,
                                                 CP_CONTACT_ID contactId,
                                                 SIP_TRANSPORT_TYPE transport,
                                                 const UtlString& replacesField);

   /**
   * Starts redirecting call RTP. Both calls will talk directly to each other, but we keep
   * control of SIP signaling. Current call will become the master call.
   */
   virtual SipConnectionStateTransition* startRtpRedirect(OsStatus& result,
                                                          const UtlString& slaveAbstractCallId,
                                                          const SipDialog& slaveSipDialog);

   /**
   * stops redirecting call RTP. Will cancel RTP redirection for both calls participating in it.
   */
   virtual SipConnectionStateTransition* stopRtpRedirect(OsStatus& result);

   /**
   * Accepts inbound call connection, sends 180 Ringing.
   */
   virtual SipConnectionStateTransition* acceptConnection(OsStatus& result,
                                                          UtlBoolean bSendSDP,
                                                          const UtlString& locationHeader,
                                                          CP_CONTACT_ID contactId,
                                                          SIP_TRANSPORT_TYPE transport);
   /**
   * Reject the incoming connection.
   */
   virtual SipConnectionStateTransition* rejectConnection(OsStatus& result);

   /**
   * Redirect the incoming connection.
   */
   virtual SipConnectionStateTransition* redirectConnection(OsStatus& result,
                                                            const UtlString& sRedirectSipUrl);
   /**
   * Answer the incoming terminal connection, sends 200 OK.
   */
   virtual SipConnectionStateTransition* answerConnection(OsStatus& result);

   /** Accepts call transfer */
   virtual SipConnectionStateTransition* acceptTransfer(OsStatus& result);

   /** Rejects call transfer with 603 Declined */
   virtual SipConnectionStateTransition* rejectTransfer(OsStatus& result);

   /** Disconnects call */
   virtual SipConnectionStateTransition* dropConnection(OsStatus& result);

   /** Blind transfer the call to sTransferSipUri. */
   virtual SipConnectionStateTransition* transferBlind(OsStatus& result, const UtlString& sTransferSipUrl);

   /** Consultative transfer call to target call. */
   virtual SipConnectionStateTransition* transferConsultative(OsStatus& result, const SipDialog& targetSipDialog);

   /** Put the specified terminal connection on hold. */
   virtual SipConnectionStateTransition* holdConnection(OsStatus& result);

   /** Convenience method to take the terminal connection off hold. */
   virtual SipConnectionStateTransition* unholdConnection(OsStatus& result);

   /** Renegotiates media session codecs */
   virtual SipConnectionStateTransition* renegotiateCodecsConnection(OsStatus& result);

   /** Sends an INFO message to the other party(s) on the call */
   virtual SipConnectionStateTransition* sendInfo(OsStatus& result,
                                                  const UtlString& sContentType,
                                                  const char* pContent,
                                                  const size_t nContentLength,
                                                  void* pCookie);

   /** Terminates media connection silently without informing remote call party. Used for conference split/join. */
   virtual SipConnectionStateTransition* terminateMediaConnection(OsStatus& result);

   /** Handles timer message. */
   virtual SipConnectionStateTransition* handleTimerMessage(const ScTimerMsg& timerMsg);

   /** Handles CpMessageTypes::SC_COMMAND message. */
   virtual SipConnectionStateTransition* handleCommandMessage(const ScCommandMsg& rMsg);

   /** Handles CpMessageTypes::ScNotificationMsg message. */
   virtual SipConnectionStateTransition* handleNotificationMessage(const ScNotificationMsg& rMsg);

   /* ============================ ACCESSORS ================================= */

   /** Sets response code returned for inbound INFO messages - used in unit tests. */
   static void setInfoTestResponseCode(int val) { ms_iInfoTestResponseCode = val; }

   /** Sets new message queue provider on the state */
   void setMessageQueueProvider(CpMessageQueueProvider* pMessageQueueProvider);

   /** Sets new media interface provider on the state */
   void setMediaInterfaceProvider(CpMediaInterfaceProvider* pMediaInterfaceProvider);

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   /**
   * Constructs transition from current state into given destination state.
   * @pTansitionMemory Optional memory object which should be supplied to state transition. Local copy
   * will be made.
   */
   virtual SipConnectionStateTransition* getTransition(ISipConnectionState::StateEnum nextState,
                                                       const StateTransitionMemory* pTansitionMemory) const = 0;

   /**
   * Constructs transition from current state into given destination state.
   * @pTansitionMemory Optional memory object which should be supplied to state transition. Local copy
   * will be made.
   */
   virtual SipConnectionStateTransition* getTransitionObject(BaseSipConnectionState* pDestination,
                                                             const StateTransitionMemory* pTansitionMemory) const;

   /** Updates state of SipDialog with given SipMessage. Use for both inbound and outbound messages. */
   void updateSipDialog(const SipMessage& sipMessage);

   /** Initializes SipDialog */
   void initializeSipDialog(const SipMessage& sipMessage);

   /** Sends specified sip message, updating the state of SipDialog. SipMessage will be updated. */
   UtlBoolean sendMessage(SipMessage& sipMessage);

   /** Sends specified provisional response reliably */
   UtlBoolean sendReliableResponse(SipMessage& sipMessage);

   /** Handles SIP request and response messages. */
   virtual SipConnectionStateTransition* processSipMessage(const SipMessage& sipMessage);

   /**
    * Processes inbound SIP request message. This method calls special handlers for each supported
    * method that should be overridden in concrete state.
    */
   virtual SipConnectionStateTransition* processRequest(const SipMessage& sipMessage);

   /** Handles inbound SIP re-INVITE requests. It won't handle initial INVITEs. */
   virtual SipConnectionStateTransition* processInviteRequest(const SipMessage& sipMessage);

   /** Handles inbound SIP UPDATE requests */
   virtual SipConnectionStateTransition* processUpdateRequest(const SipMessage& sipMessage);

   /** Handles inbound SIP ACK requests. Processes initial INVITES and re-INVITEs */
   virtual SipConnectionStateTransition* processAckRequest(const SipMessage& sipMessage);

   /** Handles inbound SIP BYE requests */
   virtual SipConnectionStateTransition* processByeRequest(const SipMessage& sipMessage);

   /** Handles inbound SIP CANCEL requests */
   virtual SipConnectionStateTransition* processCancelRequest(const SipMessage& sipMessage);

   /** Handles inbound SIP INFO requests */
   virtual SipConnectionStateTransition* processInfoRequest(const SipMessage& sipMessage);

   /** Handles inbound SIP NOTIFY requests */
   virtual SipConnectionStateTransition* processNotifyRequest(const SipMessage& sipMessage);

   /** Handles inbound SIP REFER requests */
   virtual SipConnectionStateTransition* processReferRequest(const SipMessage& sipMessage);

   /**
    * Handles inbound SIP NOTIFY body for REFER. Response has already been sent.
    */
   virtual SipConnectionStateTransition* handleReferNotifyBody(const SipMessage& sipNotifyBody);

   /** Handles inbound SIP PRACK requests */
   virtual SipConnectionStateTransition* processPrackRequest(const SipMessage& sipMessage);

   /** Handles inbound SIP OPTIONS requests */
   virtual SipConnectionStateTransition* processOptionsRequest(const SipMessage& sipMessage);

   /** Handles inbound SIP SUBSCRIBE requests */
   virtual SipConnectionStateTransition* processSubscribeRequest(const SipMessage& sipMessage);

   /**
    * Processes inbound SIP response message. This is the default handler than needs to
    * be called first, if this method is overridden. If this method handles the response,
    * and returns new transition, overriding method or caller must not continue processing,
    * but return the same transition.
    *
    * Implementation of this method is based on RFC5057. This method is responsible for
    * transition to disconnected state if some error response is received.
    *
    * This method handles error responses in a method independent way, terminating transaction
    * or dialog if needed. Response codes 1xx, 2xx and 3xx are expected to be handled by
    * method specific handlers. ACK doesn't have response.
    */
   virtual SipConnectionStateTransition* processResponse(const SipMessage& sipMessage);

   /** Handles non fatal inbound SIP responses. These do not terminate the dialog. */
   virtual SipConnectionStateTransition* processNonFatalResponse(const SipMessage& sipMessage);

   /** Handles inbound SIP INVITE responses */
   virtual SipConnectionStateTransition* processInviteResponse(const SipMessage& sipMessage);

   /** Handles inbound 1xx SIP INVITE responses */
   virtual SipConnectionStateTransition* processProvisionalInviteResponse(const SipMessage& sipMessage);

   /** Handles inbound SIP UPDATE responses */
   virtual SipConnectionStateTransition* processUpdateResponse(const SipMessage& sipMessage);

   /** Handles inbound SIP BYE responses */
   virtual SipConnectionStateTransition* processByeResponse(const SipMessage& sipMessage);

   /** Handles inbound SIP CANCEL responses */
   virtual SipConnectionStateTransition* processCancelResponse(const SipMessage& sipMessage);

   /** Handles inbound SIP INFO responses */
   virtual SipConnectionStateTransition* processInfoResponse(const SipMessage& sipMessage);

   /** Handles inbound SIP OPTIONS responses */
   virtual SipConnectionStateTransition* processOptionsResponse(const SipMessage& sipMessage);

   /** Handles inbound SIP NOTIFY responses */
   virtual SipConnectionStateTransition* processNotifyResponse(const SipMessage& sipMessage);

   /** Handles inbound SIP REFER responses */
   virtual SipConnectionStateTransition* processReferResponse(const SipMessage& sipMessage);

   /** Handles inbound SIP PRACK responses */
   virtual SipConnectionStateTransition* processPrackResponse(const SipMessage& sipMessage);

   /** Handles inbound SIP SUBSCRIBE responses */
   virtual SipConnectionStateTransition* processSubscribeResponse(const SipMessage& sipMessage);

   /** Called when authentication retry occurs. We need to update dialogs. */
   virtual SipConnectionStateTransition* handleAuthenticationRetryEvent(const SipMessage& sipMessage);

   /** Returns TRUE if some SIP method is allowed (may be sent) */
   UtlBoolean isMethodAllowed(const UtlString& sMethod) const;

   /** Returns TRUE if some SIP extension is allowed */
   UtlBoolean isExtensionSupported(const UtlString& sExtension) const;

   /** Deletes media connection if it exists, stopping remote and local audio */
   void deleteMediaConnection();

   /** Terminates sip dialog */
   void terminateSipDialog();

   /** TRUE if dialog was initiated locally */
   UtlBoolean isLocalInitiatedDialog();

   /** Gets the outbound transaction manager */
   CpSipTransactionManager& getClientTransactionManager() const;

   /** Gets the inbound transaction manager */
   CpSipTransactionManager& getServerTransactionManager() const;

   /** Gets ID of media connection */
   int getMediaConnectionId() const;

   /** Sets ID of media connection */
   void setMediaConnectionId(int mediaConnectionId);

   /** Handles 100Rel timer message. */
   virtual SipConnectionStateTransition* handle100RelTimerMessage(const Sc100RelTimerMsg& timerMsg);

   /** Handles 2xx timer message. */
   virtual SipConnectionStateTransition* handle2xxTimerMessage(const Sc2xxTimerMsg& timerMsg);

   /** Handles disconnect timer message. */
   virtual SipConnectionStateTransition* handleDisconnectTimerMessage(const ScDisconnectTimerMsg& timerMsg);

   /** Handles re-invite timer message. */
   virtual SipConnectionStateTransition* handleReInviteTimerMessage(const ScReInviteTimerMsg& timerMsg);

   /** Handles session renegotiation timer message. */
   virtual SipConnectionStateTransition* handleRenegotiateTimerMessage(const ScReInviteTimerMsg& timerMsg);

   /** Handles session refresh timer message. */
   virtual SipConnectionStateTransition* handleRefreshSessionTimerMessage(const ScReInviteTimerMsg& timerMsg);

   /** Handles session timeout check timer message. */
   virtual SipConnectionStateTransition* handleSessionTimeoutCheckTimerMessage(const ScSessionTimeoutTimerMsg& timerMsg);

   /** Handles invite expiration timer message. */
   virtual SipConnectionStateTransition* handleInviteExpirationTimerMessage(const ScInviteExpirationTimerMsg& timerMsg);

   /** Handles bye retry timer message. */
   virtual SipConnectionStateTransition* handleByeRetryTimerMessage(const ScByeRetryTimerMsg& timerMsg);

   /** Handles delayed answer timer message. */
   virtual SipConnectionStateTransition* handleDelayedAnswerTimerMessage(const ScDelayedAnswerTimerMsg& timerMsg);

   /** Handles connection state notification message. */
   virtual SipConnectionStateTransition* handleConnStateNotificationMessage(const ScConnStateNotificationMsg& rMsg);

   /** Quick access to sip call-id */
   UtlString getCallId() const;

   /** Checks if given sip method needs transaction tracking */
   UtlBoolean needsTransactionTracking(const UtlString& sipMethod) const;

   /** 
    * Builds contact URL based on contactId. Might be sips if TLS transport is selected. UserId, display name
    * are filled from fromAddress.
    */
   UtlString buildContactUrl(const Url& fromAddress) const;

   /** Initializes contact of dialog. Must be called for inbound calls, after we know contact Id */
   void initDialogContact(CP_CONTACT_ID contactId, const Url& localField);

   /** 
    * Gets local contact URL from SipDialog. Can only be used once SipDialog has been initialized with first
    * outbound request. Contact Url will have display name if its available.
    */
   void getLocalContactUrl(Url& contactUrl) const;

   /**
    * Gets local contact URL as string.
    */
   UtlString getLocalContactUrl() const;

   /**
    * Builds default contact URL. URL will not be sips, and will contain UserId, display name from fromAddress.
    * This function builds contact if we haven't selected any contact ID.
    */
   UtlString buildDefaultContactUrl(const Url& fromAddress) const;

   /** Gets local tag from sip dialog */
   UtlString getLocalTag() const;

   /** Changes scheme to sips: if secured transport is used based on contactId */
   void secureUrl(Url& fromAddress) const;

   /** Creates media connection from media interface, returning mediaConnectionId */
   UtlBoolean setupMediaConnection(RTP_TRANSPORT rtpTransportOptions, int& mediaConnectionId);

   /** Starts SDP negotiation, adds SDP offer to given SipMessage and creates a media connection if it doesn't exist */
   UtlBoolean prepareSdpOffer(SipMessage& sipMessage);

   /**
    * handles inbound SDP offer, starts SDP negotiation, creating media connection if it doesn't exist
    * Returns FALSE if an error occurred and connection needs to be disconnected.
    */
   UtlBoolean handleSdpOffer(const SipMessage& sipMessage);

   /** adds SDP offer to given SipMessage */
   UtlBoolean prepareSdpAnswer(SipMessage& sipMessage);

   /**
    * handles inbound SDP answer, setting media connection destination
    * Returns FALSE if an error occurred and connection needs to be disconnected.
    */
   UtlBoolean handleSdpAnswer(const SipMessage& sipMessage);

   /** Handles remote SDP body, finds out if there is match between our local codecs and received codecs */
   UtlBoolean handleRemoteSdpBody(const SdpBody& sdpBody);

   /** Commits changes negotiated by SDP offer/answer, starting/stopping RTP flow */
   UtlBoolean commitMediaSessionChanges();

   /** Sets last received invite */
   void setLastReceivedInvite(const SipMessage& sipMessage);

   /** Sets last sent 2xx response to invite */
   void setLastSent2xxToInvite(const SipMessage& sipMessage);

   /** Sets last sent REFER message */
   void setLastSentRefer(const SipMessage& sipMessage);

   /** Sets last received and accepted REFER message */
   void setLastReceivedRefer(const SipMessage& sipMessage);

   /** Gets session timer properties */
   CpSessionTimerProperties& getSessionTimerProperties();

   /** Handles 422 INVITE/UPDATE response */
   void handleSmallSessionIntervalResponse(const SipMessage& sipMessage);

   /** Sends options request, regardless of whether dialog is established */
   void sendOptionsRequest();

   /** Checks if we may send in dialog OPTIONS request, and sends it if we can. */
   void discoverRemoteCapabilities();

   /** Sends ACK to given 200 OK response */
   virtual SipConnectionStateTransition* handleInvite2xxResponse(const SipMessage& sipResponse);

   /** Sends BYE to terminate call. Optionally also specify values of Reason field. */
   void sendBye(int cause = 0, const UtlString& text = NULL);

   /** Sends CANCEL to terminate call. Optionally also specify values of Reason field. */
   void sendInviteCancel(int cause = 0, const UtlString& text = NULL);

   /** Sends re-INVITE for hold/unhold/codec renegotiation, or initial INVITE after 422 response */
   void sendInvite();

   /** Sends UPDATE for hold/unhold/codec renegotiation */
   void sendUpdate(UtlBoolean bRenegotiateCodecs = TRUE);

   /** Sends NOTIFY for inbound REFER, for given connection state of newly created call */
   void sendReferNotify(ISipConnectionState::StateEnum connectionState);

   /** 
    * Sends PRACK request, confirming reception of specified 1xx sip response.
    * @param bSendSDPAnswer When TRUE, then SDP answer will be generated. Otherwise
    *        an SDP offer might be generated.
    * @return TRUE if no error occurred. FALSE if there was an error. An attempt will be
    *         made to send PRACK regardless of error.
    */
   UtlBoolean sendPrack(const SipMessage& sipResponse, UtlBoolean bSendSDPAnswer);

   /**
    * Sets media connection destination to given host/ports if ICE is disabled, or to all
    * candidate addresses in SDP body if ICE is enabled.
    */
   void setMediaDestination(const char* hostAddress, ///< remote RTP address
                            int audioRtpPort, ///< remote RTP audio port
                            int audioRtcpPort, ///< remote RTPC audio port
                            int videoRtpPort,
                            int videoRtcpPort,
                            const SdpBody* pRemoteBody);

   /** Disconnects call. CANCEL should be used if 200 OK was not yet received */
   SipConnectionStateTransition* doByeConnection(OsStatus& result);

   /** Disconnects outbound call using CANCEL. Cannot be used on inbound call */
   SipConnectionStateTransition* doCancelConnection(OsStatus& result);

   /** Sends immediate request to call to destroy this connection. */
   void requestConnectionDestruction();

   /** Starts cancel timer for force dropping connection */
   void startCancelTimeoutTimer();

   /** Deletes cancel timer */
   void deleteCancelTimeoutTimer();

   /** Starts bye timer for force dropping connection */
   void startByeTimeoutTimer();

   /** Deletes bye timer */
   void deleteByeTimeoutTimer();

   /** 
    * Starts bye timer for trying to send BYE later. For inbound call we may not send BYE
    * after 200 OK unless we receive ACK. Therefore we wait a little bit, and if ACK is not received
    * then drop the call. If drop was requested we do not want to wait until 200 OK resending gives
    * a timeout.
    */
   void startByeRetryTimer();

   /** Deletes delayed bye timer */
   void deleteByeRetryTimer();

   /** Starts timer to retransmit 2xx messages until ack is received */
   void start2xxRetransmitTimer();

   /** Deletes timer responsible for retransmit of 2xx invite responses */
   void delete2xxRetransmitTimer();

   /** Starts timer to check session timeout */
   void startSessionTimeoutCheckTimer();

   /** Deletes timer for checking session timeout */
   void deleteSessionTimeoutCheckTimer();

   /** Starts timer to refresh session */
   void startSessionRefreshTimer();

   /** Deletes timer for refreshing session */
   void deleteSessionRefreshTimer();

   /** Starts timer for checking invite expiration */
   void startInviteExpirationTimer(int timeoutSec, int cseqNum, UtlBoolean bIsOutbound);

   /** Deletes timer for checking invite expiration */
   void deleteInviteExpirationTimer();

   /** Starts timer to retransmit 100rel message */
   void start100relRetransmitTimer(const SipMessage& c100relResponse);

   /** Deletes timer to retransmit 100rel message */
   void delete100relRetransmitTimer();

   /** Starts timer to answer call with delay */
   void startDelayedAnswerTimer();

   /** Deletes timer for answering call with delay */
   void deleteDelayedAnswerTimer();

   /** Gets Id of sip dialog */
   void getSipDialogId(UtlString& sipCallId,
                       UtlString& localTag,
                       UtlString& remoteTag,
                       UtlBoolean& isFromLocal);

   /** Gets next local CSeq number */
   int getNextLocalCSeq();

   /** Gets random CSeq number for bootstrap */
   int getRandomCSeq() const;

   /** Must be called for requests to track inbound/outbound transactions */
   void trackTransactionRequest(const SipMessage& sipMessage);

   /** Must be called for responses to track inbound/outbound transactions */
   void trackTransactionResponse(const SipMessage& sipMessage);

   /**
    * Verifies inbound request cseq number (must be monotonous), and does
    * loop detection. If error is detected then sends answer immediately.
    * Returns FALSE if processing of message should stop.
    */
   UtlBoolean verifyInboundRequest(const SipMessage& sipMessage);

   /**
    * Gets state of invite transaction. Considers both outbound and inbound invite transactions.
    */
   CpSipTransactionManager::InviteTransactionState getInviteTransactionState() const;

   /** Gets state of inbound invite transaction. */
   CpSipTransactionManager::InviteTransactionState getServerInviteTransactionState() const;

   /** Gets state of outbound invite transaction. */
   CpSipTransactionManager::InviteTransactionState getClientInviteTransactionState() const;

   /** Starts hold/unhold timer to execute the action later. If bCleanStart is TRUE, then restart counting. */
   void startSessionRenegotiationTimer(ScReInviteTimerMsg::ReInviteReason reason, UtlBoolean bCleanStart = TRUE);

   /** Deletes hold/unhold timer */
   void deleteSessionRenegotiationTimer();

   /** Returns TRUE if some UPDATE is active. */
   UtlBoolean isUpdateActive();

   /** Returns TRUE if some outbound UPDATE is active. */
   UtlBoolean isOutboundUpdateActive();

   /** Returns TRUE if some inbound UPDATE is active. */
   UtlBoolean isInboundUpdateActive();

   /** Returns TRUE if we may start renegotiating media session now */
   UtlBoolean mayRenegotiateMediaSession();

   /** Initiates hold via re-INVITE or UPDATE */
   void doHold();

   /** Initiates unhold via re-INVITE or UPDATE */
   void doUnhold();

   /**
    * Refreshes session. May also renegotiate codecs. Uses re-INVITE or UPDATE.
    * @param bRenegotiateCodecs When FALSE, then codecs won't be renegotiated if UPDATE
    *        method is used.
    */
   void refreshSession(UtlBoolean bRenegotiateCodecs = TRUE);

   /** Sets state of local media connection. Don't update state directly. Use this function. */
   void setLocalMediaConnectionState(SipConnectionStateContext::MediaConnectionState state,
                                     UtlBoolean bRefreshMediaSessionState = TRUE);

   /** Sets state of remote media connection. Don't update state directly. Use this function. */
   void setRemoteMediaConnectionState(SipConnectionStateContext::MediaConnectionState state,
                                      UtlBoolean bRefreshMediaSessionState = TRUE);

   /** Refreshes state of media session. Must be called after media connection state is changed */
   void refreshMediaSessionState();

   /** Fires media session events, to notify sipxtapi about held/active/remote held call states */
   void fireMediaSessionEvents(UtlBoolean bForce = FALSE, UtlBoolean bSupressConnected = FALSE);

   /**
    * Prepares a SIP request message, with given method and given cseq number.
    * From, to, sip uri, contact and other fields are filled automatically.
    */
   void prepareSipRequest(SipMessage& sipRequest, const UtlString& method, int cseqNum);

   /** Handles INVITE redirects */
   virtual SipConnectionStateTransition* handleInviteRedirectResponse(const SipMessage& sipMessage);

   /** Follows next redirect, or terminates call if none are left */
   virtual SipConnectionStateTransition* followNextRedirect();

   /** Gets sip call-id. Useful for log messages. */
   UtlString getSipCallId() const;

   /**
    * Handles answer of session timer request in sip response message. This method can handle
    * both answers from remote side, but also responses which we are about to send.
    */
   void handleSessionTimerResponse(const SipMessage& sipResponse);

   /** Prepares session timer response, based on request */
   void prepareSessionTimerResponse(const SipMessage& sipRequest, SipMessage& sipResponse);

   /** Prepares session timer request */
   void prepareSessionTimerRequest(SipMessage& sipRequest);

   /** Prepares error response to given sip request, based on enum value. */
   void prepareErrorResponse(const SipMessage& sipRequest, SipMessage& sipResponse, ERROR_RESPONSE_TYPE responseType) const;

   /** Deletes all timers */
   void deleteAllTimers();

   /** Updates given timer. Old timer is deleted and new one will be created with new queue. */
   void updateTimer(OsTimer **pTimer);

   /** Updates all timers to use new queue */
   void updateAllTimers();

   /** Updates known capabilities about remote party. Updates Allow, Supported.*/
   void updateRemoteCapabilities(const SipMessage& sipMessage);

   /** Call to reset remote party capabilities. Must be called when following redirect. */
   void resetRemoteCapabilities();

   /** Adds Require: 100rel if 100rel is configured to be mandatory */
   void maybeRequire100rel(SipMessage& sipRequest) const;

   /** Returns TRUE if we should send reliable 18x response */
   UtlBoolean shouldSend100relResponse() const;

   /**
    * Gets type of SDP body in PRACK request. This method uses heuristic based
    * on the assumption that we send only one 18x reliable response. If we want
    * to support sending multiple reliable responses, we need more powerful SDP type
    * tracking.
    */
   CpSdpNegotiation::SdpBodyType getPrackSdpBodyType(const SipMessage& sipMessage) const;

   /**
    * If needed and remote side supports it, try to announce new remote identity
    * if it has changed. Modifies supplied SipMessage request to have different from
    * field, using the same tag.
    */
   void announceConnectedIdentity(SipMessage& sipRequest) const;

   /**
   * Announcement of connected identity has been accepted, update localField of dialog.
   */
   void onConnectedIdentityAccepted();

   /**
    * Connected identity was rejected.
    */
   void onConnectedIdentityRejected();

   /** Updates remote sip dialog field with that from sip message */
   void updateSipDialogRemoteField(const SipMessage& sipRequest);

   /**
    * Initiates call transfer to given url. For consultative transfer, the url must contain header parameter Replaces,
    * to-tag and from-tag.
    */
   OsStatus transferCall(const Url& sReferToSipUrl);

   /** notifies connection state observers about current state */
   void notifyConnectionStateObservers();

   /** Called when REFER message is received, and we are ready to process it. Basic checks have already been done. */
   UtlBoolean followRefer(const SipMessage& sipRequest);

   /**
    * Gets status code for sending in NOTIFY message as status for REFER (transferee). Returns FALSE
    * if NOTIFY should not be sent.
    */
   UtlBoolean getReferNotifyCode(ISipConnectionState::StateEnum connectionState, int& code, UtlString& text) const;

   /**
    * Configures preferred transport that should be used for given sip request.
    */
   void configureRequestTransport(SipMessage& sipRequest) const;

   SipConnectionStateContext& m_rStateContext; ///< context containing state of sip connection. Needs to be locked when accessed.
   SipUserAgent& m_rSipUserAgent; // for sending sip messages
   XCpCallControl& m_rCallControl; ///< interface for controlling other calls
   CpMediaInterfaceProvider* m_pMediaInterfaceProvider; ///< media interface provider
   CpMessageQueueProvider* m_pMessageQueueProvider; ///< message queue provider
   XSipConnectionEventSink& m_rSipConnectionEventSink; ///< event sink (router) for various sip connection event types
   CpNatTraversalConfig m_natTraversalConfig; ///< NAT traversal configuration

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private assignment operator */
   BaseSipConnectionState& operator=(const BaseSipConnectionState& rhs);

   static int ms_iInfoTestResponseCode; ///< test response code to INFO request, used in unit tests
};

#endif // BaseSipConnectionState_h__
