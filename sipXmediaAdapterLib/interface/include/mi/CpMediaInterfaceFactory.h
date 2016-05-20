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
///////////////////////////////////////////////////////////////////////////////

#ifndef _CpMediaInterfaceFactory_h_
#define _CpMediaInterfaceFactory_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsMsgQ.h"
#include "os/OsStatus.h"
#include "utl/UtlDefs.h"
#include "utl/UtlString.h"
#include "sdp/SdpCodecList.h"
#include "utl/UtlSList.h"
#include "os/OsMutex.h"


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

/**
* Accoustic echo calcellation modes implementing factory might support
*/
typedef enum MEDIA_AEC_MODE
{
   MEDIA_AEC_DISABLED, 
   MEDIA_AEC_SUPPRESS,
   MEDIA_AEC_CANCEL,
   MEDIA_AEC_CANCEL_AUTO
} MEDIA_AEC_MODE;

/**
* Noise reduction modes implementing factory might support
*/
typedef enum MEDIA_NOISE_REDUCTION_MODE
{
   MEDIA_NOISE_REDUCTION_DISABLED,
   MEDIA_NOISE_REDUCTION_LOW,
   MEDIA_NOISE_REDUCTION_MEDIUM,
   MEDIA_NOISE_REDUCTION_HIGH
} MEDIA_NOISE_REDUCTION_MODE;

/**
* DTMF modes implementing factory might support. In-band is mixed with audio and compressed
* using selected codec, RFC2833 is sent in special payload
*/
typedef enum MEDIA_OUTBOUND_DTMF_MODE
{
   MEDIA_OUTBOUND_DTMF_DISABLED,
   MEDIA_OUTBOUND_DTMF_INBAND,
   MEDIA_OUTBOUND_DTMF_RFC2833
} MEDIA_OUTBOUND_DTMF_MODE;

typedef enum MEDIA_INBOUND_DTMF_MODE
{
   MEDIA_INBOUND_DTMF_INBAND,
   MEDIA_INBOUND_DTMF_RFC2833
} MEDIA_INBOUND_DTMF_MODE;

// keep in sync with MP_VOLUME_METER_TYPE
typedef enum
{
   MEDIA_VOLUME_METER_VU = 0,
   MEDIA_VOLUME_METER_PPM
} MEDIA_VOLUME_METER_TYPE;

// FORWARD DECLARATIONS
class CpMediaInterface;
class SdpCodec;
class CpAudioDeviceInfo;

/**
* Interface for factory creating CpMediaInterface and setting audio/video parameters
*/
class CpMediaInterfaceFactory
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /* ============================ CREATORS ================================== */

   /**
   * Default constructor
   */
   CpMediaInterfaceFactory();    

   /* =========================== DESTRUCTORS ================================ */

   /**
   * Destructor
   */
   virtual ~CpMediaInterfaceFactory();
