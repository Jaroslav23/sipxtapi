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

#ifndef XCpAbstractCall_h__
#define XCpAbstractCall_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMutex.h>
#include <os/OsRWMutex.h>
#include <os/OsSyncBase.h>
#include <os/OsServerTask.h>
#include <utl/UtlContainable.h>
#include <sdp/SdpCodecList.h>
#include <net/SipDialog.h>
#include <net/SipTagGenerator.h>
#include <cp/CpDefs.h>
#include <cp/CpNatTraversalConfig.h>
#include <cp/CpMediaInterfaceProvider.h>
#include <cp/CpMessageQueueProvider.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS
template <class T>
class OsPtrLock; // forward template class declaration
class SipDialog;
class SipMessage;
class SipUserAgent;
class SipMessageEvent;
class XSipConnection;
class XCpCallConnectionListener;
class CpMediaInterfaceFactory;
class CpMediaInterface;
class CpCallStateEventListener;
class XCpCallControl;
class SipInfoStatusEventListener;
class SipInfoEventListener;
class SipSecurityEventListener;
class CpMediaEventListener;
class CpRtpRedirectEventListener;
class AcAcceptConnectionMsg;
class AcRejectConnectionMsg;
class AcRedirectConnectionMsg;
class AcAnswerConnectionMsg;
class AcCommandMsg;
class AcDestroyConnectionMsg;
class AcNotificationMsg;
class AcGainFocusMsg;
class AcYieldFocusMsg;
class AcAudioBufferPlayMsg;
class AcAudioFilePlayMsg;
class AcAudioPausePlaybackMsg;
class AcAudioResumePlaybackMsg;
class AcAudioStopPlaybackMsg;
class AcAudioRecordStartMsg;
class AcAudioRecordStopMsg;
class AcAudioToneStartMsg;
class AcAudioToneStopMsg;
class AcMuteInputConnectionMsg;
class AcUnmuteInputConnectionMsg;
class AcLimitCodecPreferencesMsg;
class AcSubscribeMsg;
class AcUnsubscribeMsg;
class AcRejectTransferMsg;
class AcAcceptTransferMsg;
class AcTransferBlindMsg;
class AcHoldConnectionMsg;
class AcUnholdConnectionMsg;
class AcTransferConsultativeMsg;
class AcRenegotiateCodecsMsg;
class AcSendInfoMsg;
class CpTimerMsg;
class OsIntPtrMsg;
class AcTimerMsg;
class ScTimerMsg;
class ScCommandMsg;
class ScNotificationMsg;
class SipLineProvider;

/**
 * XCpAbstractCall is the top class for XCpConference and XCpCall providing
 * common functionality. This class can be stored in Utl containers.
 * Inherits from OsSyncBase, and can be locked externally. Locking the object with m_instanceRWMutex ensures
 * that it doesn't get deleted.
 *
 * Most public methods must acquire the object mutex m_instanceRWMutex first.
 *
 * Locking strategy:
 * - m_instanceRWMutex - used to implement methods of OsSyncBase. This is normally locked only for reading.
 * Write lock is used only when instance of this class is about to be deleted. This is meant only to be used
 * outside this class for automatic pointer locking.
 * - m_memberMutex - protects all members which are marked to be protected by it. This is the real mutex that
 * protects against member corruption due to parallel access. Members which require this mutex to be locked
 * must always be marked.
 *
 * Dialog matching:
 * - hasSipDialog - uses strict dialog matching. First we try to return connection with perfect dialog match.
 * Perfect dialog match means either established dialog match (callid + both tags), or match of an initial dialog
 * against an initial dialog (callid + 1 tag). Until such a perfect match is found, we have to go through all connections
 * and check each one for partial match. Partial match means callid + 1 tag match - either initial against established
 * or established against initial.
 * - findConnection - uses the same dialog matching like hasSipDialog
 * 
 */
