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

#ifndef _SipXMediaInterfaceImpl_h_
#define _SipXMediaInterfaceImpl_h_

// SYSTEM INCLUDES
//#include <>

// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <os/OsDefs.h>
#include <net/QoS.h>
#include <sdp/SdpCodecList.h>
#include "mi/CpMediaInterface.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class MpCallFlowGraph;
class SdpCodec;
class OsDatagramSocket;
class SipXMediaConnection;
class ISocketEvent;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipXMediaInterfaceImpl : public CpMediaInterface
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum OutputAudioDevice
   {
      UNKNOWN = 0x0,
      HANDSET = 0x1,
      SPEAKER = 0x2,
      HEADSET = 0x4
   };

/* ============================ CREATORS ================================== */

   SipXMediaInterfaceImpl(CpMediaInterfaceFactory* pFactoryImpl,
						       OsMsgQ* pInterfaceNotificationQueue,///< queue for sending interface notifications
                         const SdpCodecList* pCodecList = NULL,///< list of SdpCodec instances
                         const char* publicIPAddress = NULL,///< ignored
                         const char* localIPAddress = NULL,///< local bind IP address
                         const char* pLocale = "",///< locale for tone generator
                         int expeditedIpTos = QOS_LAYER3_LOW_DELAY_IP_TOS,
                         const char* szStunServer = NULL,
                         int iStunPort = PORT_NONE,
                         int iStunKeepAlivePeriodSecs = 28,
                         const char* szTurnServer = NULL,
                         int iTurnPort = PORT_NONE,
                         const char* szTurnUsername = NULL,
                         const char* szTurnPassword = NULL,
                         int iTurnKeepAlivePeriodSecs = 28,
                         UtlBoolean bEnableICE = FALSE);
     //:Default constructor

  protected:
   virtual
   ~SipXMediaInterfaceImpl();
     //:Destructor
  public:

   /**
    * public interface for destroying this media interface
    */ 
   void release();

