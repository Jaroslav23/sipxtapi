//
// Copyright (C) 2006-2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpCallFlowGraph_h_
#define _MpCallFlowGraph_h_

#include "rtcp/RtcpConfig.h"

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <mp/MpDefs.h>
#include "mp/MpMisc.h"
#include "mp/MpFlowGraphBase.h"
#include "os/OsProtectEvent.h"
#include "mp/MprRecorder.h"
#ifdef INCLUDE_RTCP /* [ */
#include "rtcp/RTCManager.h"
#endif /* INCLUDE_RTCP ] */

// DEFINES
// Enable Speex AEC if Speex is available
#ifdef HAVE_SPEEX // [
#define SPEEX_ECHO_CANCELATION
#define DOING_ECHO_CANCELATION
#endif // !HAVE_SPEEX ]

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef enum FLOWGRAPH_AEC_MODE
{
    FLOWGRAPH_AEC_DISABLED,
    FLOWGRAPH_AEC_SUPPRESS,
    FLOWGRAPH_AEC_CANCEL,
    FLOWGRAPH_AEC_CANCEL_AUTO
} FLOWGRAPH_AEC_MODE;

// FORWARD DECLARATIONS
class MprBridge;
class MprFromFile;
class MprFromMic;
class MprSpeexEchoCancel;
class MprSpeexPreprocess;
class MprMixer;
class MprSplitter;
class MprToSpkr;
class MprToneGen;
class SdpCodec;
class SdpCodecList;
class MpRtpInputAudioConnection;
class MpRtpOutputAudioConnection;
class MpStopDTMFTimerMsg;
class MpTimerMsg;
class OsTimer;

/// Flow graph used to handle a basic call
#ifdef INCLUDE_RTCP /* [ */
class MpCallFlowGraph : public MpFlowGraphBase,
                        public CBaseClass,
                        public IRTCPNotify
#else /* INCLUDE_RTCP ] [ */
class MpCallFlowGraph : public MpFlowGraphBase
#endif /* INCLUDE_RTCP ] */
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   typedef enum
   {
      TONE_TO_SPKR = 0x1,   ///< Play locally
      TONE_TO_NET  = 0x2    ///< Mix the tone to play out the network connection
   } ToneOptions;

   enum RecorderChoice {
      RECORDER_CALL = 0,    ///< full conversation recorder
      MAX_RECORDERS = 10
   };

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Default constructor
   MpCallFlowGraph(const char* pLocale = "",
				       OsMsgQ* pInterfaceNotificationQueue = NULL);

     /// Destructor
   virtual
   ~MpCallFlowGraph();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

   /// Starts playing the indicated tone.
   void startTone(int toneId, int toneOptions, int duration = -1);

   /**
   * Stops playing the tone (applies to all tone destinations).
   */ 
   void stopTone();

   int closeRecorders(void);

   OsStatus record(int timeMS,
                   int silenceLength,
                   const char* callRecordFile = NULL,
                   int toneOptions = 0,
                   int repeat = 0,
                   OsNotification* completion = NULL,
                   MprRecorder::RecordFileFormat format = MprRecorder::RAW_PCM_16);

   OsStatus startRecording();

   UtlBoolean setupRecorder(RecorderChoice which, const char* audioFileName,
                  int timeMS, int silenceLength, OsNotification* event = NULL,
                  MprRecorder::RecordFileFormat format = MprRecorder::RAW_PCM_16);

   OsStatus playBuffer(void* audioBuf,
                  size_t bufSize,
                  int type,
                  UtlBoolean repeat,
                  int toneOptions,
                  void* pCookie = NULL);

     /// Start playing audio from a file
   OsStatus playFile( const char* audioFileName ///< name of the audio file
                    , UtlBoolean repeat ///< TRUE/FALSE continue playing audio
                                        ///< from the beginning after the end of
                                        ///< file is reached.
                    , int toneOptions   ///< TONE_TO_SPKR/TONE_TO_NET file audio
                                        ///< played locally or both locally and
                                        ///< remotely.
                    , void* pCookie = NULL);
     /**<
     *  @returns <b>OS_INVALID_ARGUMENT</b> - if open on the given file name
     *                                        failed.
     */

     /// Stop playing audio from a file
   void stopFile();
     /**<
     *  @param closeFile - TRUE/FALSE whether to close the audio file.
     */

   /**
    * Pause playback of buffer or file.
    */
   OsStatus pausePlayback();

   /**
    * Resume playback of buffer or file.
    */
   OsStatus resumePlayback();


     /// Starts sending RTP and RTCP packets.
   void startSendRtp(OsSocket& rRtpSocket,
                     OsSocket& rRtcpSocket,
                     MpConnectionID connID = 1,
                     SdpCodec* pPrimaryCodec = NULL,
                     SdpCodec* pDtmfCodec = NULL);

     /// Stops sending RTP and RTCP packets.
   void stopSendRtp(MpConnectionID connID = 1);

     /// Starts receiving RTP and RTCP packets.
   void startReceiveRtp(const SdpCodecList& sdpCodecList,
                        OsSocket& rRtpSocket,
                        OsSocket& rRtcpSocket,
                        MpConnectionID connID = 1);

     /// Stops receiving RTP and RTCP packets.
   void stopReceiveRtp(MpConnectionID connID = 1);

     /// Informs the flow graph that it now has the MpMediaTask focus.
   virtual OsStatus gainFocus(void);
     /**<
     *  Only the flow graph that has the focus is permitted to access
     *  the audio hardware.  This may only be called if this flow graph
     *  is managed and started!
     *
     *  @returns  OS_SUCCESS, always
     */

     /// Informs the flow graph that it has lost the MpMediaTask focus.
   virtual OsStatus loseFocus(void);
     /**<
     *  Only the flow graph that has the focus is permitted to access
     *  the audio hardware.  This should only be called if this flow graph
     *  is managed and started!
     *
     *  @returns  OS_SUCCESS, always
     */

     /// Creates a new MpAudioConnection; returns -1 if failure.
   MpConnectionID createConnection(OsMsgQ* pConnectionNotificationQueue);

     /// Removes an MpAudioConnection and deletes it and all its resources.
   OsStatus deleteConnection(MpConnectionID connID);

     /// enables hearing audio data from a source
   OsStatus unmuteInput(MpConnectionID connID);

     /// disables hearing audio data from a source
   OsStatus muteInput(MpConnectionID connID);

   virtual void setInterfaceNotificationQueue(OsMsgQ* pInterfaceNotificationQueue);