class XCpAbstractCall : public OsServerTask, public UtlContainable, public OsSyncBase,
   protected CpMediaInterfaceProvider, protected CpMessageQueueProvider
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   static const UtlContainableType TYPE; /** < Class type used for runtime checking */ 

   /* ============================ CREATORS ================================== */

   XCpAbstractCall(const UtlString& sId,
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
                   XCpCallConnectionListener* pCallConnectionListener = NULL,
                   CpCallStateEventListener* pCallEventListener = NULL,
                   SipInfoStatusEventListener* pInfoStatusEventListener = NULL,
                   SipInfoEventListener* pInfoEventListener = NULL,
                   SipSecurityEventListener* pSecurityEventListener = NULL,
                   CpMediaEventListener* pMediaEventListener = NULL,
                   CpRtpRedirectEventListener* pRtpRedirectEventListener = NULL);

   virtual ~XCpAbstractCall();

   /* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean handleMessage(OsMsg& rRawMsg);

   /**
    * Starts the abstract call thread.
    */
   virtual UtlBoolean start();

   /** Connects call to given address. Uses supplied sip call-id. */
   virtual OsStatus connect(const UtlString& sSipCallId,
                            SipDialog& sipDialog,
                            const UtlString& toAddress,
                            const UtlString& fromAddress,
                            const UtlString& locationHeader,
                            CP_CONTACT_ID contactId,
                            SIP_TRANSPORT_TYPE transport,
                            CP_FOCUS_CONFIG focusConfig,
                            const UtlString& replacesField = NULL, // value of Replaces INVITE field
                            CP_CALLSTATE_CAUSE callstateCause = CP_CALLSTATE_CAUSE_NORMAL,
                            const SipDialog* pCallbackSipDialog = NULL) = 0;

   /** 
   * Accepts inbound call connection. Inbound connections can only be part of XCpCall
   *
   * Progress the connection from the OFFERING state to the
   * RINGING state. This causes a SIP 180 Ringing provisional
   * response to be sent.
   */
   virtual OsStatus acceptConnection(const SipDialog& sSipDialog,
                                     UtlBoolean bSendSDP,
                                     const UtlString& locationHeader,
                                     CP_CONTACT_ID contactId,
                                     SIP_TRANSPORT_TYPE transport);

   /**
   * Reject the incoming connection.
   *
   * Progress the connection from the OFFERING state to
   * the FAILED state with the cause of busy. With SIP this
   * causes a 486 Busy Here response to be sent.
   */
   virtual OsStatus rejectConnection(const SipDialog& sSipDialog);

   /**
   * Redirect the incoming connection.
   *
   * Progress the connection from the OFFERING state to
   * the FAILED state. This causes a SIP 302 Moved
   * Temporarily response to be sent with the specified
   * contact URI.
   */
   virtual OsStatus redirectConnection(const SipDialog& sSipDialog,
                                       const UtlString& sRedirectSipUrl);

   /**
   * Answer the incoming terminal connection.
   *
   * Progress the connection from the OFFERING or RINGING state
   * to the ESTABLISHED state and also creating the terminal
   * connection (with SIP a 200 OK response is sent).
   */
   virtual OsStatus answerConnection(const SipDialog& sSipDialog);

   /**
   * Accepts transfer request on given connection. Must be called
   * when in dialog REFER request is received to follow transfer.
   */
   virtual OsStatus acceptConnectionTransfer(const SipDialog& sipDialog);

   /**
   * Rejects transfer request on given connection. Must be called
   * when in dialog REFER request is received to reject transfer.
   */
   virtual OsStatus rejectConnectionTransfer(const SipDialog& sipDialog);

   /**
    * Disconnects given call with given sip call-id
    *
    * The appropriate disconnect signal is sent (e.g. with SIP BYE or CANCEL).  The connection state
    * progresses to disconnected and the connection is removed.
    */
   virtual OsStatus dropConnection(const SipDialog& sipDialog) = 0;

   /** Blind transfer given call to sTransferSipUrl. Works for simple call and call in a conference */
   virtual OsStatus transferBlind(const SipDialog& sipDialog,
                                  const UtlString& sTransferSipUrl);

   /**
    * Consultative transfer given call to target call. Works for simple call and call in a conference. 
    *
    * @param sourceSipDialog Source call identifier.
    * @param targetSipDialog Must be full SIP dialog with all fields initialized, not just callid and tags.
    */
   virtual OsStatus transferConsultative(const SipDialog& sourceSipDialog,
                                         const SipDialog& targetSipDialog);

  /**
   * Put the specified terminal connection on hold.
   *
   * Change the terminal connection state from TALKING to HELD.
   * (With SIP a re-INVITE message is sent with SDP indicating
   * no media should be sent.)
   */
   virtual OsStatus holdConnection(const SipDialog& sipDialog);

  /**
   * Convenience method to take the terminal connection off hold.
   *
   * Change the terminal connection state from HELD to TALKING.
   * (With SIP a re-INVITE message is sent with SDP indicating
   * media should be sent.)
   */
   virtual OsStatus unholdConnection(const SipDialog& sipDialog);

   /** Starts DTMF tone on call connection.*/
   OsStatus audioToneStart(int iToneId,
                           UtlBoolean bLocal,
                           UtlBoolean bRemote,
                           int duration);

   /** Stops DTMF tone on call connection */
   OsStatus audioToneStop();

   /** Starts playing audio file on call connection */
   OsStatus audioFilePlay(const UtlString& audioFile,
                          UtlBoolean bRepeat,
                          UtlBoolean bLocal,
                          UtlBoolean bRemote,
                          UtlBoolean bMixWithMic = FALSE,
                          int iDownScaling = 100,
                          void* pCookie = NULL);

   /** Starts playing audio buffer on call connection. Passed buffer will be copied internally. */
   OsStatus audioBufferPlay(const void* pAudiobuf,
                            size_t iBufSize,
                            int iType,
                            UtlBoolean bRepeat,
                            UtlBoolean bLocal,
                            UtlBoolean bRemote,
                            UtlBoolean bMixWithMic = FALSE,
                            int iDownScaling = 100,
                            void* pCookie = NULL);

   /** Stops playing audio file or buffer on call connection */
   OsStatus audioStopPlayback();

   /** Pauses audio playback of file or buffer. */
   OsStatus pauseAudioPlayback();

   /** Resumes audio playback of file or buffer */
   OsStatus resumeAudioPlayback();

   /** Starts recording call or conference. */
   OsStatus audioRecordStart(const UtlString& sFile);

   /** Stops recording call or conference. */
   OsStatus audioRecordStop();

   /**
   * Enables discarding of inbound RTP at bridge for given call
   * or conference. Useful for server applications without mic/speaker.
   * DTMF on given call will still be decoded.
   */
   OsStatus muteInputConnection(const SipDialog& sipDialog);

   /**
   * Disables discarding of inbound RTP for given call
   * or conference. Useful for server applications without mic/speaker.
   */
   OsStatus unmuteInputConnection(const SipDialog& sipDialog);

   /**
   * Rebuild codec factory of the call (media interface) on the fly with new audio
   * codec requirements and new video codecs. Preferences will be in effect after the next
   * INVITE or re-INVITE. Can be called on empty call or conference to limit
   * codecs for future calls. When called on an established call, hold/unhold
   * or codec renegotiation needs to be triggered to actually change codecs.
   * If used on conference, codecs will be applied to all future calls, and all
   * calls that are unheld.
   *
   * This method doesn't affect codec factory used for new outbound/inbound calls.
   */
   OsStatus limitCodecPreferences(const UtlString& sAudioCodecs,
                                  const UtlString& sVideoCodecs);

   /**
   * Rebuild codec factory on the fly with new audio codec requirements
   * and one specific video codec.  Renegotiate the codecs to be use for the
   * specified terminal connection.
   *
   * This is typically performed after a capabilities change for the
   * terminal connection (for example, addition or removal of a codec type).
   * (Sends a SIP re-INVITE.)
   */
   virtual OsStatus renegotiateCodecsConnection(const SipDialog& sipDialog,
                                                const UtlString& sAudioCodecs,
                                                const UtlString& sVideoCodecs);


   /** Sends an INFO message to the other party(s) on the call */
   virtual OsStatus sendInfo(const SipDialog& sipDialog,
                             const UtlString& sContentType,
                             const char* pContent,
                             const size_t nContentLength,
                             void* pCookie);

   /**
   * Subscribe for given notification type with given target sip call.
   * ScNotificationMsg messages will be sent to callbackSipDialog.
   */
   OsStatus subscribe(CP_NOTIFICATION_TYPE notificationType,
                      const SipDialog& targetSipDialog,
                      const SipDialog& callbackSipDialog);

   /**
   * Unsubscribes for given notification type with given target sip call.
   */
   OsStatus unsubscribe(CP_NOTIFICATION_TYPE notificationType,
                        const SipDialog& targetSipDialog,
                        const SipDialog& callbackSipDialog);

   /** Acquires exclusive lock on instance. Use only when deleting. It is never released. */
   virtual OsStatus acquireExclusive();

   /* ============================ ACCESSORS ================================= */

   /**
   * Calculate a unique hash code for this object.  If the equals
   * operator returns true for another object, then both of those
   * objects must return the same hashcode.
   */    
   virtual unsigned hash() const;

   /**
   * Get the ContainableType for a UtlContainable derived class.
   */
   virtual UtlContainableType getContainableType() const;

   /**
    * Gets Id of the abstract call.
    */
   UtlString getId() const;

   /* ============================ INQUIRY =================================== */

   /**
   * Compare the this object to another like-objects.  Results for 
   * designating a non-like object are undefined.
   *
   * @returns 0 if equal, < 0 if less then and >0 if greater.
   */
   virtual int compareTo(UtlContainable const* inVal) const;

   /**
    * Checks if this abstract call has given sip dialog. Uses strict dialog matching.
    */
   virtual SipDialog::DialogMatchEnum hasSipDialog(const SipDialog& sipDialog) const = 0;

   /** Gets the number of sip connections in this call */
   virtual int getCallCount() const = 0;

   /** gets remote user agent for call or conference */
   OsStatus getRemoteUserAgent(const SipDialog& sipDialog,
                               UtlString& userAgent) const;

   /** Gets internal id of media connection for given call or conference. Only for unit tests */
   OsStatus getMediaConnectionId(const SipDialog& sipDialog,
                                 int& mediaConnID) const;

   /** Gets copy of SipDialog for given call. Uses strict dialog matching. */
   OsStatus getSipDialog(const SipDialog& sipDialog,
                         SipDialog& sOutputSipDialog) const;

   /** Checks if given call exists and is established. */
   UtlBoolean isConnectionEstablished(const SipDialog& sipDialog) const;

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   /** Handles command messages */
   virtual UtlBoolean handleCommandMessage(const AcCommandMsg& rRawMsg);

   /** Handles command messages */
   virtual UtlBoolean handleNotificationMessage(const AcNotificationMsg& rRawMsg);

   /** Handles timer messages */
   virtual UtlBoolean handleTimerMessage(const CpTimerMsg& rRawMsg);

   /** Handler for inbound SipMessageEvent messages. */
   virtual OsStatus handleSipMessageEvent(const SipMessageEvent& rSipMsgEvent);

   /** Finds connection handling given Sip dialog. Uses loose dialog matching. */
   virtual UtlBoolean findConnection(const SipDialog& sipDialog, OsPtrLock<XSipConnection>& ptrLock) const = 0;

   /** Finds connection handling given Sip message. Uses loose dialog matching. */
   virtual UtlBoolean findConnection(const SipMessage& sipMessage, OsPtrLock<XSipConnection>& ptrLock) const;

   /** Tries to gain focus on this call asynchronously through call manager. */
   OsStatus gainFocus(UtlBoolean bGainOnlyIfNoFocusedCall = FALSE);

   /** Tries to yield focus on this call asynchronously through call manager */
   OsStatus yieldFocus();

   /**
    * Called to notify call stack that we have new sip call-id to route messages to.
    * May not be called while holding any locks.
    */
   void onConnectionAddded(const UtlString& sSipCallId);

   /**
    * Called to notify call stack that a sip call-id is no longer valid.
    * May not be called while holding any locks.
    */
   void onConnectionRemoved(const UtlString& sSipCallId);

   /** Handle call timer notification. When overriding, first call parent */
   virtual UtlBoolean handleCallTimer(const AcTimerMsg& timerMsg);

   /**
   * Releases media interface.
   *
   * Must be called from the OsServerTask only or destructor.
   */
   void releaseMediaInterface();

   /**
   * Gets current media interface of abstract call. Creates one if it doesn't exist.
   * 
   * Must be called from the OsServerTask only.
   */
   virtual CpMediaInterface* getMediaInterface(UtlBoolean bCreateIfNull = TRUE);

   /**
   * Gets local call queue for sending messages.
   */       
   virtual OsMsgQ& getLocalQueue();

   /**
   * Gets global queue for inter call communication.
   */       
   virtual OsMsgQ& getGlobalQueue();

   /**
    * Destroys XSipConnection if it exists by sip dialog. This should be called
    * after call has been disconnected and connection is ready to be deleted.
    */
   virtual void destroySipConnection(const SipDialog& sSipDialog) = 0;

   /** Limits codec preferences for future renegotiations */
   OsStatus doLimitCodecPreferences(const UtlString& sAudioCodecs, const UtlString& sVideoCodecs);

   /** 
    * Discovers real line identity for new inbound call. Returns full line url. This is
    * needed for inbound calls, since request uri can be different than to url after redirect,
    * and may be slightly different than identity of locally defined line (for example by display name).
    */
   UtlString getRealLineIdentity(const SipMessage& sipRequest) const;

   static const int CALL_MAX_REQUEST_MSGS;

   mutable OsMutex m_memberMutex; ///< mutex for member synchronization
   // begin of members requiring m_memberMutex
   // end of members requiring m_memberMutex

   // no mutex required, used only from OsServerTask
   CpMediaInterface* m_pMediaInterface; ///< media interface handling RTP

   // thread safe
   SipTagGenerator m_sipTagGenerator; ///< generator for sip tags
   const UtlString m_sId; ///< unique identifier of the abstract call
   SipUserAgent& m_rSipUserAgent; // for sending sip messages
   XCpCallControl& m_rCallControl; ///< interface for managing other calls
   CpMediaInterfaceFactory& m_rMediaInterfaceFactory; // factory for creating CpMediaInterface
   OsMsgQ& m_rCallManagerQueue; ///< message queue of call manager
   const SdpCodecList m_rDefaultSdpCodecList; ///< default SdpCodec factory for new calls. Independent of codec list of call manager.
   const SipLineProvider* m_pSipLineProvider; // read only functionality of line manager

   // thread safe, atomic
   CP_FOCUS_CONFIG m_focusConfig; ///< configuration of media focus policy

   // set only once and thread safe
   XCpCallConnectionListener* m_pCallConnectionListener; ///< listener for updating call stack
   CpCallStateEventListener* m_pCallEventListener; ///< listener for firing call events
   SipInfoStatusEventListener* m_pInfoStatusEventListener; ///< listener for firing info status events
   SipInfoEventListener* m_pInfoEventListener; ///< listener for firing info message events
   SipSecurityEventListener* m_pSecurityEventListener; ///< listener for firing security events
   CpMediaEventListener* m_pMediaEventListener; ///< listener for firing media events
   CpRtpRedirectEventListener* m_pRtpRedirectEventListener; ///< listener for firing rtp redirect events
   const CpNatTraversalConfig m_natTraversalConfig; ///< NAT traversal configuration
   const UtlString m_sBindIpAddress; ///< default local IP for media interface. May be 0.0.0.0
   const UtlString m_sLocale; ///< locale for DTMF, empty by default
   const int m_sessionTimerExpiration; ///< time between RFC4028 session refreshes
   const CP_SESSION_TIMER_REFRESH m_sessionTimerRefresh; ///< type of refresh to use with session timer
   const CP_SIP_UPDATE_CONFIG m_updateSetting; ///< configuration of SIP UPDATE method
   const CP_100REL_CONFIG m_100relSetting; ///< configuration of 100rel support
   const CP_SDP_OFFERING_MODE m_sdpOfferingMode; ///< SDP offering mode configuration. Late or immediate.
   const int m_inviteExpiresSeconds; ///< time in which INVITE must be answered with final response in seconds

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Handles gain focus command from call manager. Never use directly, go through call manager. */
   OsStatus handleGainFocus(const AcGainFocusMsg& rMsg);
   /** Handles defocus command from call manager. Never use directly, go through call manager. */
   OsStatus handleDefocus(const AcYieldFocusMsg& rMsg);
   /** Handles command to start playing buffer on call */
   OsStatus handleAudioBufferPlay(const AcAudioBufferPlayMsg& rMsg);
   /** Handles command to start playing file on call */
   OsStatus handleAudioFilePlay(const AcAudioFilePlayMsg& rMsg);
   /** Handles command to pause playing file/buffer on call */
   OsStatus handleAudioPausePlayback(const AcAudioPausePlaybackMsg& rMsg);
   /** Handles command to resume playing file/buffer on call */
   OsStatus handleAudioResumePlayback(const AcAudioResumePlaybackMsg& rMsg);
   /** Handles command to stop playing file/buffer on call */
   OsStatus handleAudioStopPlayback(const AcAudioStopPlaybackMsg& rMsg);
   /** Handles command to start recording call */
   OsStatus handleAudioRecordStart(const AcAudioRecordStartMsg& rMsg);
   /** Handles command to stop recording call */
   OsStatus handleAudioRecordStop(const AcAudioRecordStopMsg& rMsg);
   /** Handles command to send audio DTMF (in-band or rfc8233)*/
   OsStatus handleAudioToneStart(const AcAudioToneStartMsg& rMsg);
   /** Handles command to stop sending audio DTMF */
   OsStatus handleAudioToneStop(const AcAudioToneStopMsg& rMsg);
   /** Handles message to mute inbound RTP in audio bridge */
   OsStatus handleMuteInputConnection(const AcMuteInputConnectionMsg& rMsg);
   /** Handles message to unmute inbound RTP in audio bridge */
   OsStatus handleUnmuteInputConnection(const AcUnmuteInputConnectionMsg& rMsg);
   /** Handles message to limit codec preferences for future sip connections */
   OsStatus handleLimitCodecPreferences(const AcLimitCodecPreferencesMsg& rMsg);
   /** Handles message to subscribe to notifications */
   OsStatus handleSubscribe(const AcSubscribeMsg& rMsg);
   /** Handles message to unsubscribe from notifications */
   OsStatus handleUnsubscribe(const AcUnsubscribeMsg& rMsg);
   /** Handles message to accept call transfer */
   OsStatus handleAcceptTransfer(const AcAcceptTransferMsg& rMsg);
   /** Handles message to reject call transfer */
   OsStatus handleRejectTransfer(const AcRejectTransferMsg& rMsg);
   /** Handler for OsMsg::PHONE_APP messages */
   UtlBoolean handlePhoneAppMessage(const OsMsg& rRawMsg);
   /** Handle media connection notification message */
   UtlBoolean handleConnectionNotfMessage(const OsIntPtrMsg& rMsg);
   /** Handle media interface notification message */
   UtlBoolean handleInterfaceNotfMessage(const OsIntPtrMsg& rMsg);
   /** Handles message to destroy sip connection */
   virtual OsStatus handleDestroyConnection(const AcDestroyConnectionMsg& rMsg) = 0;
   /** Handles message to accept inbound sip connection */
   OsStatus handleAcceptConnection(const AcAcceptConnectionMsg& rMsg);
   /** Handles message to reject inbound sip connection */
   OsStatus handleRejectConnection(const AcRejectConnectionMsg& rMsg);
   /** Handles message to redirect inbound sip connection */
   OsStatus handleRedirectConnection(const AcRedirectConnectionMsg& rMsg);
   /** Handles message to answer inbound sip connection */
   OsStatus handleAnswerConnection(const AcAnswerConnectionMsg& rMsg);
   /** Handles message to initiate blind call transfer */
   OsStatus handleTransferBlind(const AcTransferBlindMsg& rMsg);
   /** Handles message to initiate consultative call transfer */
   OsStatus handleTransferConsultative(const AcTransferConsultativeMsg& rMsg);
   /** Handles message to initiate remote hold on sip connection */
   OsStatus handleHoldConnection(const AcHoldConnectionMsg& rMsg);
   /** Handles message to initiate remote unhold on sip connection */
   OsStatus handleUnholdConnection(const AcUnholdConnectionMsg& rMsg);
   /** Handles message to renegotiate codecs for some sip connection */
   OsStatus handleRenegotiateCodecs(const AcRenegotiateCodecsMsg& rMsg);
   /** Handles message to send SIP INFO to on given sip connection */
   OsStatus handleSendInfo(const AcSendInfoMsg& rMsg);

   /** Fires given media interface event to listeners. */
   virtual void fireSipXMediaInterfaceEvent(CP_MEDIA_EVENT event,
                                            CP_MEDIA_CAUSE cause,
                                            CP_MEDIA_TYPE type,
                                            intptr_t pEventData1,
                                            intptr_t pEventData2) = 0;

   /** Finds the correct connection by mediaConnectionId and fires media event for it. */
   virtual void fireSipXMediaConnectionEvent(CP_MEDIA_EVENT event,
                                             CP_MEDIA_CAUSE cause,
                                             CP_MEDIA_TYPE type,
                                             int mediaConnectionId,
                                             intptr_t pEventData1,
                                             intptr_t pEventData2) = 0;

   /** Called when media focus is gained (speaker and mic are engaged) */
   virtual void onFocusGained() = 0;

   /** Called when media focus is lost (speaker and mic are disengaged) */
   virtual void onFocusLost() = 0;

   /** Called when abstract call thread is started */
   virtual void onStarted() = 0;

   /** Block until the sync object is acquired. Timeout is not supported! */
   virtual OsStatus acquire(const OsTime& rTimeout = OsTime::OS_INFINITY);

   /** Conditionally acquire the semaphore (i.e., don't block) */
   virtual OsStatus tryAcquire();

   /** Release the sync object */
   virtual OsStatus release();

   /** Handles sip connection timer notification. Lets sip connection to handle the timer. */
   UtlBoolean handleSipConnectionTimer(const ScTimerMsg& timerMsg);

   /** Handler for CpMessageTypes::SC_COMMAND messages */
   UtlBoolean handleSipConnectionCommandMessage(const ScCommandMsg& rMsg);

   /** Handler for CpMessageTypes::ScNotificationMsg messages */
   UtlBoolean handleSipConnectionNotificationMessage(const ScNotificationMsg& rMsg);

   XCpAbstractCall(const XCpAbstractCall& rhs);

   XCpAbstractCall& operator=(const XCpAbstractCall& rhs);
   mutable OsRWMutex m_instanceRWMutex; ///< mutex for guarding instance against deletion from call manager
};

#endif // XCpAbstractCall_h__
