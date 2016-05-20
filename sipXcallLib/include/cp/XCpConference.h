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

#ifndef XCpConference_h__
#define XCpConference_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlSList.h>
#include <cp/XCpAbstractCall.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS
class AcConnectMsg;
class AcDropConnectionMsg;
class AcDropAllConnectionsMsg;
class AcHoldAllConnectionsMsg;
class AcUnholdAllConnectionsMsg;
class AcRenegotiateCodecsAllMsg;
class AcConferenceJoinMsg;
class AcConferenceSplitMsg;
class CpConferenceEventListener;
class XCpCallLookup;

/**
 * XCpConference wraps several XSipConnections realizing conference functionality. XCpConference
 * is designed to hold multiple XSipConnections, sharing single media session. Conference cannot
 * accept inbound calls, those are routed to XCpCall automatically. Therefore methods related to
 * inbound call processing are not implemented.
 *
 * Call management functions are asynchronous, and post message to the OsServerTask thread. This is
 * useful because sending a sip message can take more time, and we don't want to block the call manager
 * during that time. Also audio functionality can take several 10ms, or even 100ms if media task is
 * getting overloaded.
 * Audio functionality is synchronous, except for focus gain/yield. Focus gain/yield always needs to go through
 * XCpCallManager, since we need a single place to manage focus. It will also ensure that no attept to gain
 * focus is made before media interface is created.
 *
 * Locking strategy:
 * @see XCpAbstractCall
 * - local members that require locking are always after comment mentioning which mutex needs to be locked
 * - const members and pointers/references set from constructor do not require locking, they are deleted by
 *   caller at suitable time
 *
 * This class should have absolute minimum knowledge about sip, all sip communication is done in XSipConnection.
 */