/* ============================ MANIPULATORS ============================== */


   virtual OsStatus createConnection(int& connectionId, ///< will get assigned connection id
                                     const char* szLocalAddress, ///< optionally override local bind address of media interface
                                     int localPort = 0,
                                     void* videoWindowHandle = NULL,
                                     void* const pSecurityAttributes = NULL,
                                     OsMsgQ* pConnectionNotificationQueue = NULL,
                                     const RtpTransportOptions rtpTransportOptions=RTP_TRANSPORT_UDP);
   
   virtual void setInterfaceNotificationQueue(OsMsgQ* pInterfaceNotificationQueue);

   virtual OsStatus getCapabilities(int connectionId, 
                                    UtlString& rtpHostAddress, 
                                    int& rtpAudioPort,
                                    int& rtcpAudioPort,
                                    int& rtpVideoPort,
                                    int& rtcpVideoPort,
                                    SdpCodecList& supportedCodecs,
                                    SdpSrtpParameters& srtpParams,
                                    int& videoBandwidth,
                                    int& videoFramerate) ;

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
                                      int& videoFramerate);

   virtual OsStatus setMediaNotificationsEnabled(bool enabled, 
                                                 const UtlString& resourceName);

   virtual OsStatus setConnectionDestination(int connectionId,
                                             const char* rtpHostAddress, 
                                             int rtpAudioPort,
                                             int rtcpAudioPort,
                                             int rtpVideoPort,
                                             int rtcpVideoPort);

   virtual OsStatus startRtpSend(int connectionId, 
                                 const SdpCodecList& codecList);

   virtual OsStatus startRtpReceive(int connectionId,
                                    const SdpCodecList& codecList);

   virtual OsStatus stopRtpSend(int connectionId);
   virtual OsStatus stopRtpReceive(int connectionId);

   virtual OsStatus deleteConnection(int connectionId);

   virtual OsStatus startTone(int toneId, UtlBoolean local, UtlBoolean remote, int duration = 120);
   virtual OsStatus stopTone();

   virtual OsStatus muteInput(int connectionId, UtlBoolean bMute);

   virtual OsStatus playAudio(const char* url, 
                              UtlBoolean repeat,
                              UtlBoolean local, 
                              UtlBoolean remote,
                              UtlBoolean mixWithMic = false,
                              int downScaling = 100,
                              void* pCookie = NULL);


    virtual OsStatus playBuffer(void* buf, 
                               size_t bufSize,
                               int type, 
                              UtlBoolean repeat,
                              UtlBoolean local, 
                              UtlBoolean remote,
                              UtlBoolean mixWithMic = false,
                              int downScaling = 100,
                              void* pCookie = NULL);

    virtual OsStatus stopAudio();

    virtual OsStatus pausePlayback();

    virtual OsStatus resumePlayback();

   virtual OsStatus giveFocus();
   virtual OsStatus defocus();
   virtual UtlBoolean hasFocus();

   virtual OsStatus stopRecording();

   /**
   * Starts recording all audio channels into given file.
   */
   virtual OsStatus recordAudio(const char* szFile);

    virtual void setContactType(int connectionId, SIP_CONTACT_TYPE eType, int contactId) ;
     //: Set the contact type for this Phone media interface.  
     //  It is important to set the contact type BEFORE creating the 
     //  connection -- setting after the connection has been created
     //  is essentially a NOP.

    /**
     * Rebuilds internal SdpCodecList using supplied SdpCodecList
     */
    virtual OsStatus setCodecList(const SdpCodecList& sdpCodecList);

    /** Copies internal SdpCodecList into supplied SdpCodecList */
    virtual OsStatus getCodecList(SdpCodecList& sdpCodecList);

    /** Copies internal SdpCodecList of media connection into supplied SdpCodecList */
    virtual OsStatus getCodecList(int connectionId, SdpCodecList& sdpCodecList);

    //! Set connection framerate on the fly
    virtual OsStatus setConnectionFramerate(int connectionId, int framerate) ;

    virtual OsStatus setSecurityAttributes(const void* security) ;

    virtual OsStatus addAudioRtpConnectionDestination(int         connectionId,
                                                      int         iPriority,
                                                      const char* candidateIp, 
                                                      int         candidatePort) ;

    virtual OsStatus addAudioRtcpConnectionDestination(int         connectionId,
                                                       int         iPriority,
                                                       const char* candidateIp, 
                                                       int         candidatePort) ;

    virtual OsStatus addVideoRtpConnectionDestination(int         connectionId,
                                                      int         iPriority,
                                                      const char* candidateIp, 
                                                      int         candidatePort) ;

    virtual OsStatus addVideoRtcpConnectionDestination(int         connectionId,
                                                       int         iPriority,
                                                       const char* candidateIp, 
                                                       int         candidatePort) ;
    
    virtual void setConnectionTcpRole(const int connectionId, const RtpTcpRoles role)
    {
        // NOT IMPLEMENTED
    }

/* ============================ ACCESSORS ================================= */

   virtual OsStatus getVideoQuality(int& quality);
   virtual OsStatus getVideoBitRate(int& bitRate);
   virtual OsStatus getVideoFrameRate(int& frameRate);

     //:Returns primary codec for the connection
   virtual OsStatus setVideoWindowDisplay(const void* hWnd);
   virtual const void* getVideoWindowDisplay();

   virtual OsStatus setVideoQuality(int quality);
   virtual OsStatus setVideoParameters(int bitRate, int frameRate);

   // Returns the primary codec for the connection
   virtual OsStatus getPrimaryCodec(int connectionId, 
                                    UtlString& audioCodec,
                                    UtlString& videoCodec,
                                    int* audiopPayloadType,
                                    int* videoPayloadType,
                                    bool& isEncrypted) ;

   virtual UtlString getType() { return "SipXMediaInterfaceImpl"; };

