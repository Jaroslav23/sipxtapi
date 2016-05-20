//
// Copyright (C) 2005-2007 SIPez LLC.
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

// SYSTEM INCLUDES
#include <stdlib.h>

// APPLICATION INCLUDES
#include "os/OsConfigDb.h"
#include "include/SipXMediaFactoryImpl.h"
#include "include/SipXMediaInterfaceImpl.h"
#include "sdp/SdpCodec.h"
#include <sdp/SdpCodecFactory.h>
#include "mp/MpMediaTask.h"
#include "mp/MpMisc.h"
#include "mp/MpCallFlowGraph.h"
#include "sdp/SdpCodecList.h"
#include "mi/CpMediaInterfaceFactoryFactory.h"
#include "mp/MpAudioDriverDefs.h"
#include "mp/MpAudioDriverManager.h"
#include "mi/CpAudioDeviceInfo.h"

#ifndef DISABLE_LOCAL_AUDIO
#include <mp/MpAudioStreamInfo.h>
#endif

#ifdef INCLUDE_RTCP /* [ */
#include "rtcp/RTCManager.h"
#endif /* INCLUDE_RTCP ] */

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// GLOBAL FUNCTION
// STATIC VARIABLE INITIALIZATIONS
int SipXMediaFactoryImpl::miInstanceCount = 0;
CpMediaInterfaceFactory* spFactory = NULL;
int siInstanceCount = 0;

extern "C" CpMediaInterfaceFactory* cpDefaultMediaFactoryFactory(OsConfigDb* pConfigDb)
{
    if (!spFactory)
    {
        spFactory = new SipXMediaFactoryImpl(pConfigDb);
    }
    siInstanceCount++;
    
    return spFactory;
}

#ifndef DISABLE_DEFAULT_PHONE_MEDIA_INTERFACE_FACTORY
extern "C" CpMediaInterfaceFactory* sipXmediaFactoryFactory(OsConfigDb* pConfigDb)
{
    return(cpDefaultMediaFactoryFactory(pConfigDb));
}
#endif

extern "C" void sipxDestroyMediaFactoryFactory()
{
    if (siInstanceCount > 0)
    {
        siInstanceCount--;
        if (siInstanceCount == 0 && spFactory)
        {
            delete spFactory;
            spFactory = NULL;
        }
    }
}

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipXMediaFactoryImpl::SipXMediaFactoryImpl(OsConfigDb* pConfigDb)
: m_bIsAudioOutputMuted(FALSE)
, m_bIsAudioInputMuted(FALSE)
, m_fMutedAudioOutputVolume(0.0)
, m_fMutedAudioInputVolume(0.0)
{    
    // Start audio subsystem if still not started.
    if (miInstanceCount == 0)
    {
        mpStartUp();
    }

#ifdef INCLUDE_RTCP /* [ */
    mpiRTCPControl = CRTCManager::getRTCPControl();
#endif /* INCLUDE_RTCP ] */

    miGain = 7 ;
    ++miInstanceCount;

    // We are missing synchronization -- give the tasks time to start
    OsTask::delay(100) ;
}


// Destructor
SipXMediaFactoryImpl::~SipXMediaFactoryImpl()
{
    // TODO: Shutdown
    --miInstanceCount;
    if (miInstanceCount == 0)
    {
        mpShutdown();
    }
}

/* ============================ MANIPULATORS ============================== */

CpMediaInterface* SipXMediaFactoryImpl::createMediaInterface(OsMsgQ* pInterfaceNotificationQueue,///< queue for sending interface notifications
                                                             const SdpCodecList* pCodecList,///< list of SdpCodec instances
															                const char* publicAddress,///< ignored
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
                                                             int iTurnKeepAlivePeriodSecs,
                                                             UtlBoolean bEnableICE) 
{
    return new SipXMediaInterfaceImpl(this, pInterfaceNotificationQueue, pCodecList, publicAddress, localIPAddress, 
            locale, expeditedIpTos, szStunServer,
            iStunPort, iStunKeepAliveSecs, szTurnServer, iTurnPort, 
            szTurnUsername, szTurnPassword, iTurnKeepAlivePeriodSecs, 
            bEnableICE) ;
}

