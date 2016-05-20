// 
// Copyright (C) 2005-2006 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// Author: Dan Petrie (dpetrie AT SIPez DOT com)

#ifndef _CpMediaInterface_h_
#define _CpMediaInterface_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <os/OsDefs.h>
#include <os/OsProtectEvent.h>
#include <os/OsMsgQ.h>
#include <os/OsDatagramSocket.h>
#include <net/SipContactDb.h>
#include <net/SdpBody.h>
#include <os/IStunSocket.h>
//#include <tapi/sipXtapi.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SdpCodec;
class SdpCodecList;
class CpMediaInterfaceFactory;
class OsMsgDispatcher;

/** 
 * @brief Abstract media control interface.
 * 
 * The CpCallManager creates a CpMediaInterface for each call created.
 * The media interface is then used to control and query the media sub-system 
 * used for that call.  As connections are added to the call, the 
 * media interface is used to add those connections to the media control system 
 * such that all connections in that call are bridged together.
 * 
 * @note This abstract class must be sub-classed and implemented to replace 
 *       the default media sub-system.
 */
class CpMediaInterface : public UtlInt
{
/* //////////////////////////// PUBLIC //////////////////////////// */
public:
   enum
   {
      INVALID_CONNECTION_ID = -1 ///< Id of invalid media connection
   };

/* =========================== CREATORS =========================== */

     /// @brief Default constructor
   CpMediaInterface(CpMediaInterfaceFactory *pFactoryImpl);

/* ========================= DESTRUCTORS ========================== */

protected:
     /// @brief Protected Destructor so that we can manage media 
     ///        interface pointers.
   virtual ~CpMediaInterface();
public:

     /// @brief public interface for destroying this media interface
   virtual void release() = 0; 

/* ========================= MANIPULATORS ========================= */
     /// @brief Create a media connection in the media processing subsystem.
   virtual OsStatus createConnection(int& connectionId, ///< will get assigned connection id
                                     const char* szLocalIPAddress, ///< optionally override local bind address of media interface
                                     int localPort = 0,
                                     void* videoWindowHandle = NULL,
                                     void* const pSecurityAttributes = NULL,
                                     OsMsgQ* pConnectionNotificationQueue = NULL,
                                     const RtpTransportOptions rtpTransportOptions = RTP_TRANSPORT_UDP) = 0;
     /**<
     *  One instance of the CpMediaInterface exists for each call, however, 
     *  each leg of the call requires in individual connection.
     *
     *  @param[out] connectionId - A newly allocated connection id returned via 
     *              this call.  The connection passed to many other media 
     *              processing methods in this interface.
     *  @param[in]  szLocalIPAddress - Local address (interface) that should 
     *              be used for this connection.
     *  @param[in]  localPort - Local port that should be used for this
     *              connection.
     *              Note, that in fact two ports will be allocated - 
     *              (localPort) for RTP and (localPort+1) for RTCP.
     *              If 0 is passed, port number will be selected automatically.
     *  @param[in]  videoWindowHandle - Video Window handle if using video.  
     *              Supply a window handle of \c NULL to disable video for this 
     *              call/connection.
     *  @param      pSecurityAttributes - Pointer to a 
     *              SIPXVE_SECURITY_ATTRIBUTES object.  
     *  @param      pSocketIdleSink - <<UNKNOWN -- What does this do? 
     *              -- kkyzivat 20070801 >>
     *  @param      pMediaEventListener - <<UNKNOWN -- What does this do?
     *              -- kkyzivat 20070801 >>
     *  @param[in]  rtpTransportOptions RTP_TRANSPORT_UDP, RTP_TRANSPORT_TCP, 
     *              or BOTH
     *  @retval     UNKNOWN - << TODO: Add useful return values here - i.e.
     *              failure codes to expect, etc. -- kkyzivat 20070801 >>
     */ 

   virtual void setInterfaceNotificationQueue(OsMsgQ* pInterfaceNotificationQueue) = 0;

     /**<
     *  Gives the Media Interface an object to help in the dispatching of 
     *  notification messages to users of the media interface.  Users
     *  are free to subclass OsMsgDispatcher to filter messages as
     *  they see fit, and should hold on to it to receive their messages.
     *  
     *  @param[in] pNoteDisper - A notification dispatcher to give to the MI.
     *  @retval Pointer to the previous media notification dispatcher set in this MI -
     *          If there was no previous media notification dispatcher, NULL is returned.
     */