/* ============================ INQUIRY =================================== */

   virtual UtlBoolean isSendingRtpAudio(int connectionId);
   virtual UtlBoolean isReceivingRtpAudio(int connectionId);
   virtual UtlBoolean isSendingRtpVideo(int connectionId);
   virtual UtlBoolean isReceivingRtpVideo(int connectionId);
   virtual UtlBoolean isDestinationSet(int connectionId);   
   virtual UtlBoolean canAddParty() ;
   virtual UtlBoolean isVideoInitialized(int connectionId) ;
   virtual UtlBoolean isAudioInitialized(int connectionId) ;
   virtual UtlBoolean isAudioAvailable() ;
   virtual UtlBoolean isVideoConferencing() { return false; } ;


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    UtlBoolean getLocalAddresses(int connectionId,
                                 int nMaxAddresses,
                                 UtlString hostIps[], ///< host ip addresses to which socket is bound (in case 0.0.0.0 we get real ips)
                                 int& rtpAudioPort,
                                 int& rtcpAudioPort,
                                 int& rtpVideoPort,
                                 int& rtcpVideoPort,
                                 int& nActualAddresses);

    UtlBoolean getNatedAddresses( int connectionId,
                                  UtlString& hostIp,
                                  int& rtpAudioPort,
                                  int& rtcpAudioPort,
                                  int& rtpVideoPort,
                                  int& rtcpVideoPort) ;


    UtlBoolean getRelayAddresses( int connectionId,
                                  UtlString& hostIp,
                                  int& rtpAudioPort,
                                  int& rtcpAudioPort,
                                  int& rtpVideoPort,
                                  int& rtcpVideoPort) ;


    OsStatus addLocalContacts(  int connectionId, 
                                int nMaxAddresses,
                                UtlString rtpHostAddresses[], 
                                int rtpAudioPorts[],
                                int rtcpAudioPorts[],
                                int rtpVideoPorts[],
                                int rtcpVideoPorts[],
                                int& nActualAddresses) ;

    OsStatus addNatedContacts(  int connectionId, 
                                int nMaxAddresses,
                                UtlString rtpHostAddresses[], 
                                int rtpAudioPorts[],
                                int rtcpAudioPorts[],
                                int rtpVideoPorts[],
                                int rtcpVideoPorts[],
                                int& nActualAddresses) ;

    OsStatus addRelayContacts(  int connectionId, 
                                int nMaxAddresses,
                                UtlString rtpHostAddresses[], 
                                int rtpAudioPorts[],
                                int rtcpAudioPorts[],
                                int rtpVideoPorts[],
                                int rtcpVideoPorts[],
                                int& nActualAddresses) ;

    void applyAlternateDestinations(int connectionId) ;

      /// Create socket pair for RTP/RTCP streams.
    OsStatus createRtpSocketPair(UtlString localAddress,
                                 int localPort,
                                 SIP_CONTACT_TYPE contactType,
                                 OsDatagramSocket* &rtpSocket,
                                 OsDatagramSocket* &rtcpSocket);
      /**<
      *  For RTP/RTCP port pair will be set next free port pair.
      *  
      *  @param[in] localIPAddress - Address to bind to (for multihomed hosts).
      *  @param[in] localPort - Local port to bind to (0 for auto select).
      *  @param[in] contactType - Contact type (see SIPX_CONTACT_TYPE).
      *  @param[out] rtpSocket - Created socket for RTP stream.
      *  @param[out] rtcpSocket - Created socket for RTCP stream.
      */


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    SipXMediaConnection* getMediaConnection(int connectionId);
    SipXMediaConnection* removeMediaConnection(int connectionId);
    OsStatus doDeleteConnection(SipXMediaConnection* mediaConnection);

   UtlString mRtpReceiveHostAddress; ///< Advertised as place to send RTP/RTCP
   UtlString mLocalIPAddress;          ///< Address on which ports are bound
   MpCallFlowGraph* mpFlowGraph;     ///< Flowgraph for audio part of call
   UtlBoolean mRingToneFromFile;
   SdpCodecList mSdpCodecList;
   UtlDList mMediaConnections;
   int mExpeditedIpTos;
   UtlString mStunServer ;
   int mStunPort;
   int mStunRefreshPeriodSecs ;
   UtlString mTurnServer ;
   int mTurnPort ;
   int mTurnRefreshPeriodSecs ;
   UtlString mTurnUsername ;
   UtlString mTurnPassword ;
   UtlBoolean mEnableIce ;
   OsMsgQ* m_pInterfaceNotificationQueue;

   // Disabled copy constructor
   SipXMediaInterfaceImpl(const SipXMediaInterfaceImpl& rCpPhoneMediaInterface);

   // Disabled equals operator
   SipXMediaInterfaceImpl& operator=(const SipXMediaInterfaceImpl& rhs);
     

};

/* ============================ INLINE METHODS ============================ */
#endif  // _SipXMediaInterfaceImpl_h_
