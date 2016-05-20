//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef CpDefs_h__
#define CpDefs_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES

#ifdef LONG_EVENT_RESPONSE_TIMEOUTS
#  define CP_MAX_EVENT_WAIT_SECONDS    2592000    // 30 days in seconds
#else
#  define CP_MAX_EVENT_WAIT_SECONDS    30         // time out, seconds
#endif

#define CP_CALL_HISTORY_LENGTH 50

#define CP_MAXIMUM_RINGING_EXPIRE_SECONDS 180
#define CP_MINIMUM_RINGING_EXPIRE_SECONDS 1

#define CONF_MAX_CONNECTIONS    32      /**< Max number of conference participants */

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
// STRUCTS
// TYPEDEFS

#define AUTOMATIC_CONTACT_ID -1

typedef int CP_CONTACT_ID; 

/**
* DTMF/other tone ids.
*/
typedef enum CP_TONE_ID
{
   CP_ID_DTMF_0 = 0,
   CP_ID_DTMF_1 = 1,
   CP_ID_DTMF_2 = 2,
   CP_ID_DTMF_3 = 3,
   CP_ID_DTMF_4 = 4,
   CP_ID_DTMF_5 = 5,
   CP_ID_DTMF_6 = 6,
   CP_ID_DTMF_7 = 7,
   CP_ID_DTMF_8 = 8,
   CP_ID_DTMF_9 = 9,
   CP_ID_DTMF_STAR = 10,
   CP_ID_DTMF_POUND = 11,
   CP_ID_DTMF_A = 12,
   CP_ID_DTMF_B = 13,
   CP_ID_DTMF_C = 14,
   CP_ID_DTMF_D = 15,
   CP_ID_DTMF_FLASH = 16,
   CP_ID_TONE_DIALTONE = 512,
   CP_ID_TONE_BUSY,
   CP_ID_TONE_RINGBACK,
   CP_ID_TONE_RINGTONE,
   CP_ID_TONE_CALLFAILED,
   CP_ID_TONE_SILENCE,
   CP_ID_TONE_BACKSPACE,
   CP_ID_TONE_CALLWAITING,
   CP_ID_TONE_CALLHELD,
   CP_ID_TONE_LOUD_FAST_BUSY
} CP_TONE_ID;

typedef enum
{
   CP_MEDIA_TYPE_AUDIO = 0,
   CP_MEDIA_TYPE_VIDEO,
} CP_MEDIA_TYPE;

typedef enum
{
   CP_MEDIA_CAUSE_NORMAL = 0,
   CP_MEDIA_CAUSE_HOLD,
   CP_MEDIA_CAUSE_UNHOLD,
   CP_MEDIA_CAUSE_FAILED,
   CP_MEDIA_CAUSE_DEVICE_UNAVAILABLE,
   CP_MEDIA_CAUSE_INCOMPATIBLE,
   CP_MEDIA_CAUSE_DTMF_INBAND,
   CP_MEDIA_CAUSE_DTMF_RFC2833,
   CP_MEDIA_CAUSE_DTMF_SIPINFO
} CP_MEDIA_CAUSE;

typedef enum
{
   CP_MEDIA_UNKNOWN = 0,
   CP_MEDIA_LOCAL_START = 50000,
   CP_MEDIA_LOCAL_STOP, 
   CP_MEDIA_REMOTE_START,
   CP_MEDIA_REMOTE_STOP,
   CP_MEDIA_REMOTE_SILENT,
   CP_MEDIA_PLAYFILE_START,
   CP_MEDIA_PLAYFILE_STOP,
   CP_MEDIA_PLAYBUFFER_START,
   CP_MEDIA_PLAYBUFFER_STOP,
   CP_MEDIA_PLAYBACK_PAUSED,
   CP_MEDIA_PLAYBACK_RESUMED,
   CP_MEDIA_REMOTE_DTMF,
   CP_MEDIA_DEVICE_FAILURE,
   CP_MEDIA_REMOTE_ACTIVE,
   CP_MEDIA_RECORDING_START,
   CP_MEDIA_RECORDING_STOP
} CP_MEDIA_EVENT;

typedef enum
{
    CP_INFOSTATUS_UNKNOWN = 0,
    CP_INFOSTATUS_RESPONSE = 30000,
    CP_INFOSTATUS_NETWORK_ERROR = 31000
} CP_INFOSTATUS_EVENT;