     /// @brief Enable or disable media notifications for one/all resource(s).
   virtual OsStatus setMediaNotificationsEnabled(bool enabled, 
                                                 const UtlString& resourceName = NULL) = 0;
     /**<
     *  Enable or disable media notifications for a given resource or all resources.
     *
     *  @NOTE If /p NULL is passed for resource name, then all resources 
     *        will have all notifications enabled/disabled
     *  
     *  @param[in] enabled - Whether notification type is to be enabled or disabled.
     *  @param[in] resourceName - the name of the resource to have notifications 
     *             enabled/disabled on.
     *  @retval OS_SUCCESS if the initial sending of a message to enable/disable
     *          notifications succeeded.
     *  @retval OS_NOT_FOUND if there is no resource named /p resourceName.
     *  @retval OS_FAILED if some other failure in queueing the message occured.
     */

     /// @brief Set the secure RTP parameters.
   virtual OsStatus setSrtpParams(SdpSrtpParameters& srtpParameters);
     /**<
     *  @param[in] srtpParameters - the parameter block to pull requested
     *             srtp settings from.
     *  @retval     UNKNOWN - << TODO: Add useful return values here - i.e.
     *              failure codes to expect, etc. -- kkyzivat 20070801 >>
     */


     /// @brief Set the connection destination (target) for the designated
     ///        media connection.
   virtual OsStatus setConnectionDestination(int connectionId,
                                             const char* rtpHostAddress, 
                                             int rtpAudioPort,
                                             int rtcpAudioPort,
                                             int rtpVideoPort,
                                             int rtcpVideoPort) = 0 ;
     /**<
     *  @param[in] connectionId - Connection Id for the call leg obtained from 
     *             createConnection
     *  @param[in] rtpHostAddress - IP (or host) address of remote party.
     *  @param[in] rtpAudioPort - RTP audio port of remote party
     *  @param[in] rtcpAudioPort - RTCP audio port of remote party
     *  @param[in] rtpVideoPort - RTP video port of remote party
     *  @param[in] rtcpVideoPort - RTCP video port of remote party
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070801 >>
     */

     /// @brief Add an alternate Audio RTP connection destination for 
     ///        this media interface.
   virtual OsStatus addAudioRtpConnectionDestination(int connectionId,
                                                     int iPriority,
                                                     const char* candidateIp, 
                                                     int candidatePort) = 0 ;
     /**<
     * Alternerates are generally obtained from the SdpBody in the form
     * of candidate addresses.  When adding an alternate connection, the
     * implementation should use an ICE-like method to determine the 
     * best destination address.
     *
     * @param[in] connectionId - Connection Id for the call leg obtained from 
     *            createConnection
     * @param[in] iPriority - Relatively priority of the destination. 
     *            Higher numbers have greater priority.
     * @param[in] candidateIp - Target/Candidate IP Address
     * @param[in] candidatePort - Target/Candidate Port
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070801 >>
     */

     /// @brief Add an alternate Audio RTCP connection destination for 
     ///        this media interface.
   virtual OsStatus addAudioRtcpConnectionDestination(int connectionId,
                                                       int iPriority,
                                                       const char* candidateIp,
                                                       int candidatePort) = 0 ;
     /**<
     *  @see CpMediaInterface::addAudioRtpConnectionDestination
     */

     /// @brief Add an alternate Video RTP connection destination for 
     ///        this media interface.
   virtual OsStatus addVideoRtpConnectionDestination(int connectionId,
                                                     int iPriority,
                                                     const char* candidateIp,
                                                     int candidatePort) = 0 ;
     /**<
     *  @see CpMediaInterface::addAudioRtpConnectionDestination
     */

     /// @brief Add an alternate Video RTCP connection destination for 
     ///        this media interface.
   virtual OsStatus addVideoRtcpConnectionDestination(int connectionId,
                                                      int iPriority,
                                                      const char* candidateIp,
                                                      int candidatePort) = 0 ;
     /**<
     *  @see CpMediaInterface::addAudioRtpConnectionDestination
     */