OsStatus SipXMediaFactoryImpl::getAudioInputDeviceCount(int& count) const
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      count = pAudioManager->getInputDeviceCount();
      return OS_SUCCESS;
   }

   return OS_FAILED;
#endif

   return OS_NOT_SUPPORTED;
}

OsStatus SipXMediaFactoryImpl::getAudioOutputDeviceCount(int& count) const
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      count = pAudioManager->getOutputDeviceCount();
      return OS_SUCCESS;
   }

   return OS_FAILED;
#endif

   return OS_NOT_SUPPORTED;
}

OsStatus SipXMediaFactoryImpl::getAudioInputDeviceInfo(int deviceIndex, CpAudioDeviceInfo& deviceInfo) const
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      MpAudioDeviceInfo mDeviceInfo;
      OsStatus res = pAudioManager->getInputDeviceInfo(deviceIndex, mDeviceInfo);

      if (res == OS_SUCCESS)
      {
         deviceInfo.m_bIsInput = TRUE;
         deviceInfo.m_defaultSampleRate = mDeviceInfo.getDefaultSampleRate();
         deviceInfo.m_deviceName = mDeviceInfo.getName();
         deviceInfo.m_driverName = mDeviceInfo.getHostApiName();
         deviceInfo.m_maxChannels = mDeviceInfo.getMaxInputChannels();
      }
      
      return res;
   }

   return OS_FAILED;
#endif

   return OS_NOT_SUPPORTED;
}

OsStatus SipXMediaFactoryImpl::getAudioOutputDeviceInfo(int deviceIndex, CpAudioDeviceInfo& deviceInfo) const
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      MpAudioDeviceInfo mDeviceInfo;
      OsStatus res = pAudioManager->getOutputDeviceInfo(deviceIndex, mDeviceInfo);

      if (res == OS_SUCCESS)
      {
         deviceInfo.m_bIsInput = FALSE;
         deviceInfo.m_defaultSampleRate = mDeviceInfo.getDefaultSampleRate();
         deviceInfo.m_deviceName = mDeviceInfo.getName();
         deviceInfo.m_driverName = mDeviceInfo.getHostApiName();
         deviceInfo.m_maxChannels = mDeviceInfo.getMaxOutputChannels();
      }

      return res;
   }

   return OS_FAILED;
#endif

   return OS_NOT_SUPPORTED;
}

OsStatus SipXMediaFactoryImpl::setAudioOutputDevice(const UtlString& device, const UtlString& driverName) 
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      return pAudioManager->setCurrentOutputDevice(device, driverName);
   }

   return OS_FAILED;
#endif

   return OS_NOT_SUPPORTED;
}

OsStatus SipXMediaFactoryImpl::setAudioInputDevice(const UtlString& device, const UtlString& driverName) 
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      return pAudioManager->setCurrentInputDevice(device, driverName);
   }

   return OS_FAILED;
#endif

   return OS_NOT_SUPPORTED;
}

OsStatus SipXMediaFactoryImpl::setAudioDriverLatency(double inputLatency, double outputLatency)
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      pAudioManager->setInitialInputStreamLatency(inputLatency);
      pAudioManager->setInitialOutputStreamLatency(outputLatency);
   }

   return OS_SUCCESS;
#endif

   return OS_NOT_SUPPORTED;
}