typedef enum
{
   CP_CALLSTATE_UNKNOWN         = 0,    /**< An UNKNOWN event is generated when the state for a call 
                                       is no longer known.  This is generally an error 
                                       condition; see the minor event for specific causes. */
   CP_CALLSTATE_NEWCALL         = 1000, /**< The NEWCALL event indicates that a new call has been 
                                       created automatically by the sipXtapi.  This event is 
                                       most frequently generated in response to an inbound 
                                       call request.  */
   CP_CALLSTATE_DIALTONE        = 2000, /**< The DIALTONE event indicates that a new call has been 
                                       created for the purpose of placing an outbound call.  
                                       The application layer should determine if it needs to 
                                       simulate dial tone for the end user. */
   CP_CALLSTATE_REMOTE_OFFERING = 2500, /**< The REMOTE_OFFERING event indicates that a call setup 
                                       invitation has been sent to the remote party.  The 
                                       invitation may or may not every receive a response.  If
                                       a response is not received in a timely manor, sipXtapi 
                                       will move the call into a disconnected state.  If 
                                       calling another sipXtapi user agent, the reciprocal 
                                       state is OFFER. */
   CP_CALLSTATE_REMOTE_ALERTING = 3000, /**< The REMOTE_ALERTING event indicates that a call setup 
                                       invitation has been accepted and the end user is in the
                                       alerting state (ringing).  Depending on the SIP 
                                       configuration, end points, and proxy servers involved, 
                                       this event should only last for 3 minutes.  Afterwards,
                                       the state will automatically move to DISCONNECTED.  If 
                                       calling another sipXtapi user agent, the reciprocate 
                                       state is ALERTING. 
                                    
                                       Pay attention to the cause code for this event.  If
                                       the cause code is "CALLSTATE_CAUSE_EARLY_MEDIA", the 
                                       remote the party is sending early media (e.g. gateway is
                                       producing ringback or audio feedback).  In this case, the
                                       user agent should not produce local ringback. */
   CP_CALLSTATE_CONNECTED       = 4000, /**< The CONNECTED state indicates that call has been setup 
                                       between the local and remote party.  Network audio should be 
                                       flowing provided and the microphone and speakers should
                                       be engaged. */
   CP_CALLSTATE_BRIDGED         = 5000, /** The BRIDGED state indicates that a call is active,
                                       however, the local microphone/speaker are not engaged.  If
                                       this call is part of a conference, the party will be able
                                       to talk with other BRIDGED conference parties.  Application
                                       developers can still play and record media. */
   CP_CALLSTATE_HELD            = 6000, /** The HELD state indicates that a call is
                                       both locally and remotely held.  No network audio is flowing 
                                       and the local microphone and speaker are not engaged. */
   CP_CALLSTATE_REMOTE_HELD     = 7000, /** The REMOTE_HELD state indicates that the remote 
                                       party is on hold.  Locally, the microphone and speaker are
                                       still engaged, however, no network audio is flowing. */

   CP_CALLSTATE_DISCONNECTED    = 8000, /**< The DISCONNECTED state indicates that a call was 
                                       disconnected or failed to connect.  A call may move 
                                       into the DISCONNECTED states from almost every other 
                                       state.  Please review the DISCONNECTED minor events to
                                       understand the cause. If disconnected state is a result
                                       of remote party hanging up, the application must destroy
                                       the call explicitly unless it is a conference call.
                                       Conference calls are destroyed automatically.*/
   CP_CALLSTATE_OFFERING        = 9000, /**< An OFFERING state indicates that a new call invitation 
                                       has been extended this user agent.  Application 
                                       developers should invoke sipxCallAccept(), 
                                       sipxCallReject() or sipxCallRedirect() in response.  
                                       Not responding will result in an implicit call 
                                       sipXcallReject(). */                                
   CP_CALLSTATE_ALERTING        = 10000, /**< An ALERTING state indicates that an inbound call has 
                                       been accepted and the application layer should alert 
                                       the end user.  The alerting state is limited to 3 
                                       minutes in most configurations; afterwards the call 
                                       will be canceled.  Applications will generally play 
                                       some sort of ringing tone in response to this event. */
   CP_CALLSTATE_DESTROYED       = 11000, /**< The DESTORYED event indicates the underlying resources 
                                       have been removed for a call.  This is the last event 
                                       that the application will receive for any call.  The 
                                       call handle is invalid after this event is received. */
   CP_CALLSTATE_TRANSFER_EVENT   = 12000, /**< The transfer state indicates a state change in a 
                                       transfer attempt.  Please see the CALLSTATE_TRANSFER_EVENT cause 
                                       codes for details on each state transition */
   CP_CALLSTATE_QUEUED          = 13000,/**< inbound Call has been queued - is awaiting processing. */
   CP_CALLSTATE_REMOTE_QUEUED   = 14000, /**< Outbound call has been put into queued state by remote party. */
} CP_CALLSTATE_EVENT;