//@}

   /** Sets timeout in seconds for media connections. After that timeout, notifications will be sent. */
   static OsStatus setConnectionIdleTimeout(const int idleTimeout);

     /// Enables/Disable the transmission of inband DTMF audio. Othersise RFC 2833 will be used.
   static UtlBoolean enableSendInbandDTMF(UtlBoolean bEnable);

   /// Gets status if sending inbound DTMF is enabled
   static UtlBoolean isSendInbandDTMFEnabled();

   /**
    * Gets AEC settings.
    */
   static UtlBoolean getAECMode(FLOWGRAPH_AEC_MODE& mode);

   /// Set Acoustic Echo Cancelation mode
   static UtlBoolean setAECMode(FLOWGRAPH_AEC_MODE mode);
     /**<
     *  @warning Only available when Speex or internal AEC module is enabled!
     */

   /// Gets AGC settings
   static UtlBoolean getAGC(UtlBoolean& bEnabled);

   /// Enable/disable Automatic Gain Control.
   static UtlBoolean setAGC(UtlBoolean bEnable);
     /**<
     *  @warning Only available when Speex is enabled!
     */

   /// Gets noise reduction configuration
   static UtlBoolean getAudioNoiseReduction(UtlBoolean& bEnabled);

   /// Enable/disable speex noise reduction.
   static UtlBoolean setAudioNoiseReduction(UtlBoolean bEnable);
     /**<
     *  @note Enabling this also enables echo residue filtering.
     *
     *  @warning Only available when Speex is enabled!
     */

   /// Gets speex voice activity detection setting
   static UtlBoolean isVADEnabled();

   /// Enable/disable speex voice activity detection
   static void enableVAD(UtlBoolean bEnable);

   static UtlBoolean enableInboundInBandDTMF(UtlBoolean enable);
   static UtlBoolean enableInboundRFC2833DTMF(UtlBoolean enable);

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

#ifdef INCLUDE_RTCP /* [ */
     /// Returns the RTCP Session interface pointer associated with this call's flow graph.
   IRTCPSession* getRTCPSessionPtr(void);
#endif /* INCLUDE_RTCP ] */

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

   static UtlBoolean isInboundInBandDTMFEnabled();
   static UtlBoolean isInboundRFC2833DTMFEnabled();

/* ============================ CALLBACKS ================================= */
#ifdef INCLUDE_RTCP /* [ */