OsStatus SipXMediaFactoryImpl::getAudioDriverLatency(double& inputLatency, double& outputLatency)
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      MpAudioStreamId inputStreamId = pAudioManager->getInputAudioStream();
      MpAudioStreamId outputStreamId = pAudioManager->getOutputAudioStream();
      if (inputStreamId != 0)
      {
         // input stream is active, get real latency
         MpAudioStreamInfo streamInfo;
         OsStatus res = pAudioManager->getInputStreamInfo(streamInfo);
         if (res == OS_SUCCESS)
         {
            inputLatency = streamInfo.getInputLatency();
         }
         else
         {
            outputLatency = 0.0;
         }
      }
      else
      {
         // get initial latency
         inputLatency = pAudioManager->getInitialInputStreamLatency();
      }

      if (outputStreamId != 0)
      {
         // output stream is active, get real latency
         MpAudioStreamInfo streamInfo;
         OsStatus res = pAudioManager->getOutputStreamInfo(streamInfo);
         if (res == OS_SUCCESS)
         {
            outputLatency = streamInfo.getOutputLatency();
         }
         else
         {
            outputLatency = 0.0;
         }
      }
      else
      {
         // get initial latency
         outputLatency = pAudioManager->getInitialOutputStreamLatency();
      }
   }

   return OS_SUCCESS;
#endif

   return OS_NOT_SUPPORTED;
}

OsStatus SipXMediaFactoryImpl::muteAudioOutput(UtlBoolean bMute)
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      if (bMute)
      {
         // if we want to mute
         if (!m_bIsAudioOutputMuted)
         {
            // if not muted, save volume
            m_fMutedAudioOutputVolume = pAudioManager->getPCMOutputVolume();
            // if output volume cannot be discovered, unmute won't work
            if (m_fMutedAudioOutputVolume < 0) m_fMutedAudioOutputVolume = 0.0f;
            m_bIsAudioOutputMuted = TRUE;
         }

         pAudioManager->setPCMOutputVolume(0.0);
         // mute after mute will succeed
         return OS_SUCCESS;
      }
      else
      {
         // if we want to unmute
         if (m_bIsAudioOutputMuted)
         {
            // if muted, restore volume
            pAudioManager->setPCMOutputVolume(m_fMutedAudioOutputVolume);
            m_bIsAudioOutputMuted = FALSE;
         }

         // unmute after unmute will succeed
         return OS_SUCCESS;
      }
   }

   return OS_FAILED;
#endif

   return OS_NOT_SUPPORTED;
}

OsStatus SipXMediaFactoryImpl::muteAudioInput(UtlBoolean bMute) 
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      if (bMute)
      {
         // if we want to mute
         if (!m_bIsAudioInputMuted)
         {
            // if not muted, save volume
            m_fMutedAudioInputVolume = pAudioManager->getInputVolume();
            // if input volume cannot be discovered, unmute won't work
            if (m_fMutedAudioInputVolume < 0) m_fMutedAudioInputVolume = 0.0f;
            m_bIsAudioInputMuted = TRUE;
         }

         pAudioManager->setInputVolume(0.0);
         // mute after mute will succeed
         return OS_SUCCESS;
      }
      else
      {
         // if we want to unmute
         if (m_bIsAudioInputMuted)
         {
            // if muted, restore volume
            pAudioManager->setInputVolume(m_fMutedAudioInputVolume);
            m_bIsAudioInputMuted = FALSE;
         }

         // unmute after unmute will succeed
         return OS_SUCCESS;
      }
   }

   return OS_FAILED;
#endif

   return OS_NOT_SUPPORTED;
}

OsStatus SipXMediaFactoryImpl::getAudioAECMode(MEDIA_AEC_MODE& mode) const
{
   if (MpCallFlowGraph::getAECMode((FLOWGRAPH_AEC_MODE&)mode)) {
      return OS_SUCCESS;
   } else {
      return OS_NOT_SUPPORTED; 
   }
}

OsStatus SipXMediaFactoryImpl::setAudioAECMode(const MEDIA_AEC_MODE mode)
{
  if (MpCallFlowGraph::setAECMode((FLOWGRAPH_AEC_MODE)mode)) {
    return OS_SUCCESS;
  } else {
    return OS_NOT_SUPPORTED; 
  }
}