/**
 * Callstate cause events identify the reason for a Callstate event or 
 * provide more detail.
 */
typedef enum
{
   CP_CALLSTATE_CAUSE_UNKNOWN,        /**< Unknown cause */
   CP_CALLSTATE_CAUSE_NORMAL,         /**< The stage changed due to normal operation */
   CP_CALLSTATE_CAUSE_TRANSFERRED,	  /**< A call is being transferred to this user 
                                        agent from another user agent.*/
   CP_CALLSTATE_CAUSE_TRANSFER,	     /**< A call on this user agent is being transferred 
                                        to another user agent. */                                     
   CP_CALLSTATE_CAUSE_CONFERENCE,     /**< A conference operation caused a stage change */
   CP_CALLSTATE_CAUSE_EARLY_MEDIA,    /**< The remote party is alerting and providing 
                                        ringback audio (early media) */
   CP_CALLSTATE_CAUSE_REQUEST_NOT_ACCEPTED, 
                                   /**< The callee rejected a request (e.g. hold) */
   CP_CALLSTATE_CAUSE_BAD_ADDRESS,    /**< The state changed due to a bad address.  This 
                                        can be caused by a malformed URL or network
                                        problems with your DNS server */
   CP_CALLSTATE_CAUSE_BUSY,           /**< The state changed because the remote party is
                                        busy */
   CP_CALLSTATE_CAUSE_RESOURCE_LIMIT, /**< Not enough resources are available to complete
                                        the desired operation */
   CP_CALLSTATE_CAUSE_NETWORK,        /**< A network error caused the desired operation to 
                                        fail */
   CP_CALLSTATE_CAUSE_REJECTED,       /**< The stage changed due to a rejection of a call. */
   CP_CALLSTATE_CAUSE_REDIRECTED,     /**< The stage changed due to a redirection of a call. */
   CP_CALLSTATE_CAUSE_NO_RESPONSE,    /**< No response was received from the remote party or 
                                        network node. */
   CP_CALLSTATE_CAUSE_AUTH,           /**< Unable to authenticate due to either bad or 
                                        missing credentials */
   CP_CALLSTATE_CAUSE_TRANSFER_INITIATED,  
                                /**< A transfer attempt has been initiated.  This event
                                     is sent when a user agent attempts either a blind
                                     or consultative transfer. */
   CP_CALLSTATE_CAUSE_TRANSFER_ACCEPTED,  
                                /**< A transfer attempt has been accepted by the remote
                                     transferee.  This event indicates that the 
                                     transferee supports transfers (REFER method).  The
                                     event is fired upon a 2xx class response to the SIP
                                     REFER request. */
   CP_CALLSTATE_CAUSE_TRANSFER_TRYING,
                                /**< The transfer target is attempting the transfer.  
                                     This event is sent when transfer target (or proxy /
                                     B2BUA) receives the call invitation, but before the
                                     the tranfer target accepts is. */
   CP_CALLSTATE_CAUSE_TRANSFER_RINGING,   
                                /**< The transfer target is ringing.  This event is 
                                     generally only sent during blind transfer.  
                                     Consultative transfer should proceed directly to 
                                     TRANSFER_SUCCESS or TRANSFER_FAILURE. */
   CP_CALLSTATE_CAUSE_TRANSFER_SUCCESS,
                                /**< The transfer was completed successfully.  The
                                     original call to transfer target will
                                     automatically disconnect.*/
   CP_CALLSTATE_CAUSE_TRANSFER_FAILURE,
                                /**< The transfer failed.  After a transfer fails,
                                     the application layer is responsible for 
                                     recovering original call to the transferee. 
                                     That call is left on hold. */
   CP_CALLSTATE_CAUSE_REMOTE_SMIME_UNSUPPORTED,
                                /**< Fired if the remote party's user-agent does not
                                     support S/MIME. */
   CP_CALLSTATE_CAUSE_SMIME_FAILURE,
                                /**< Fired if a local S/MIME operation failed. 
                                     For more information, applications should 
                                     process the SECURITY event. */
   CP_CALLSTATE_CAUSE_BAD_REFER,     /**< An unusable refer was sent to this user-agent. */    
   CP_CALLSTATE_CAUSE_NO_KNOWN_INVITE, /**< This user-agent received a request or response, 
                                         but there is no known matching invite. */  
   CP_CALLSTATE_CAUSE_BYE_DURING_IDLE, /**< A BYE message was received, however, the call is in
                                         in an idle state. */       
   CP_CALLSTATE_CAUSE_UNKNOWN_STATUS_CODE, /**< A response was received with an unknown status code. */
   CP_CALLSTATE_CAUSE_BAD_REDIRECT,    /**< Receive a redirect with NO contact or a RANDOM redirect. */
   CP_CALLSTATE_CAUSE_TRANSACTION_DOES_NOT_EXIST, /**< No such transaction;  Accepting or Rejecting a call that
                                                    is part of a transfer. */
   CP_CALLSTATE_CAUSE_CANCEL,        /**< The event was fired in response to a cancel
                                       attempt from the remote party */
   CP_CALLSTATE_CAUSE_CLIENT_ERROR,/**< Result of unknown 4xx response */
   CP_CALLSTATE_CAUSE_SERVER_ERROR,/**< Result of unknown 5xx response */
   CP_CALLSTATE_CAUSE_GLOBAL_ERROR,/**< Result of unknown 6xx response */
} CP_CALLSTATE_CAUSE;

