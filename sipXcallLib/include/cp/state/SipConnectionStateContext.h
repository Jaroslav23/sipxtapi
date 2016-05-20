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

#ifndef SipConnectionStateContext_h__
#define SipConnectionStateContext_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlSList.h>
#include <net/SipTagGenerator.h>
#include <net/SmimeBody.h>
#include <net/SipTransport.h>
#include <cp/CpDefs.h>
#include <cp/XSipConnectionContext.h>
#include <cp/Cp100RelTracker.h>
#include <cp/CpLoopDetector.h>
#include <cp/CpSdpNegotiation.h>
#include <cp/CpSipTransactionManager.h>
#include <cp/CpSessionTimerProperties.h>
#include <cp/CpNotificationRegister.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS
class OsTimer;

/**
 * SipConnectionStateContext contains public members which are visible only to state itself
 * and don't need to be locked when accessed.
 *
 * When accessing members of XSipConnectionContext, this class needs to be locked as those
 * members are public.
 */
class SipConnectionStateContext : public XSipConnectionContext
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /**
    * MediaSessionState tracks state of media session - if RTP is flowing or not.
    */
   typedef enum
   {
      MEDIA_SESSION_NONE = 0, ///< initial state
      MEDIA_SESSION_FULLY_HELD, ///< media session is held
      MEDIA_SESSION_LOCALLY_HELD, ///< media session is held
      MEDIA_SESSION_REMOTELY_HELD, ///< media session is held
      MEDIA_SESSION_ACTIVE, ///< media session is active
   } MediaSessionState;

   /**
   * MediaConnectionState tracks state of media connection. Local represents audio sent from us
   * remote represents audio sent from remote party.
   */
   typedef enum
   {
      MEDIA_CONNECTION_NONE = 0, ///< initial state
      MEDIA_CONNECTION_ACTIVE, ///< initial state
      MEDIA_CONNECTION_HELD, ///< initial state
   } MediaConnectionState;

   typedef enum
   {
      IDENTITY_NOT_YET_ANNOUNCED, ///< connected identity has not yet been announced
      IDENTITY_UP_TO_DATE, ///< connected identity has been accepted by remote user
      IDENTITY_ANNOUNCING, ///< connected identity is being announced to remote user by UPDATE/re-INVITE requests
      IDENTITY_REJECTED, ///< connected identity announcement has been rejected
   } ConnectedIdentityState;

   typedef enum
   {
      ENTITY_NORMAL, ///< default value, no call transfer is in progress or maybe transfer target.
      ENTITY_TRANSFEREE, ///< call transfer is in progress, and we are transferee (side which received REFER)
      ENTITY_TRANSFER_CONTROLLER, ///< call transfer is in progress, and we are the controller (side which sent REFER)
   } LocalEntityType;

   typedef enum
   {
      REFER_NO_RESPONSE = 0, ///< refer was not yet accepted or rejected
      REFER_ACCEPTED, ///< refer was accepted
      REFER_REJECTED, ///< refer was rejected
   } InboundReferResponse;

   MediaSessionState m_mediaSessionState; ///< keeps track of media session state (active, held etc)
   MediaSessionState m_previousMediaSessionState; ///< keeps track of previous media session state (active, held etc)
   MediaConnectionState m_localMediaConnectionState; ///< keeps track of local media connection state
   MediaConnectionState m_remoteMediaConnectionState; ///< keeps track of remote media connection state
   CpSdpNegotiation m_sdpNegotiation; ///< tracks state of SDP negotiation
   CpLoopDetector m_loopDetector; ///< helper class for detecting loops
   CpNotificationRegister m_notificationRegister; ///< register for inter connection notifications

   // member variables storing remote party capabilities
   UtlString m_allowedRemote;  ///< methods supported by the other side
   UtlBoolean m_allowedRemoteDiscovered;  ///< TRUE if Allow: of remote party is known
   UtlString m_supportedRemote;  ///< values of Supported: header field or remote party
   UtlBoolean m_supportedRemoteDiscovered;  ///< TRUE if Supported: of remote party is known

   UtlString m_implicitAllowedRemote; ///< methods which are allowed implicitly
   SipTagGenerator m_sipTagGenerator; ///< generator for sip tags
   Cp100RelTracker m_100RelTracker; ///< tracker for 100rel responses and PRACKs
   CP_100REL_CONFIG m_100relSetting; ///< configuration of 100rel support
   CpSipTransactionManager m_sipClientTransactionMgr; ///< sip outbound transaction tracking
   CpSipTransactionManager m_sipServerTransactionMgr; ///< sip inbound transaction tracking
   UtlString m_locationHeader; ///< value of sip location header
   int m_contactId; ///< id of contact we use. Can be used to lookup SIPX_CONTACT_ADDRESS
   SIP_TRANSPORT_TYPE m_transportType; ///< transport to use for sending requests
   SIPXTACK_SECURITY_ATTRIBUTES* m_pSecurity; ///< security configuration for S/MIME
   RTP_TRANSPORT m_rtpTransport;
   SipMessage* m_pLastReceivedInvite; ///< last received INVITE
   SipMessage* m_pLastSent2xxToInvite; ///< last sent 2xx response to INVITE (sent until ACK is received)
   SipMessage* m_pLastSentRefer; ///< last sent REFER. Deleted when REFER finishes or fails.
   SipMessage* m_pLastReceivedRefer; ///< last received and accepted REFER. Deleted when REFER finishes or fails.
   int m_491failureCounter; ///< counts 491 failures, to avoid infinite loops
   UtlBoolean m_bUseLocalHoldSDP; ///< whether we use local hold SDP when offering or answering
   CP_SIP_UPDATE_CONFIG m_updateSetting; ///< whether UPDATE method is enabled
   int m_inviteExpiresSeconds; ///< expiration time for INVITE requests. If no final response is received, INVITE is canceled
   UtlString m_sBindIpAddress; ///< default local IP for media interface. May be 0.0.0.0!. Don't use for contact.

   // members related to from-change extension (rfc4916)
   Url m_realLineIdentity; ///< used to support from-change extension (rfc4916), it is the real discovered full line url
   ConnectedIdentityState m_connectedIdentityState; ///< state of connected identity announcement

   // members related to call transfer
   LocalEntityType m_localEntityType; ///< type of local entity - used to remember if we are in call transfer
   UtlBoolean m_referInSubscriptionActive; ///< TRUE if refer inbound subscription is active and should be canceled
   UtlString m_subscriptionId; ///< event ID of subscription if available
   SipDialog m_referencedSipDialog; ///< referenced sip dialog. Used when we are transfer target with replaces.
   UtlBoolean m_bDropReferencedCall; ///< drop referenced call when we enter established state
   SipDialog m_transferSipDialog; ///< sip dialog used to dial new call, when we are transferee
   UtlBoolean m_referOutSubscriptionActive; ///< TRUE when refer outbound subscription is active and should be canceled. Used to support "norefersub"
   InboundReferResponse m_inboundReferResponse; ///< flag how we responded to inbound refer

   // session timer member variables
   CpSessionTimerProperties m_sessionTimerProperties; ///< properties of session timer (RFC4028)

   // redirect member variables
   UtlSList m_redirectContactList; ///< contact URIs which should be followed in redirect
   UtlBoolean m_bRedirecting; ///< TRUE if we are following redirection

   // members used during call tear down
   UtlBoolean m_bAckReceived; ///< TRUE if ACK was received for our sent 200 OK. Needed to make decision for callee if we may send BYE.
   UtlBoolean m_bCancelSent; ///< call is being disconnected. Set to TRUE when CANCEL to INVITE is sent.
   UtlBoolean m_bByeSent; ///< TRUE when we are disconnecting and BYE was already sent
   int m_iByeRetryCount; ///< counter when retrying BYE for inbound call

   // members used for RTP redirect
   UtlBoolean m_bRTPRedirectActive; ///< TRUE if we are attached to some other call, which remote SDP we use
   SipDialog m_bRTPRedirectSipDialog; ///< SIP dialog of the call we are attached to
   UtlBoolean m_bRTPRedirectMasterRole; ///< TRUE if we are in master (not slave) role for RTP redirect
   SdpBody* m_pRemoteAttachedSdpBody; ///< last SDP body of remote party of attached call
   SdpBody* m_pRemoteSdpBody; ///< last SDP body of remote party of our call

   // timers
   OsTimer* m_pByeRetryTimer; ///< timer started if drop is attempted for inbound call, but call cannot be dropped at current state
   OsTimer* m_pCancelTimeoutTimer; ///< timer started after CANCEL is sent to force drop connection if timeout
   OsTimer* m_pByeTimeoutTimer; ///< timer started after BYE is sent to force drop connection if timeout
   OsTimer* m_pSessionRenegotiationTimer; ///< timer started when hold/unhold is requested but re-INVITE is in progress
   int m_iRenegotiationRetryCount; ///< how many times we retried m_pSessionRenegotiationTimer
   OsTimer* m_p2xxInviteRetransmitTimer; ///< timer started when 2xx response to invite is sent, shut down when ack is received
   int m_i2xxInviteRetransmitCount; ///< how many times we retransmitted 2xx
   OsTimer* m_pSessionTimeoutCheckTimer; ///< timer for checking if session timeout occurred (session timer support)
   OsTimer* m_pSessionRefreshTimer; ///< timer for periodically refreshing session (session timer support)
   OsTimer* m_pInviteExpiresTimer; ///< timer for checking INVITE expiration (occurs when INVITE is not accepted within value of Expires header)
   OsTimer* m_p100relRetransmitTimer; ///< timer for retransmitting reliable 18x response
   int m_i100relRetransmitCount; ///< how many times we retransmitted reliable 18x response
   OsTimer* m_pDelayedAnswerTimer; ///< timer for delaying call answer, until all 18x reliable responses have been acknowledged
   int m_iDelayedAnswerCount; ///< how many times we retried answer

   /* ============================ CREATORS ================================== */

   /** Constructor */
   SipConnectionStateContext();

   /** Destructor */
   ~SipConnectionStateContext();

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   void setRemoteAttachedSdpBody(SdpBody* val);
   void setRemoteSdpBody(SdpBody* val);

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   SipConnectionStateContext(const SipConnectionStateContext& rhs);

   SipConnectionStateContext& operator=(const SipConnectionStateContext& rhs);

};

#endif // SipConnectionStateContext_h__