OsStatus SipXMediaFactoryImpl::isAGCEnabled(UtlBoolean& bEnable) const {
   if (MpCallFlowGraph::getAGC(bEnable)) {
      return OS_SUCCESS;
   } else {
      return OS_NOT_SUPPORTED; 
   }
}

OsStatus SipXMediaFactoryImpl::enableAGC(UtlBoolean bEnable) {
  if (MpCallFlowGraph::setAGC(bEnable)) {
    return OS_SUCCESS;
  } else {
    return OS_NOT_SUPPORTED; 
  }
}

OsStatus SipXMediaFactoryImpl::getAudioNoiseReductionMode(MEDIA_NOISE_REDUCTION_MODE& mode) const {
   UtlBoolean bMode = FALSE;
   if (MpCallFlowGraph::getAudioNoiseReduction(bMode)) {
      if (bMode) {
         mode = MEDIA_NOISE_REDUCTION_HIGH;
      } else {
         mode = MEDIA_NOISE_REDUCTION_DISABLED;
      }
      return OS_SUCCESS;
   } else {
      return OS_NOT_SUPPORTED;
   }
}

OsStatus SipXMediaFactoryImpl::setAudioNoiseReductionMode(const MEDIA_NOISE_REDUCTION_MODE mode) {
  if (mode == MEDIA_NOISE_REDUCTION_DISABLED)
  {
    if (MpCallFlowGraph::setAudioNoiseReduction(FALSE))
    {
      return OS_SUCCESS;
    }
  }
  else
  {
    if (MpCallFlowGraph::setAudioNoiseReduction(TRUE))
    {
      return OS_SUCCESS;
    }
  }
  return OS_NOT_SUPPORTED;
}

OsStatus SipXMediaFactoryImpl::getVADMode(UtlBoolean& bEnable) const
{
   bEnable = MpCallFlowGraph::isVADEnabled();
   return OS_SUCCESS;
}

OsStatus SipXMediaFactoryImpl::setVADMode(UtlBoolean bEnable)
{
   MpCallFlowGraph::enableVAD(bEnable);
   return OS_SUCCESS;
}

OsStatus SipXMediaFactoryImpl::enableInboundDTMF(MEDIA_INBOUND_DTMF_MODE mode, UtlBoolean enable)
{
   switch(mode)
   {
   case MEDIA_INBOUND_DTMF_INBAND:
      MpCallFlowGraph::enableInboundInBandDTMF(enable);
      break;
   case MEDIA_INBOUND_DTMF_RFC2833:
      MpCallFlowGraph::enableInboundRFC2833DTMF(enable);
      break;
   default:
      return OS_FAILED;
   }

   return OS_SUCCESS;
}

OsStatus SipXMediaFactoryImpl::setOutboundDTMFMode(MEDIA_OUTBOUND_DTMF_MODE mode)
{
   MpCallFlowGraph::enableSendInbandDTMF(mode == MEDIA_OUTBOUND_DTMF_INBAND);

   return OS_SUCCESS;
}

OsStatus SipXMediaFactoryImpl::getOutboundDTMFMode(MEDIA_OUTBOUND_DTMF_MODE& mode)
{
   UtlBoolean inBandDtmfEnabled = MpCallFlowGraph::isSendInbandDTMFEnabled();
   if (inBandDtmfEnabled)
   {
      mode = MEDIA_OUTBOUND_DTMF_INBAND;
   }
   else
   {
      mode = MEDIA_OUTBOUND_DTMF_RFC2833;
   }

   return OS_SUCCESS;
}