/**
* Enumeration of possible CP_RTP_REDIRECT events.
*/
typedef enum
{
   CP_RTP_REDIRECT_REQUESTED = 0, ///< fired when RTP redirect is initiated, but final result is unknown
   CP_RTP_REDIRECT_ACTIVE, ///< fired when RTP redirect succeeds. However a failure may be fired later.
   CP_RTP_REDIRECT_ERROR, ///< fired when RTP redirect fails for some reason after being initiated.
   CP_RTP_REDIRECT_STOP ///< fired when RTP redirect stops. Will be fired only after SUCCESS event.
} CP_RTP_REDIRECT_EVENT;

/**
* Enumeration of possible CP_RTP_REDIRECT cause codes.
*/
typedef enum
{
   CP_RTP_REDIRECT_CAUSE_NORMAL = 0,         /**< No error occurred. */
   CP_RTP_REDIRECT_CAUSE_SDP_CODEC_MISMATCH, /**< SDP codec mismatch occurred during negotiation. */
   CP_RTP_REDIRECT_CAUSE_CALL_NOT_READY, /**< Used when RTP redirect is requested but call is not ready for RTP redirect.
                                           Request may be retried at later time. */
   CP_RTP_REDIRECT_CAUSE_SETUP_FAILED, /**< Setup of RTP redirect failed. RTP redirect could not be coordinated successfully
                                         between participating calls. */
} CP_RTP_REDIRECT_CAUSE;

/**
* Enumeration of possible CP_CONFERENCE_EVENT events.
*/
typedef enum
{
   CP_CONFERENCE_CREATED = 0, ///< fired when conference is created
   CP_CONFERENCE_DESTROYED, ///< fired when conference is destroyed
   CP_CONFERENCE_CALL_ADDED, ///< fired when a new call is added to conference
   CP_CONFERENCE_CALL_ADD_FAILURE, ///< fired when call failed to be added to conference (join)
   CP_CONFERENCE_CALL_REMOVED, ///< fired when a call is removed from conference
   CP_CONFERENCE_CALL_REMOVE_FAILURE ///< fired when call failed to be removed from conference (split)
} CP_CONFERENCE_EVENT;

/**
* Enumeration of possible CP_CONFERENCE_CAUSE cause codes.
*/
typedef enum
{
   CP_CONFERENCE_CAUSE_NORMAL = 0,         /**< No error occurred. */
   CP_CONFERENCE_CAUSE_INVALID_STATE,       ///< call is in invalid state for requested operation
   CP_CONFERENCE_CAUSE_NOT_FOUND,           ///< call was not found
   CP_CONFERENCE_CAUSE_LIMIT_REACHED,       ///< call limit was reached
   CP_CONFERENCE_CAUSE_UNEXPECTED_ERROR,    ///< some unknown unexpected error occurred
} CP_CONFERENCE_CAUSE;

/**
* Configuration of session timer refresher. Refresher is side which is responsible
* for periodically refreshing the session with re-INVITE or UPDATE within session
* expiration time. If no session refresh occurs until that period, session may be
* torn down. Refresher is negotiated by default.
*/
typedef enum CP_SESSION_TIMER_REFRESH
{
   CP_SESSION_REFRESH_AUTO = 0, /**< Refresher negotiation is automatic  */
   CP_SESSION_REFRESH_LOCAL,  /**< Our side will carry out session refresh */
   CP_SESSION_REFRESH_REMOTE   /**< Remote side will carry out session refresh */
} CP_SESSION_TIMER_REFRESH;