class XCpConference : public XCpAbstractCall
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   XCpConference(const UtlString& sId,
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
                 XCpCallConnectionListener* pCallConnectionListener = NULL,
                 CpCallStateEventListener* pCallEventListener = NULL,
                 SipInfoStatusEventListener* pInfoStatusEventListener = NULL,
                 SipInfoEventListener* pInfoEventListener = NULL,
                 SipSecurityEventListener* pSecurityEventListener = NULL,
                 CpMediaEventListener* pMediaEventListener = NULL,
                 CpRtpRedirectEventListener* pRtpRedirectEventListener = NULL,
                 CpConferenceEventListener* pConferenceEventListener = NULL);

   virtual ~XCpConference();

   /* ============================ MANIPULATORS ============================== */

   /** Connects call to given address. Uses supplied sip call-id. */
   virtual OsStatus connect(const UtlString& sipCallId,
                            SipDialog& sipDialog,
                            const UtlString& toAddress,
                            const UtlString& fromAddress,
                            const UtlString& locationHeader,
                            CP_CONTACT_ID contactId,
                            SIP_TRANSPORT_TYPE transport,
                            CP_FOCUS_CONFIG focusConfig,
                            const UtlString& replacesField = NULL, // value of Replaces INVITE field
                            CP_CALLSTATE_CAUSE callstateCause = CP_CALLSTATE_CAUSE_NORMAL,
                            const SipDialog* pCallbackSipDialog = NULL);

   /**
   * Disconnects given call with given sip call-id
   *
   * The appropriate disconnect signal is sent (e.g. with SIP BYE or CANCEL).  The connection state
   * progresses to disconnected and the connection is removed.
   */
   virtual OsStatus dropConnection(const SipDialog& sipDialog);

   /** Disconnects all calls */
   OsStatus dropAllConnections(UtlBoolean bDestroyConference = FALSE);

   /**
   * Convenience method to put all of the terminal connections in
   * the specified conference on hold.
   */
   OsStatus holdAllConnections();

   /**
   * Convenience method to take all of the terminal connections in
   * the specified conference off hold.
   *
   * Change the terminal connection state from HELD to TALKING.
   * (With SIP a re-INVITE message is sent with SDP indicating
   * media should be sent.)
   */
   OsStatus unholdAllConnections();

   /**
   * Rebuild codec factory on the fly with new audio codec requirements
   * and one specific video codec. Convenience method to renegotiate the codecs
   * for all of the terminal connections in the specified conference.
   *
   * This is typically performed after a capabilities change for the
   * terminal connection (for example, addition or removal of a codec type).
   * (Sends a SIP re-INVITE.)
   */
   OsStatus renegotiateCodecsAllConnections(const UtlString& sAudioCodecs,
                                            const UtlString& sVideoCodecs);

   /**
    * Joins given call with this conference. XSipConnection will be extracted from
    * given call and moved to this conference. Then codec renegotiation will be
    * triggered and call shell destroyed.
    */
   OsStatus join(const SipDialog& sipDialog);

   /**
    * Splits XSipConnection with given sip dialog into a new existing call with given id.
    */
   OsStatus split(const SipDialog& sipDialog,
                  const UtlString& sNewCallId);

   /* ============================ ACCESSORS ================================= */

   void getConferenceUri(Url& conferenceUri) const { conferenceUri = m_conferenceUri; }

   /* ============================ INQUIRY =================================== */

   /**
   * Checks if this conference has given sip dialog. Uses strict dialog matching.
   */
   virtual SipDialog::DialogMatchEnum hasSipDialog(const SipDialog& sipDialog) const;

   /** Gets the number of sip connections in this call */
   virtual int getCallCount() const;

   /** Gets sip call-id of conference if its available */
   OsStatus getConferenceSipCallIds(UtlSList& sipCallIdList) const;

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /** Finds connection handling given Sip dialog. Uses loose dialog matching. */
   virtual UtlBoolean findConnection(const SipDialog& sipDialog, OsPtrLock<XSipConnection>& ptrLock) const;

   /** Handles command messages */
   virtual UtlBoolean handleCommandMessage(const AcCommandMsg& rRawMsg);

   /** Handles command messages */
   virtual UtlBoolean handleNotificationMessage(const AcNotificationMsg& rRawMsg);

   /** Handles timer messages */
   virtual UtlBoolean handleTimerMessage(const CpTimerMsg& rRawMsg);

   /** Handler for inbound SipMessageEvent messages. */
   virtual OsStatus handleSipMessageEvent(const SipMessageEvent& rSipMsgEvent);

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   XCpConference(const XCpConference& rhs);

   XCpConference& operator=(const XCpConference& rhs);

   /** Handles message to create new sip connection for call */
   OsStatus handleConnect(const AcConnectMsg& rMsg);
   /** Handles message to drop sip connection */
   OsStatus handleDropConnection(const AcDropConnectionMsg& rMsg);
   /** Handles message to destroy sip connection */
   virtual OsStatus handleDestroyConnection(const AcDestroyConnectionMsg& rMsg);
   /** Handles message to drop all sip connections */
   OsStatus handleDropAllConnections(const AcDropAllConnectionsMsg& rMsg);
   /** Handles message to initiate remote hold on all sip connections */
   OsStatus handleHoldAllConnections(const AcHoldAllConnectionsMsg& rMsg);
   /** Handles message to initiate remote unhold on all sip connection */
   OsStatus handleUnholdAllConnections(const AcUnholdAllConnectionsMsg& rMsg);
   /** Handles message to renegotiate codecs for all sip connections */
   OsStatus handleRenegotiateCodecsAll(const AcRenegotiateCodecsAllMsg& rMsg);
   /** Handles message to join a call with this conference */
   OsStatus handleJoin(const AcConferenceJoinMsg& rMsg);
   /** Handles message to split a call from this conference */
   OsStatus handleSplit(const AcConferenceSplitMsg& rMsg);

   /**
    * Finds connection handling given Sip dialog. Uses loose dialog matching.
    * Assumes external lock on m_memberMutex.
    */
   XSipConnection* findConnection(const SipDialog& sipDialog) const;

   /**
    * Destroys all XSipConnection objects. Meant to be called during destruction.
    */
   void destroyAllSipConnections();

   /**
    * Destroys XSipConnection if it exists by sip dialog. This should be called
    * after call has been disconnected and connection is ready to be deleted.
    */
   void destroySipConnection(const SipDialog& sSipDialog, UtlBoolean bFireEvents);

   /**
    * Destroys XSipConnection if it exists by sip dialog. This should be called
    * after call has been disconnected and connection is ready to be deleted.
    * Conference call removed event will be fired.
    */
   virtual void destroySipConnection(const SipDialog& sSipDialog);

   /** Creates new XSipConnection for the call, if it doesn't exist yet */
   void createSipConnection(const SipDialog& sipDialog, const UtlString& sFullLineUrl, UtlBoolean bFireEvents = TRUE);

   /** Finds the correct connection by mediaConnectionId and fires media event for it. */
   virtual void fireSipXMediaConnectionEvent(CP_MEDIA_EVENT event,
                                             CP_MEDIA_CAUSE cause,
                                             CP_MEDIA_TYPE type,
                                             int mediaConnectionId,
                                             intptr_t pEventData1,
                                             intptr_t pEventData2);

   /** Fires given media interface event to listeners. */
   virtual void fireSipXMediaInterfaceEvent(CP_MEDIA_EVENT event,
                                            CP_MEDIA_CAUSE cause,
                                            CP_MEDIA_TYPE type,
                                            intptr_t pEventData1,
                                            intptr_t pEventData2);

   /** Fires given conference event to listeners */
   void fireConferenceEvent(CP_CONFERENCE_EVENT event,
                            CP_CONFERENCE_CAUSE cause,
                            const SipDialog* pSipDialog = NULL);

   /** Called when media focus is gained (speaker and mic are engaged) */
   virtual void onFocusGained();

   /** Called when media focus is lost (speaker and mic are disengaged) */
   virtual void onFocusLost();

   /** Called when abstract call thread is started */
   virtual void onStarted();

   /**
    * Request the conference to be destroyed by call manager.
    */
   void requestConferenceDestruction();

   // begin of members requiring m_memberMutex
   UtlSList m_sipConnections;
   // end of members requiring m_memberMutex

   // thread safe, atomic
   UtlBoolean m_bDestroyConference; ///< flag set when dropping all connections, if also conference should be destroyed

   // set only once and thread safe
   CpConferenceEventListener* m_pConferenceEventListener; ///< listener for firing conference events
   XCpCallLookup& m_rCallLookup; ///< used to lookup other calls by abstractCallId
   const Url m_conferenceUri; ///< sip uri of public conference. Used for matching inbound calls. Only valid for public conferences.
};

#endif // XCpConference_h__