UtlString SipXMediaFactoryImpl::getAllSupportedAudioCodecs() const
{
   UtlString supportedCodecs = 
      "TELEPHONE-EVENT PCMU PCMA "
#ifdef HAVE_SPEEX // [
      "SPEEX_5 SPEEX_8 SPEEX_11 SPEEX_15 SPEEX_18 SPEEX_24 "
#ifdef ENABLE_WIDEBAND_AUDIO
      "SPEEX_WB_9 SPEEX_WB_12 SPEEX_WB_16 SPEEX_WB_20 SPEEX_WB_23 SPEEX_WB_27 SPEEX_WB_34 SPEEX_WB_42 "
      "SPEEX_UWB_11 SPEEX_UWB_14 SPEEX_UWB_18 SPEEX_UWB_22 SPEEX_UWB_25 SPEEX_UWB_29 SPEEX_UWB_36 SPEEX_UWB_44 "
#endif // ENABLE_WIDEBAND_AUDIO ]
#endif // HAVE_SPEEX ]
#ifdef ENABLE_WIDEBAND_AUDIO
      "L16_8000_MONO L16_11025_MONO L16_16000_MONO L16_22050_MONO L16_24000_MONO L16_32000_MONO L16_44100_MONO L16_48000_MONO "
#endif // ENABLE_WIDEBAND_AUDIO ]
#ifdef HAVE_GSM // [
      "GSM "
#endif // HAVE_GSM ]
#ifdef HAVE_ILBC // [
      "ILBC_20MS ILBC_30MS "
#endif // HAVE_ILBC ]
#ifdef HAVE_INTEL_IPP // [
      "G723.1 G728 G729 G729D G729E AMR_4750 AMR_10200 "
#ifdef ENABLE_WIDEBAND_AUDIO
      "AMR_WB_12650 AMR_WB_23850 G722.1_16 G722.1_24 G722.1_32 G729.1_8000 G729.1_12000 G729.1_14000 G729.1_16000 G729.1_18000 "
      "G729.1_20000 G729.1_22000 G729.1_24000 G729.1_26000 G729.1_28000 G729.1_30000 G729.1_32000 "
#endif // ENABLE_WIDEBAND_AUDIO ]
#endif // HAVE_INTEL_IPP ]
#ifdef HAVE_SPAN_DSP // [
#ifdef ENABLE_WIDEBAND_AUDIO
      "G722 "
#endif // ENABLE_WIDEBAND_AUDIO ]
      "G726_16 G726_24 G726_32 G726_40 "
#endif // HAVE_SPAN_DSP ]
      ;
   return supportedCodecs;
}

UtlString SipXMediaFactoryImpl::getAllSupportedVideoCodecs() const
{
   return "";
}

OsStatus SipXMediaFactoryImpl::setConnectionIdleTimeout(const int idleTimeout)
{
   return MpCallFlowGraph::setConnectionIdleTimeout(idleTimeout);
}

OsStatus SipXMediaFactoryImpl::buildAllCodecList(SdpCodecList& codecList)
{
   codecList.clearCodecs();
   codecList.addCodecs(getAllSupportedAudioCodecs());
   codecList.addCodecs(getAllSupportedVideoCodecs());
   codecList.bindPayloadIds();
   return OS_SUCCESS;
}

OsStatus SipXMediaFactoryImpl::buildCodecList(SdpCodecList& codecList, 
                                              const UtlString& sAudioPreferences,
                                              const UtlString& sVideoPreferences)
{
   OsStatus rc = OS_SUCCESS;
   codecList.clearCodecs();

   // add preferred audio codecs first
   if (sAudioPreferences.length() > 0)
   {
      UtlString audioCodecs(SdpCodecFactory::getFixedAudioCodecs(sAudioPreferences));
      codecList.addCodecs(audioCodecs);
   }
   else
   {
      // Build up all supported audio codecs
      codecList.addCodecs(getAllSupportedAudioCodecs());
   }

   // add preferred video codecs first
   if (sVideoPreferences.length() > 0)
   {
      codecList.addCodecs(sVideoPreferences);
   }
   else
   {
      // Build up all supported video codecs
      codecList.addCodecs(getAllSupportedVideoCodecs());
   }

   codecList.bindPayloadIds();

   return rc;
}


OsStatus SipXMediaFactoryImpl::updateVideoPreviewWindow(void* displayContext) 
{
    return OS_NOT_SUPPORTED ;
}


/* ============================ ACCESSORS ================================= */

