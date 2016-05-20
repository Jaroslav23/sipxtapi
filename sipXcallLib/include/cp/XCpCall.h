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

#ifndef XCpCall_h__
#define XCpCall_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
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
class AcStartRtpRedirectMsg;
class AcStopRtpRedirectMsg;
class AcDropConnectionMsg;
class AcDestroyConnectionMsg;
class XCpConference;

/**
 * XCpCall wraps XSipConnection realizing all call functionality. XCpCall is designed to hold
 * only single XSipConnection, and corresponds to a sip connection and media session.
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
class XCpCall : public XCpAbstractCall
{
   friend class XCpConference; // needed for conference split/join

   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   XCpCall(const UtlString& sId,
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

   virtual ~XCpCall();

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
   * Starts redirecting call RTP. Both calls will talk directly to each other, but we keep
   * control of SIP signaling. Current call will become the master call.
   */
   OsStatus startCallRedirectRtp(const UtlString& slaveAbstractCallId,
                                 const SipDialog& slaveSipDialog);

   /**
   * stops redirecting call RTP. Will cancel RTP redirection for both calls participating in it.
   */
   OsStatus stopCallRedirectRtp();

   /**
   * Disconnects given call with given sip call-id
   *
   * The appropriate disconnect signal is sent (e.g. with SIP BYE or CANCEL).  The connection state
   * progresses to disconnected and the connection is removed.
   */
   virtual OsStatus dropConnection(const SipDialog& sipDialog);

   /** Disconnects call without knowing the sip call-id*/
   OsStatus dropConnection();

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /**
   * Checks if this call has given sip dialog. Uses strict dialog matching.
   */
   virtual SipDialog::DialogMatchEnum hasSipDialog(const SipDialog& sipDialog) const;

   /** Gets the number of sip connections in this call */
   virtual int getCallCount() const;

   /** Gets sip call-id of call if its available */
   OsStatus getCallSipCallId(UtlString& sSipCallId) const;

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /**
    * Finds connection handling given Sip dialog. Uses loose dialog matching.
    */
   virtual UtlBoolean findConnection(const SipDialog& sipDialog, OsPtrLock<XSipConnection>& ptrLock) const;

   /** Gets Sip connection of the call if there is any */
   virtual UtlBoolean getConnection(OsPtrLock<XSipConnection>& ptrLock) const;

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
   XCpCall(const XCpCall& rhs);

   XCpCall& operator=(const XCpCall& rhs);

   /** Handles message to create new sip connection for call */
   OsStatus handleConnect(const AcConnectMsg& rMsg);
   /** Handles message to start RTP redirect */
   OsStatus handleStartRtpRedirect(const AcStartRtpRedirectMsg& rMsg);
   /** Handles message to stop RTP redirect */
   OsStatus handleStopRtpRedirect(const AcStopRtpRedirectMsg& rMsg);
   /** Handles message to drop sip connection */
   OsStatus handleDropConnection(const AcDropConnectionMsg& rMsg);
   /** Handles message to destroy sip connection */
   virtual OsStatus handleDestroyConnection(const AcDestroyConnectionMsg& rMsg);

   /** Creates new XSipConnection for the call, if it doesn't exist yet */
   void createSipConnection(const SipDialog& sipDialog, const UtlString& sFullLineUrl);

   /** Destroys XSipConnection if it exists */
   void destroySipConnection();

   /**
    * Request the call to be destroyed by call manager.
    */
   void requestCallDestruction();

   /**
    * Destroys XSipConnection if it exists by sip dialog. This should be called
    * after call has been disconnected and connection is ready to be deleted.
    */
   virtual void destroySipConnection(const SipDialog& sSipDialog);

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

   /** Called when media focus is gained (speaker and mic are engaged) */
   virtual void onFocusGained();

   /** Called when media focus is lost (speaker and mic are disengaged) */
   virtual void onFocusLost();

   /** Called when abstract call thread is started */
   virtual void onStarted();

   /**
    * Tries to extract sip connection from call. Meant to be used by XCpConference for
    * join. Reference to queue and media interface, callId will need to be updated by caller.
    * XCpCall thread must not be running at the time this operation executes.
    * After this operation call will have no sip connection.
    */
   OsStatus extractConnection(XSipConnection **pSipConnection);

   /**
    * Tries to set sip connection on the call. Meant to be used by XCpConference for
    * split. Reference to queue and media interface will be updated if operation succeeds.
    * XCpCall thread must not be running at the time this operation executes.
    * Operation will fail if call already has some sip connection.
    */
   OsStatus setConnection(XSipConnection *pSipConnection);


   // begin of members requiring m_memberMutex
   XSipConnection* m_pSipConnection; ///< XSipConnection handling Sip messages. Use destroySipConnection to delete it.
   // end of members requiring m_memberMutex
};

#endif // XCpCall_h__