     /// @brief Start sending RTP using the specified codec list.
   virtual OsStatus startRtpSend(int connectionId, 
                                 const SdpCodecList& codecList) = 0 ;
     /**<
     *  Generally, this codec list is the intersection between both parties.
     *
     *  @param[in] connectionId - Connection Id for the call leg obtained from 
     *             createConnection
     *  @param codecList List of SdpCodec instances
     */ 

     /// @brief Start receiving RTP using the specified codec list.
   virtual OsStatus startRtpReceive(int connectionId,
                                    const SdpCodecList& codecList) = 0;
     /**<
     *  Generally, this codec list is the intersection between both parties.
     *  The media processing subsystem should be prepared to receive any of 
     *  the specified payload type without additional signaling.
     *  For example, it is perfectly legal to switch between codecs on a whim 
     *  if multiple codecs are agreed upon.
     *
     *  @param[in] connectionId - Connection Id for the call leg obtained from 
     *             createConnection
     *  @param codecList List of SdpCodec instances
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070801 >>
     */

     /// @brief Enables RTP read notification.
   virtual OsStatus enableRtpReadNotification(int connectionId,
                                              UtlBoolean bEnable = TRUE);
     /**<
     *  Enables read notification through the ISocketEvent listener passed 
     *  in via createConnection.  This should be enabled immediately after 
     *  calling startRtpReceive.  It is automatically disabled as part of 
     *  stopRtpReceive.
     *
     *  @param[in] connectionId - the ID of the connection to enable RTP read
     *             notification from.
     *  @param[in] bEnable - Whether or not you want to enable or disable 
     *             read notification.
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070801 >>
     */ 

     /// @brief Stop sending RTP and RTCP data for the specified connection
   virtual OsStatus stopRtpSend(int connectionId) = 0 ;
     /**<
     *  @param[in] connectionId - Connection Id for the call leg obtained from 
     *             createConnection
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070801 >>
     */ 

     /// @brief Stop receiving RTP and RTCP data for the specified connection
   virtual OsStatus stopRtpReceive(int connectionId) = 0 ;
     /**<
     *  @param[in] connectionId - Connection Id for the call leg obtained from 
     *             createConnection
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070801 >>
     */

     /// @brief Delete the specified RTP or RTCP connetion.
   virtual OsStatus deleteConnection(int connectionId) = 0 ;
     /**<
     *  Delete the specified connection and free up any resources associated 
     *  with that connection.
     *
     *  @param[in] connectionId - Connection Id for the call leg obtained 
     *             from createConnection.
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070801 >>
     */

     /// @brief Start playing the specified tone for this call.
   virtual OsStatus startTone(int toneId, 
                              UtlBoolean local, 
                              UtlBoolean remote,
                              int duration = 120) = 0 ;
     /**<
     *  If the tone is a DTMF tone and the remote flag is set, the interface 
     *  should send out of band DTMF using RFC 2833.  Inband audio should be 
     *  sent to all connections.  If a previous tone was playing, calling 
     *  startTone should automatically stop existing tone.
     * 
     *  @param[in] toneId - The designated tone to play (TODO: make enum)
     *  @param[in] local - True indicates that sound should be played to 
     *             the local speaker (assuming call is in focus).
     *  @param[in] remote - True indicates that the sound should be played to 
     *             all remote parties.  
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070801 >>
     */

   /// @brief Stop playing all tones.
   virtual OsStatus stopTone() = 0 ;
   /**
    * Some tones/implementations may not support this.
    * For example, some DTMF playing implementations will only play DTMF 
    * for a fixed interval.
    *
    *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
    *             failure codes to expect, etc. -- kkyzivat 20070801 >>
    */
     /// @brief Play the specified audio URL to the call.
   virtual OsStatus playAudio(const char* url, 
                              UtlBoolean repeat,
                              UtlBoolean local, 
                              UtlBoolean remote,
                              UtlBoolean mixWithMic = false,
                              int downScaling = 100,
                              void* pCookie = NULL) = 0 ;
     /**<
     *
     *  @param[in] url - Audio url to be played -- The sipX implementation is limited 
     *             to file paths (not file Urls).
     *  @param[in] repeat - If set, loop the audio file until stopAudio is called.
     *  @param[in] local - True indicates that sound should be played to the local 
     *             speaker (assuming call is in focus).
     *  @param[in] remote - True indicates that the sound should be played to all 
     *             remote parties.  
     *  @param[in] mixWithMic - True to mix with microphone or False to replace it.
     *  @param[in] downScaling - 100 for no down scaling (range from 0 to 100) 
     *
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070801 >>
     */