OsStatus SipXMediaFactoryImpl::getCurrentAudioOutputDevice(CpAudioDeviceInfo& deviceInfo) const
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      MpAudioDeviceInfo mDeviceInfo;
      OsStatus res = pAudioManager->getCurrentOutputDevice(mDeviceInfo);
      if (res == OS_SUCCESS)
      {
         deviceInfo.m_bIsInput = FALSE;
         deviceInfo.m_defaultSampleRate = mDeviceInfo.getDefaultSampleRate();
         deviceInfo.m_deviceName = mDeviceInfo.getName();
         deviceInfo.m_driverName = mDeviceInfo.getHostApiName();
         deviceInfo.m_maxChannels = mDeviceInfo.getMaxOutputChannels();
      }
      
      return res;
   }

   return OS_FAILED;
#endif

   return OS_NOT_SUPPORTED;
}


OsStatus SipXMediaFactoryImpl::getCurrentAudioInputDevice(CpAudioDeviceInfo& deviceInfo) const
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      MpAudioDeviceInfo mDeviceInfo;
      OsStatus res = pAudioManager->getCurrentInputDevice(mDeviceInfo);
      if (res == OS_SUCCESS)
      {
         deviceInfo.m_bIsInput = TRUE;
         deviceInfo.m_defaultSampleRate = mDeviceInfo.getDefaultSampleRate();
         deviceInfo.m_deviceName = mDeviceInfo.getName();
         deviceInfo.m_driverName = mDeviceInfo.getHostApiName();
         deviceInfo.m_maxChannels = mDeviceInfo.getMaxInputChannels();
      }

      return res;
   }

   return OS_FAILED;
#endif

   return OS_NOT_SUPPORTED;
}

OsStatus SipXMediaFactoryImpl::setVideoPreviewDisplay(void* pDisplay)
{
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus SipXMediaFactoryImpl::setVideoQuality(int quality)
{
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus SipXMediaFactoryImpl::setVideoParameters(int bitRate, int frameRate)
{
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus SipXMediaFactoryImpl::getVideoQuality(int& quality) const
{
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus SipXMediaFactoryImpl::getVideoBitRate(int& bitRate) const
{
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus SipXMediaFactoryImpl::getVideoFrameRate(int& frameRate) const
{
    return OS_NOT_YET_IMPLEMENTED;
}

OsStatus SipXMediaFactoryImpl::getAudioInputMixerName(UtlString& name) const
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      pAudioManager->getInputMixerName(name);
      return OS_SUCCESS;
   }

   return OS_FAILED;
#endif

   return OS_NOT_SUPPORTED;
}

OsStatus SipXMediaFactoryImpl::getAudioOutputMixerName(UtlString& name) const
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      pAudioManager->getOutputMixerName(name);
      return OS_SUCCESS;
   }

   return OS_FAILED;
#endif

   return OS_NOT_SUPPORTED;
}

OsStatus SipXMediaFactoryImpl::getAudioMasterVolume(int& volume) const
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      float fVolume = pAudioManager->getMasterVolume();
      if (fVolume >= 0.0f) {
         volume = (int)(fVolume * 100 + 0.5f);
         return OS_SUCCESS;
      }
   }

   return OS_FAILED;
#endif

   return OS_NOT_SUPPORTED;
}

OsStatus SipXMediaFactoryImpl::setAudioMasterVolume(int volume)
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      pAudioManager->setMasterVolume((float)volume / 100);
      return OS_SUCCESS;
   }

   return OS_FAILED;
#endif

   return OS_NOT_SUPPORTED;
}

OsStatus SipXMediaFactoryImpl::getAudioPCMOutputVolume(int& volume) const
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      float fVolume = pAudioManager->getPCMOutputVolume();
      if (fVolume >= 0.0f) {
         volume = (int)(fVolume * 100 + 0.5f);
         return OS_SUCCESS;
      }
   }

   return OS_FAILED;