/**
 * Configuration of SIP UPDATE method. By default, processing of inbound UPDATE
 * is enabled, but sipXtapi will never send UPDATE itself. It is possible to enable
 * sending UPDATE instead of re-INVITE for hold/unhold/session refresh/codec renegotiation.
 * UPDATE is faster than re-INVITE, but requires immediate response without user interaction,
 * and could therefore be rejected.
 *
 * UPDATE will only be sent, if remote party supports it. Otherwise re-INVITE will be used, even
 * if UPDATE is enabled.
 */
typedef enum CP_SIP_UPDATE_CONFIG
{
   CP_SIP_UPDATE_DISABLED = 0, /**< UPDATE is completely disabled, UPDATE requests will be rejected,
                                *   but sipXtapi will continue advertising support for UPDATE
                                *   method. RFC4916 (Connected Identity) will use re-INVITE.
                                */
   CP_SIP_UPDATE_ONLY_INBOUND, /**< UPDATE is enabled only for inbound requests - default.
                                *   RFC4916 (Connected Identity) will use re-INVITE.
                                */
   CP_SIP_UPDATE_BOTH          /**< We may send UPDATE if remote side supports it,
                                *   and accept inbound requests. RFC4916 support will use
                                *   UPDATE if possible.
                                */
} CP_SIP_UPDATE_CONFIG;

/**
 * Configuration of reliable provisional responses (100rel) support in sipXtapi. Reliable
 * provisional responses are defined in rfc3262. Reliable 18x responses are important for
 * interoperability with PTST world, and also bring advantages for early session negotiation.
 *
 * SipXtapi supports sending SDP offer in unreliable 18x responses, to enable unreliable early
 * audio for inbound calls. It is also capable of processing SDP answers in unreliable 18x responses.
 * This method is unreliable, because the 18x response could be lost, and cannot be used if late SDP
 * negotiation is employed, when SDP offer of caller not presented in INVITE.
 *
 * With reliable 18x responses, it is possible to send SDP offer in the reliable response itself.
 * Remote side then must send SDP answer in PRACK, and thus early session can be established. Or
 * if SDP offer was in PRACK, then SDP answer will be in PRACK response.
 *
 * 100rel support also enables usage of UPDATE method for negotiation of early session parameters.
 * So called "early-session" disposition type (rfc3959) is not supported. This however doesn't prevent
 * negotiation of early session parameters.
 */
typedef enum CP_100REL_CONFIG
{
   CP_100REL_PREFER_UNRELIABLE = 0, /**< Prefer sending unreliable 18x responses */
   CP_100REL_PREFER_RELIABLE,       /**< Prefer sending reliable 18x responses, if remote
                                     *   side supports it - default.
                                     */
   CP_100REL_REQUIRE_RELIABLE       /**< We will require support for 100rel when connecting
                                     *   new outbound calls, and prefer sending reliable 18x
                                     *   responses when possible for inbound calls.
                                     */
} CP_100REL_CONFIG;

/**
* We support 2 SDP offering modes - immediate and delayed. Immediate sends
* offer as soon as possible, to be able to receive early audio.
* Delayed offering sends SDP offer as late as possible. This saves media
* resources, in case lots of calls are made which might be rejected.
*/
typedef enum CP_SDP_OFFERING_MODE
{
   CP_SDP_OFFERING_IMMEDIATE = 0, /**
                                   * Offer SDP in the initial INVITE request.
                                   */
   CP_SDP_OFFERING_DELAYED = 1,   /**
                                  * Do not offer SDP in INVITE. SDP will be sent in ACK or PRACK for
                                  * outbound calls.
                                   */
} CP_SDP_OFFERING_MODE;

/**
* Configures how sipXcallLib should manage focus for calls. If call is focused then speaker and microphone
* are available for it.
*/
typedef enum CP_FOCUS_CONFIG
{
   CP_FOCUS_MANUAL = 0,   /**< Setting focus is completely manual, call will not be put in focus */
   CP_FOCUS_IF_AVAILABLE, /**< Focus call only if there is no other focused call */
   CP_FOCUS_ALWAYS        /**< Always focus new call, and defocus previously active call */
} CP_FOCUS_CONFIG;

/**
 * Defines types of notifications that can exist between call connections.
 */
typedef enum CP_NOTIFICATION_TYPE
{
   CP_NOTIFICATION_CONNECTION_STATE
} CP_NOTIFICATION_TYPE;

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

#endif // CpDefs_h__