     /// @brief Play the specified audio buffer to the call. 
   virtual OsStatus playBuffer(void* buf, 
                               size_t bufSize,
                               int type, 
                               UtlBoolean repeat,
                               UtlBoolean local, 
                               UtlBoolean remote,
                               UtlBoolean mixWithMic = false,
                               int downScaling = 100,
                               void* pCookie = NULL) = 0 ;

   /**
    * Pause playback of file or buffer.
    */
   virtual OsStatus pausePlayback() = 0;

   /**
    * Resume playback of file or buffer.
    */
   virtual OsStatus resumePlayback() = 0;

     /**<
     *
     *  @TODO This method should also specify the audio format (e.g. samples/per 
     *        second, etc.).
     *
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070801 >>
     * 
     *  @see CpMediaInterface::playAudio
     */


     /// @brief Stop playing any URLs or buffers
   virtual OsStatus stopAudio()  = 0 ;
     /**<
     *
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070802 >>
     */

   /**
    * Mute input for given call on bridge.
    */
   virtual OsStatus muteInput(int connectionId, UtlBoolean bMute);

     /// @brief Give the focus of the local audio device to the associated call 
   virtual OsStatus giveFocus() = 0 ;
     /**<
     *  (for example, take this call off hold).
     *
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070802 >>
     */

     /// @brief Take this call out of focus for the local audio device 
   virtual OsStatus defocus() = 0 ;
     /**<
     *  (for example, put this call on hold).
     *
     *  @retval    UNKNOWN - << TODO: Add useful return values here - i.e.
     *             failure codes to expect, etc. -- kkyzivat 20070802 >>
     */

   /**
    * Returns TRUE if this media interface is in focus.
    */
   virtual UtlBoolean hasFocus() = 0;

   /**
    * Starts recording all audio channels into given file.
    */
   virtual OsStatus recordAudio(const char* szFile) = 0;

   //! Stop recording for this call.
   virtual OsStatus stopRecording() = 0;

   //! Set the preferred contact type for this media connection
   virtual void setContactType(int connectionId, SIP_CONTACT_TYPE eType, int contactId) = 0 ;

   /** Rebuilds internal SdpCodecList using supplied SdpCodecList */
   virtual OsStatus setCodecList(const SdpCodecList& sdpCodecList) = 0;

   /** Copies internal SdpCodecList into supplied SdpCodecList */
   virtual OsStatus getCodecList(SdpCodecList& sdpCodecList) = 0;

   /** Copies internal SdpCodecList of media connection into supplied SdpCodecList */
   virtual OsStatus getCodecList(int connectionId, SdpCodecList& sdpCodecList) = 0;

   //! Set connection framerate on the fly
   virtual OsStatus setConnectionFramerate(int connectionId, int framerate) = 0;

   virtual OsStatus setVideoWindowDisplay(const void* hWnd) = 0;

   virtual OsStatus setSecurityAttributes(const void* security) = 0;

   virtual void setConnectionTcpRole(const int connectionId,
                                     const RtpTcpRoles role) = 0;

/* ============================ ACCESSORS ================================= */

   /**
    * Get the port, address, and codec capabilities for the specified media 
    * connection.  The CpMediaInterface implementation is responsible for 
    * managing port allocations.
    *  
    * @param connectionId Connection Id for the call leg obtained from 
    *        createConnection
    * @param rtpHostAddress IP address or hostname that should be advertised
    *        in SDP data.
    * @param rtpPort RTP port number that should be advertised in SDP.
    * @param rtcpPort RTCP port number that should be advertised in SDP.
    * @param supportedCodecs List of supported codecs.
    * @param srtParams supported SRTP parameters
    */
   virtual OsStatus getCapabilities(int connectionId, 
                                    UtlString& rtpHostAddress, 
                                    int& rtpAudioPort,
                                    int& rtcpAudioPort,
                                    int& rtpVideoPort,
                                    int& rtcpVideoPort, 
                                    SdpCodecList& supportedCodecs,
                                    SdpSrtpParameters& srtpParams,
                                    int& videoBandwidth,
                                    int& videoFramerate) = 0;
    