#endif

   return OS_NOT_SUPPORTED;
}

OsStatus SipXMediaFactoryImpl::setAudioPCMOutputVolume(int volume)
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      if (!m_bIsAudioOutputMuted)
      {
         // output is not muted, set volume
         pAudioManager->setPCMOutputVolume((float)volume / 100);
      }
      else
      {
         // output is muted, just update internal variable
         m_fMutedAudioOutputVolume = (float)volume / 100;
      }
      
      return OS_SUCCESS;
   }

   return OS_FAILED;
#endif

   return OS_NOT_SUPPORTED;
}

OsStatus SipXMediaFactoryImpl::getAudioInputVolume(int& volume) const
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      float fVolume = pAudioManager->getInputVolume();
      if (fVolume >= 0.0f) {
         volume = (int)(fVolume * 100 + 0.5f);
         return OS_SUCCESS;
      }
   }

   return OS_FAILED;
#endif

   return OS_NOT_SUPPORTED;
}

OsStatus SipXMediaFactoryImpl::setAudioInputVolume(int volume)
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      if (!m_bIsAudioInputMuted)
      {
         // input is not muted, set volume
         pAudioManager->setInputVolume((float)volume / 100);
      }
      else
      {
         // input is muted, just update internal variable
         m_fMutedAudioInputVolume = (float)volume / 100;
      }
      
      return OS_SUCCESS;
   }

   return OS_FAILED;
#endif

   return OS_NOT_SUPPORTED;
}

OsStatus SipXMediaFactoryImpl::getAudioOutputBalance(int& balance) const
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      float fBalance = pAudioManager->getOutputBalance();
      balance = (int)(fBalance * 100);
      return OS_SUCCESS;
   }

   return OS_FAILED;
#endif

   return OS_NOT_SUPPORTED;
}

OsStatus SipXMediaFactoryImpl::setAudioOutputBalance(int balance)
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      pAudioManager->setOutputBalance((float)balance / 100);
      return OS_SUCCESS;
   }

   return OS_FAILED;
#endif

   return OS_NOT_SUPPORTED;
}

OsStatus SipXMediaFactoryImpl::getAudioOutputVolumeMeterReading(MEDIA_VOLUME_METER_TYPE type,
                                                                double& volume) const
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      return pAudioManager->getOutputVolumeMeterReading((MP_VOLUME_METER_TYPE)type, volume);
   }

   return OS_FAILED;
#endif

   return OS_NOT_SUPPORTED;
}

OsStatus SipXMediaFactoryImpl::getAudioInputVolumeMeterReading(MEDIA_VOLUME_METER_TYPE type,
                                                               double& volume) const
{
#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   if (pAudioManager)
   {
      return pAudioManager->getInputVolumeMeterReading((MP_VOLUME_METER_TYPE)type, volume);
   }

   return OS_FAILED;
#endif

   return OS_NOT_SUPPORTED;
}

/* ============================ INQUIRY =================================== */

OsStatus SipXMediaFactoryImpl::isInboundDTMFEnabled(MEDIA_INBOUND_DTMF_MODE mode, UtlBoolean& enabled)
{
   switch(mode)
   {
   case MEDIA_INBOUND_DTMF_INBAND:
      enabled = MpCallFlowGraph::isInboundInBandDTMFEnabled();
      break;
   case MEDIA_INBOUND_DTMF_RFC2833:
      enabled = MpCallFlowGraph::isInboundRFC2833DTMFEnabled();
      break;
   default:
      return OS_FAILED;
   }

   return OS_SUCCESS;  
}

OsStatus SipXMediaFactoryImpl::isAudioOutputMuted(UtlBoolean& bIsMuted) const
{
   bIsMuted = m_bIsAudioOutputMuted;
   return OS_SUCCESS;
}

OsStatus SipXMediaFactoryImpl::isAudioInputMuted(UtlBoolean& bIsMuted) const
{
   bIsMuted = m_bIsAudioInputMuted;
   return OS_SUCCESS;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