public:

   /**
   * public interface for destroying this media interface
   */ 
   virtual void release();

   /* ============================ MANIPULATORS ============================== */

   /**
   * Create a media interface given the designated parameters.
   */
   virtual CpMediaInterface* createMediaInterface(OsMsgQ* pInterfaceNotificationQueue,///< queue for sending interface notifications
                                                  const SdpCodecList* pCodecList,///< list of SdpCodec instances
                                                  const char* publicIPAddress,///< ignored
                                                  const char* localIPAddress,///< local bind IP address
                                                  const char* locale,///< locale for tone generator
                                                  int expeditedIpTos,
                                                  const char* szStunServer,
                                                  int iStunPort,
                                                  int iStunKeepAliveSecs,
                                                  const char* szTurnServer,
                                                  int iTurnPort,
                                                  const char* szTurnUsername,
                                                  const char* szTurnPassword,
                                                  int iTurnKeepAliveSecs,
                                                  UtlBoolean bEnableICE
                                                  ) = 0;

   /**
    * Gets number of input audio devices
    */
   virtual OsStatus getAudioInputDeviceCount(int& count) const
   {
      return OS_NOT_SUPPORTED;
   }

   /**
    * Gets number of output audio devices
    */
   virtual OsStatus getAudioOutputDeviceCount(int& count) const
   {
      return OS_NOT_SUPPORTED;
   }

   /**
    * Returns information about given input audio device.
    */
   virtual OsStatus getAudioInputDeviceInfo(int deviceIndex, CpAudioDeviceInfo& deviceInfo) const
   {
      return OS_NOT_SUPPORTED;
   }

   /**
    * Returns information about given output audio device.
    */
   virtual OsStatus getAudioOutputDeviceInfo(int deviceIndex, CpAudioDeviceInfo& deviceInfo) const
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Set the Microphone device
   */
   virtual OsStatus setAudioInputDevice(const UtlString& device, const UtlString& driverName = "")
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Set the speaker device.
   */
   virtual OsStatus setAudioOutputDevice(const UtlString& device, const UtlString& driverName = "")
   {
      return OS_NOT_SUPPORTED;
   }

   /**
    * Sets audio driver latency.
    */
   virtual OsStatus setAudioDriverLatency(double inputLatency, double outputLatency)
   {
      return OS_NOT_SUPPORTED;
   }

   /**
    * Gets audio driver latency.
    */
   virtual OsStatus getAudioDriverLatency(double& inputLatency, double& outputLatency)
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Mute the speaker
   */
   virtual OsStatus muteAudioOutput(UtlBoolean bMute)
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Mute the microphone
   */
   virtual OsStatus muteAudioInput(UtlBoolean bMute)
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Enable/Disable echo cancellation
   */
   virtual OsStatus setAudioAECMode(const MEDIA_AEC_MODE mode)
   {
      return OS_NOT_SUPPORTED;
   }


   /**
   * Enable/Disable Noise Reduction
   */
   virtual OsStatus setAudioNoiseReductionMode(const MEDIA_NOISE_REDUCTION_MODE mode)
   {
      return OS_NOT_SUPPORTED;
   }

   /**
    * Enable/disable voice activity detection
    */
   virtual OsStatus setVADMode(UtlBoolean bEnable)
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Enable AGC Status
   */
   virtual OsStatus enableAGC(UtlBoolean bEnable)
   {
      return OS_NOT_SUPPORTED;
   }


   /**
   * Set mode how to send outbound DTMF
   */
   virtual OsStatus setOutboundDTMFMode(MEDIA_OUTBOUND_DTMF_MODE mode)
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Enable/Disable reception of DTMF
   */
   virtual OsStatus enableInboundDTMF(MEDIA_INBOUND_DTMF_MODE mode, UtlBoolean enable) 
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Enable/Disable RTCP reports
   */
   virtual OsStatus enableRTCP(UtlBoolean bEnable)
   {
      return OS_NOT_SUPPORTED;
   }

   /** 
   * Set name send as part of RTCP reports.
   */
   virtual OsStatus setRTCPName(const char* szName)
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Populate the codec list using supplied string with audio and video codec names.
   */
   virtual OsStatus buildCodecList(SdpCodecList& codecList, 
                                   const UtlString& sAudioPreferences,
                                   const UtlString& sVideoPreferences) = 0;

   /**
   * Populate the codec list with all available codecs.
   */
   virtual OsStatus buildAllCodecList(SdpCodecList& codecList) = 0;

   /**
    * Gets string with all supported audio codecs. Can be used to build codec list
    * with all supported codecs.
    */
   virtual UtlString getAllSupportedAudioCodecs() const = 0;

   /**
   * Gets string with all supported video codecs. Can be used to build codec list
   * with all supported codecs.
   */
   virtual UtlString getAllSupportedVideoCodecs() const = 0;

   /**
   * Sets the RTP port range for this factory
   */     
   virtual void setRtpPortRange(int startRtpPort, int lastRtpPort);

   /** 
   * Gets the next available rtp port
   */
   virtual OsStatus getNextRtpPort(int &rtpPort);

   /**
   * Release the rtp port back to the pool of available RTP ports
   */
   virtual OsStatus releaseRtpPort(const int rtpPort);

   /**
   * Set the connection idle timeout
   */
   virtual OsStatus setConnectionIdleTimeout(const int idleTimeout) 
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Set the global video preview window 
   */ 
   virtual OsStatus setVideoPreviewDisplay(void* pDisplay) 
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Set the global video quality 
   */ 
   virtual OsStatus setVideoQuality(int quality) 
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Set the global video parameters 
   */ 
   virtual OsStatus setVideoParameters(int bitRate, int frameRate) 
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Sets video bitrate
   */
   virtual OsStatus setVideoBitrate(int bitrate)
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Set the global CPU usage
   */
   virtual OsStatus setVideoCpuValue(int cpuValue) 
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Update the video preview window given the specified display context.
   */ 
   virtual OsStatus updateVideoPreviewWindow(void* displayContext) 
   {
      return OS_NOT_SUPPORTED;
   }

   /** 
   * Sets the video capture device, given its string name.
   */
   virtual OsStatus setVideoCaptureDevice(const UtlString& videoDevice) 
   {
      return OS_NOT_SUPPORTED;
   }

   /* ============================ ACCESSORS ================================= */

   /**
   * Get the speaker device
   */ 
   virtual OsStatus getCurrentAudioOutputDevice(CpAudioDeviceInfo& deviceInfo) const
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Get the microphone device
   */
   virtual OsStatus getCurrentAudioInputDevice(CpAudioDeviceInfo& deviceInfo) const
   {
      return OS_NOT_SUPPORTED;
   }

   virtual OsStatus getAudioInputMixerName(UtlString& name) const
   {
      return OS_NOT_SUPPORTED;
   }

   virtual OsStatus getAudioOutputMixerName(UtlString& name) const
   {
      return OS_NOT_SUPPORTED;
   }

   virtual OsStatus getAudioMasterVolume(int& volume) const
   {
      return OS_NOT_SUPPORTED;
   }

   virtual OsStatus setAudioMasterVolume(int volume)
   {
      return OS_NOT_SUPPORTED;
   }

   virtual OsStatus getAudioPCMOutputVolume(int& volume) const
   {
      return OS_NOT_SUPPORTED;
   }

   virtual OsStatus setAudioPCMOutputVolume(int volume)
   {
      return OS_NOT_SUPPORTED;
   }

   virtual OsStatus getAudioInputVolume(int& volume) const
   {
      return OS_NOT_SUPPORTED;
   }

   virtual OsStatus setAudioInputVolume(int volume)
   {
      return OS_NOT_SUPPORTED;
   }

   virtual OsStatus getAudioOutputBalance(int& balance) const
   {
      return OS_NOT_SUPPORTED;
   }

   virtual OsStatus setAudioOutputBalance(int balance)
   {
      return OS_NOT_SUPPORTED;
   }

   virtual OsStatus getAudioOutputVolumeMeterReading(MEDIA_VOLUME_METER_TYPE type,
                                                     double& volume) const
   {
      return OS_NOT_SUPPORTED;
   }

   virtual OsStatus getAudioInputVolumeMeterReading(MEDIA_VOLUME_METER_TYPE type,
                                                    double& volume) const
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Gets mode how to send outbound DTMF
   */
   virtual OsStatus getOutboundDTMFMode(MEDIA_OUTBOUND_DTMF_MODE& mode)
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Return status of echo cancellation
   */
   virtual OsStatus getAudioAECMode(MEDIA_AEC_MODE& mode) const 
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Return status of noise reduction
   */
   virtual OsStatus getAudioNoiseReductionMode(MEDIA_NOISE_REDUCTION_MODE& mode) const 
   {
      return OS_NOT_SUPPORTED;
   }

   /**
    * Returns status of voice activity detection
    */
   virtual OsStatus getVADMode(UtlBoolean& bEnable) const
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Get video quality
   */
   virtual OsStatus getVideoQuality(int& quality) const 
   {
      return OS_NOT_SUPPORTED;
   }

   /** 
   * Gets video bit rate
   */
   virtual OsStatus getVideoBitRate(int& bitRate) const 
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Sets video framerate
   */
   virtual OsStatus setVideoFramerate(int framerate)
   {
      return OS_NOT_SUPPORTED;
   }

   /** 
   * Gets video frame rate
   */
   virtual OsStatus getVideoFrameRate(int& frameRate) const 
   {
      return OS_NOT_SUPPORTED;
   }

   /** 
   * Get cpu usage
   */  
   virtual OsStatus getVideoCpuValue(int& cpuValue)
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Gets a list of UtlStrings representing available video capture devices.
   */
   virtual OsStatus getVideoCaptureDevices(UtlSList& videoDevices) const 
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Gets the current video device string.
   */
   virtual OsStatus getVideoCaptureDevice(UtlString& videoDevice) 
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Get the connection idle timeout
   */
   virtual OsStatus getConnectionIdleTimeout(int& idleTimeout) const 
   {
      return OS_NOT_SUPPORTED;
   }


   /* ============================ INQUIRY =================================== */

   /**
   * Return status of AGC
   */ 
   virtual OsStatus isAGCEnabled(UtlBoolean& bEnable) const
   {
      return OS_NOT_SUPPORTED;
   }

   /**
   * Returns status of reception of DTMF
   */
   virtual OsStatus isInboundDTMFEnabled(MEDIA_INBOUND_DTMF_MODE mode, UtlBoolean& enabled)
   {
      return OS_NOT_SUPPORTED;
   }

   virtual OsStatus isAudioOutputMuted(UtlBoolean& bIsMuted) const
   {
      return OS_NOT_SUPPORTED;
   }

   virtual OsStatus isAudioInputMuted(UtlBoolean& bIsMuted) const
   {
      return OS_NOT_SUPPORTED;
   }

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   int      miGain;          /**< Gain value stored for unmuting */
   int      miStartRtpPort;  /**< Requested starting rtp port */
   int      miLastRtpPort;   /**< Requested ending rtp port */
   int      miNextRtpPort;   /**< Next available rtp port */
   UtlSList mlistFreePorts;  /**< List of recently freed ports */
   UtlSList mlistBusyPorts;  /**< List of busy ports */
   OsMutex  mlockList;       /**< Lock for port allocation */


   /**
   * Bind the the specified port and see if any data is ready to read for
   * the designated check time.
   *
   * @param iPort Port number to check
   * @param checkTimeMS Number of ms to wait for data.
   */
   virtual UtlBoolean isPortBusy(int iPort, int checkTimeMS);

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /**
   * Disabled copy constructor
   */
   CpMediaInterfaceFactory(const CpMediaInterfaceFactory& rCpMediaInterfaceFactoryImpl);

   /** 
   * Disabled equals operator
   */
   CpMediaInterfaceFactory& operator=(const CpMediaInterfaceFactory& rhs);   
};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpMediaInterfaceFactory_h_