   /**
    * DOCME
    */
   virtual OsStatus getCapabilitiesEx(int connectionId, 
                                      int nMaxAddresses,
                                      UtlString rtpHostAddresses[], 
                                      int rtpAudioPorts[],
                                      int rtcpAudioPorts[],
                                      int rtpVideoPorts[],
                                      int rtcpVideoPorts[],
                                      RTP_TRANSPORT transportTypes[],
                                      int& nActualAddresses,
                                      SdpCodecList& supportedCodecs,
                                      SdpSrtpParameters& srtpParameters,
                                      int& videoBandwidth,
                                      int& videoFramerate) = 0 ;


   // Returns the primary codec for the connection
   virtual OsStatus getPrimaryCodec(int connectionId, 
                                    UtlString& audioCodec,
                                    UtlString& videoCodec,
                                    int* audiopPayloadType,
                                    int* videoPayloadType,
                                    bool& isEncrypted) = 0;

   virtual const void* getVideoWindowDisplay() = 0;

   virtual OsStatus getAudioEnergyLevels(int& iInputEnergyLevel,
                                         int& iOutputEnergyLevel)
        { return OS_NOT_SUPPORTED ;} ;

   virtual OsStatus getAudioEnergyLevels(int connectionId,
                                         int& iInputEnergyLevel,
                                         int& iOutputEnergyLevel,
                                         int& nContributors,
                                         unsigned int* pContributorSRCIds,
                                         int* pContributorEngeryLevels) 
        { return OS_NOT_SUPPORTED ;} ;

   virtual OsStatus getAudioRtpSourceIDs(int connectionId,
                                         unsigned int& uiSendingSSRC,
                                         unsigned int& uiReceivingSSRC) 
        { return OS_NOT_SUPPORTED ;} ;

   virtual OsStatus enableAudioTransport(int connectionId, UtlBoolean bEnable)
   {
       return OS_NOT_SUPPORTED; 
   };

   virtual OsStatus enableVideoTransport(int connectionId, UtlBoolean bEnable)
   {
       return OS_NOT_SUPPORTED; 
   };

     ///< Get the specific type of this media interface
   virtual UtlString getType() = 0;

/* ============================ INQUIRY =================================== */

    /// Query if connectionId is valid
   virtual UtlBoolean isConnectionIdValid(int connectionId);

   //! Query whether the specified media connection is enabled for 
   //! sending RTP.
   virtual UtlBoolean isSendingRtpAudio(int connectionId) = 0 ;

   //! Query whether the specified media connection is enabled for 
   //! sending RTP.
   virtual UtlBoolean isSendingRtpVideo(int connectionId) = 0 ;

   //! Query whether the specified media connection is enabled for
   //! sending RTP.
   virtual UtlBoolean isReceivingRtpAudio(int connectionId) = 0 ;

   //! Query whether the specified media connection is enabled for
   //! sending RTP.
   virtual UtlBoolean isReceivingRtpVideo(int connectionId) = 0 ;

   //! Query whether the specified media connection has a destination 
   //! specified for sending RTP.
   virtual UtlBoolean isDestinationSet(int connectionId) = 0 ;

   //! Query whether a new party can be added to this media interfaces
   virtual UtlBoolean canAddParty() = 0 ;

   //! Query whether the connection has started sending or receiving video
   virtual UtlBoolean isVideoInitialized(int connectionId) = 0 ;

   //! Query whether the connection has started sending or receiving audio
   virtual UtlBoolean isAudioInitialized(int connectionId) = 0 ;

   //! Query if the audio device is available.
   virtual UtlBoolean isAudioAvailable() = 0;

   //! Query if we are mixing a video conference
   virtual UtlBoolean isVideoConferencing() = 0 ;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    CpMediaInterfaceFactory *mpFactoryImpl ;
    SdpSrtpParameters mSrtpParams;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   //! Assignment operator disabled
   CpMediaInterface& operator=(const CpMediaInterface& rhs);

   //! Copy constructor disabled
   CpMediaInterface(const CpMediaInterface& rCpMediaInterface);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpMediaInterface_h_