/**
 *
 * Method Name:  GetEventInterest()
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long - Mask of Event Interests
 *
 * Description: The GetEventInterest() event method shall allow the dispatcher
 *              of notifications to access the event interests of a subscriber
 *              and use these wishes to dispatch RTCP event notifications
 *
 * Usage Notes:
 *
 */
    unsigned long GetEventInterest(void);

/**
 *
 * Method Name:  LocalSSRCCollision()
 *
 *
 * Inputs:      IRTCPConnection *piRTCPConnection - Interface to
 *                                                   associated RTCP Connection
 *              IRTCPSession    *piRTCPSession    - Interface to associated
 *                                                   RTCP Session
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: The LocalSSRCCollision() event method shall inform the
 *              recipient of a collision between the local SSRC and one
 *              used by one of the remote participants.
 *
 * Usage Notes:
 *
 */
    void LocalSSRCCollision(IRTCPConnection    *piRTCPConnection,
                            IRTCPSession       *piRTCPSession);


/**
 *
 * Method Name:  RemoteSSRCCollision()
 *
 *
 * Inputs:      IRTCPConnection *piRTCPConnection - Interface to associated
 *                                                    RTCP Connection
 *              IRTCPSession    *piRTCPSession    - Interface to associated
 *                                                    RTCP Session
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: The RemoteSSRCCollision() event method shall inform the
 *              recipient of a collision between two remote participants.
 *              .
 *
 * Usage Notes:
 *
 */
    void RemoteSSRCCollision(IRTCPConnection    *piRTCPConnection,
                             IRTCPSession       *piRTCPSession);


/**
 *
 * Macro Name:  DECLARE_IBASE_M
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: This implements the IBaseClass functions used and exposed by
 *              derived classes.
 *
 * Usage Notes:
 *
 *
 */
DECLARE_IBASE_M

#endif /* INCLUDE_RTCP ] */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   static UtlBoolean sbSendInBandDTMF;
   UtlBoolean m_bPlayingInBandDTMF;
   UtlBoolean m_bPlayingRfc2833DTMF;
   static FLOWGRAPH_AEC_MODE ms_AECMode;
   static UtlBoolean sbEnableAEC;
   static UtlBoolean sbEnableAGC;
   static UtlBoolean sbEnableNoiseReduction;
   static UtlBoolean ms_bEnableInboundInBandDTMF;
   static UtlBoolean ms_bEnableInboundRFC2833DTMF;

   OsMsgQ* m_pInterfaceNotificationQueue;

   MprBridge*    mpBridge;
   MprFromFile*  mpFromFile;
#ifndef DISABLE_LOCAL_AUDIO // [
   MprFromMic*   mpFromMic;
#ifdef HAVE_SPEEX // [
   MprSpeexPreprocess* mpSpeexPreProcess;
#if defined (SPEEX_ECHO_CANCELATION)
   MprSpeexEchoCancel* mpEchoCancel;
#endif // SPEEX_ECHO_CANCELATION ]
#endif // HAVE_SPEEX ]
   MprToSpkr*    mpToSpkr;
#endif // DISABLE_LOCAL_AUDIO ]
   MprMixer*     mpTFsMicMixer;
   MprMixer*     mpTFsBridgeMixer;
   MprMixer*     mpCallrecMixer;
   MprSplitter*  mpMicCallrecSplitter;
   MprSplitter*  mpSpeakerCallrecSplitter;
   MprSplitter*  mpToneFileSplitter;
   MprToneGen*   mpToneGen;
   OsBSem        mConnTableLock;
   MpRtpInputAudioConnection* mpInputConnections[MAX_CONNECTIONS];
   MpRtpOutputAudioConnection* mpOutputConnections[MAX_CONNECTIONS];
   int mpBridgePorts[MAX_CONNECTIONS];
   UtlBoolean     mToneGenDefocused; ///< disabled during defocused state flag
#ifdef INCLUDE_RTCP /* [ */
   IRTCPSession* mpiRTCPSession;
   /// Event Interest Attribute for RTCP Notifications
   unsigned long mulEventInterest;
#endif /* INCLUDE_RTCP ] */
   OsTimer* m_pStopDTMFToneTimer; ///< timer which is started when DTMF tone starts playing

   /// these array should really be made into a structure
   /// but for now we'll just use em this way.
   ///
   ///  D.W.
   MprRecorder* mpRecorders[MAX_RECORDERS];

   /**
   * Sends interface notification to interface notification queue if it was supplied
   */
   virtual void sendInterfaceNotification(MpNotificationMsgMedia msgMedia,
                                          MpNotificationMsgType msgSubType,
                                          intptr_t msgData1 = 0,
                                          intptr_t msgData2 = 0);

   /**
   * Estimates optimum length of echo queue. Input and output driver
   * have some latency, and we have to delay processing echo frames
   * by exactly that delay. This method will estimate this latency and
   * is crucial for correct echo canceler operation
   */
   static int estimateEchoQueueLatency(int samplesPerSec,int samplesPerFrame);

     ///  Write out standard 16bit 8k sampled WAV Header
   UtlBoolean writeWAVHeader(int handle);

     /// Handles an incoming message for the flow graph.
   virtual UtlBoolean handleMessage(OsMsg& rMsg);
     /**<
     *  @returns <b>TRUE</b> if the message was handled
     *  @returns <b>FALSE</b> otherwise.
     */

   /** Handles timer messages. */
   UtlBoolean handleTimerMessage(const MpTimerMsg& rMsg);

   /** Handles stop DTMF timer message. */
   UtlBoolean handleStopDTMFTimerMessage(const MpStopDTMFTimerMsg& rMsg);

     /// Handle the FLOWGRAPH_REMOVE_CONNECTION message.
   UtlBoolean handleRemoveConnection(MpFlowGraphMsg& rMsg);
     /**<
     *  @returns <b>TRUE</b> if the message was handled
     *  @returns <b>FALSE</b> otherwise.
     */

     /// Handle the FLOWGRAPH_START_PLAY message for MprFromFile.
   UtlBoolean handleStartPlay(MpFlowGraphMsg& rMsg);
     /**<
     *  @returns <b>TRUE</b> if the message was handled
     *  @returns <b>FALSE</b> otherwise.
     */

     /// Handle the FLOWGRAPH_START_RECORD message.
   UtlBoolean handleStartRecord(MpFlowGraphMsg& rMsg);
     /**<
     *  @returns <b>TRUE</b> if the message was handled
     *  @returns <b>FALSE</b> otherwise.
     */

     /// Handle the FLOWGRAPH_STOP_RECORD message.
   UtlBoolean handleStopRecord(MpFlowGraphMsg& rMsg);
     /**<
     *  @returns <b>TRUE</b> if the message was handled
     *  @returns <b>FALSE</b> otherwise.
     */

     /// Handle the FLOWGRAPH_STOP_PLAY messages.
   UtlBoolean handleStopToneOrPlay(void);
     /**<
     *  @returns <b>TRUE</b> if the message was handled
     *  @returns <b>FALSE</b> otherwise.
     */

   /** Handle the ON_MPRRECORDER_ENABLED message. It is sent when
   *   a recorder is really enabled.
   */
   UtlBoolean handleOnMprRecorderEnabled(MpFlowGraphMsg& rMsg);
   /**<
   *  @returns <b>TRUE</b> if the message was handled
   *  @returns <b>FALSE</b> otherwise.
   */

   /** Handle the ON_MPRRECORDER_DISABLED message. It is sent when
   *   a recorder is really disabled, so we can send an event to
   *   sipxtapi in the future. Currently we only disable some other
   *   resources if the recorder is call recorder.
   */
   UtlBoolean handleOnMprRecorderDisabled(MpFlowGraphMsg& rMsg);
   /**<
   *  @returns <b>TRUE</b> if the message was handled
   *  @returns <b>FALSE</b> otherwise.
   */

     /// Copy constructor (not implemented for this class)
   MpCallFlowGraph(const MpCallFlowGraph& rMpCallFlowGraph);

     /// Assignment operator (not implemented for this class)
   MpCallFlowGraph& operator=(const MpCallFlowGraph& rhs);

};

/* ============================ INLINE METHODS ============================ */
#ifdef INCLUDE_RTCP /* [ */
inline IRTCPSession *MpCallFlowGraph::getRTCPSessionPtr(void)
{
    return(mpiRTCPSession);
}



/**
 *
 * Method Name:  GetEventInterest()
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long - Mask of Event Interests
 *
 * Description: The GetEventInterest() event method shall allow the dispatcher
 *              of notifications to access the event interests of a subscriber
 *              and use these wishes to dispatch RTCP event notifications
 *
 * Usage Notes:
 *
 */
inline unsigned long MpCallFlowGraph::GetEventInterest(void)
{

    return(mulEventInterest);
}
#endif /* INCLUDE_RTCP ] */

#endif  // _MpCallFlowGraph_h_
