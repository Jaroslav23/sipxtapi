//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
//
// Copyright (C) 2005-2007 SIPez LLC.
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

// SYSTEM INCLUDES
#include <assert.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#endif /* _WIN32 */

// APPLICATION INCLUDES
#include <tapi/sipXtapi.h>
#include "tapi/SipXAudio.h"
#include <tapi/SipXCore.h>
#include "cp/XCpCallManager.h"
#include <sdp/SdpCodecFactory.h>
#include "mi/CpMediaInterfaceFactory.h"
#include "mi/CpMediaInterfaceFactoryFactory.h"
#include <mi/CpAudioDeviceInfo.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS


void initAudioDevices(SIPX_INSTANCE_DATA& pInst)
{
#if defined(_WIN32)
   WAVEOUTCAPS outcaps;
   WAVEINCAPS  incaps;
   int numDevices;
   MMRESULT result;

   numDevices = waveInGetNumDevs();
   for (int i = 0; i < numDevices && i < MAX_AUDIO_DEVICES; i++)
   {
      result = waveInGetDevCaps(i, &incaps, sizeof(WAVEINCAPS));
      assert(result == MMSYSERR_NOERROR);
      pInst.inputAudioDevices[i] = SAFE_STRDUP(incaps.szPname);
   }
   pInst.nInputAudioDevices = numDevices;

   numDevices = waveOutGetNumDevs();
   for (int i = 0; i < numDevices && i < MAX_AUDIO_DEVICES; i++)
   {
      result = waveOutGetDevCaps(i, &outcaps, sizeof(WAVEOUTCAPS));
      assert(result == MMSYSERR_NOERROR);
      pInst.outputAudioDevices[i] = SAFE_STRDUP(outcaps.szPname) ;
   }
   pInst.nOutputAudioDevices = numDevices;

#else
   pInst.inputAudioDevices[0] = SAFE_STRDUP("Default");
   pInst.outputAudioDevices[0] = SAFE_STRDUP("Default");
#endif
}


void freeAudioDevices(SIPX_INSTANCE_DATA& pInst)
{
   for (int i = 0; i < MAX_AUDIO_DEVICES; i++)
   {
      if (pInst.inputAudioDevices[i])
      {
         free(pInst.inputAudioDevices[i]);
         pInst.inputAudioDevices[i] = NULL;
      }

      if (pInst.outputAudioDevices[i])
      {
         free(pInst.outputAudioDevices[i]);
         pInst.outputAudioDevices[i] = NULL;
      }
   }
}

SIPXTAPI_API SIPX_RESULT sipxAudioGetInputMixerName(const SIPX_INST hInst, char* name, int buffSize)
{
   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst && name && buffSize > 0)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();
      memset(name, 0, buffSize);

      if (pInterface)
      {
         OsStatus rc = OS_FAILED;
         UtlString sName;
         rc = pInterface->getAudioInputMixerName(sName);

         if (rc == OS_SUCCESS)
         {
            SAFE_STRNCPY(name, sName.data(), buffSize);
            sr = SIPX_RESULT_SUCCESS;
         }
      }      
   }

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxAudioGetOutputMixerName(const SIPX_INST hInst, char* name, int buffSize)
{
   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst && name && buffSize > 0)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();
      memset(name, 0, buffSize);

      if (pInterface)
      {
         OsStatus rc = OS_FAILED;
         UtlString sName;
         rc = pInterface->getAudioOutputMixerName(sName);

         if (rc == OS_SUCCESS)
         {
            SAFE_STRNCPY(name, sName.data(), buffSize);
            sr = SIPX_RESULT_SUCCESS;
         }
      }      
   }

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxAudioGetMasterVolume(const SIPX_INST hInst,
                                                  int* iLevel)
{
   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst && iLevel)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         OsStatus rc = OS_FAILED;
         rc = pInterface->getAudioMasterVolume(*iLevel);

         if (rc == OS_SUCCESS)
         {
            sr = SIPX_RESULT_SUCCESS;
         }
      }      
   }

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxAudioSetMasterVolume(const SIPX_INST hInst,
                                                  int iLevel)
{
   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         // Validate volume is within range
         if (iLevel >= OUTPUT_VOLUME_MIN && iLevel <= OUTPUT_VOLUME_MAX)
         {
            OsStatus rc = OS_FAILED;
            rc = pInterface->setAudioMasterVolume(iLevel);

            if (rc == OS_SUCCESS)
            {
               sr = SIPX_RESULT_SUCCESS;
            }
         }
         else
         {
            sr = SIPX_RESULT_INVALID_ARGS;
         }
      }      
   }

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxAudioGetOutputBalance(const SIPX_INST hInst,
                                                   int* iBalance)
{
   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst && iBalance)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         OsStatus rc = OS_FAILED;
         rc = pInterface->getAudioOutputBalance(*iBalance);

         if (rc == OS_SUCCESS)
         {
            sr = SIPX_RESULT_SUCCESS;
         }
      }      
   }

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxAudioSetOutputBalance(const SIPX_INST hInst,
                                                   int iBalance)
{
   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         // Validate volume is within range
         if (iBalance >= BALANCE_MIN && iBalance <= BALANCE_MAX)
         {
            OsStatus rc = OS_FAILED;
            rc = pInterface->setAudioOutputBalance(iBalance);

            if (rc == OS_SUCCESS)
            {
               sr = SIPX_RESULT_SUCCESS;
            }
         }
         else
         {
            sr = SIPX_RESULT_INVALID_ARGS;
         }
      }      
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxAudioSetInputVolume(const SIPX_INST hInst,
                                                 const int iLevel)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioSetInputVolume");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxAudioSetInputVolume hInst=%p iLevel=%d",
      hInst, iLevel);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         // Validate gain is within range
         if (iLevel >= INPUT_VOLUME_MIN && iLevel <= INPUT_VOLUME_MAX)
         {
            OsStatus rc = OS_FAILED;
            rc = pInterface->setAudioInputVolume(iLevel);

            if (rc == OS_SUCCESS)
            {
               sr = SIPX_RESULT_SUCCESS;
            }
         }
         else
         {
            sr = SIPX_RESULT_INVALID_ARGS;
         }
      }      
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxAudioGetInputVolume(const SIPX_INST hInst,
                                                 int* iLevel)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioGetInputVolume");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxAudioGetInputVolume hInst=%p",
      hInst);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst && iLevel)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         OsStatus status = pInterface->getAudioInputVolume(*iLevel);

         if (status == OS_SUCCESS)
         {
            sr = SIPX_RESULT_SUCCESS;
         }
      }      
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxAudioMuteInput(const SIPX_INST hInst,
                                            const int bMute)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioMuteInput");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxAudioMuteInput hInst=%p bMute=%d",
      hInst, bMute);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         // Mute or unmute gain
         OsStatus rc = pInterface->muteAudioInput(bMute);

         if (rc == OS_SUCCESS)
         {
            sr = SIPX_RESULT_SUCCESS;
         }
      }     
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxAudioMuteOutput(const SIPX_INST hInst,
                                             const int bMute)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioMuteOutput");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxAudioMuteOutput hInst=%p bMute=%d",
      hInst, bMute);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         // Mute or unmute speaker
         OsStatus rc = pInterface->muteAudioOutput(bMute);

         if (rc == OS_SUCCESS)
         {
            sr = SIPX_RESULT_SUCCESS;
         }
      }     
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxAudioIsInputMuted(const SIPX_INST hInst,
                                               int* bMuted)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioIsInputMuted");
   OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
      "sipxAudioIsInputMuted hInst=%p",
      hInst);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst && bMuted)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();
      if (pInterface)
      {
         UtlBoolean bIsMuted;
         OsStatus res = pInterface->isAudioInputMuted(bIsMuted);
         if (res == OS_SUCCESS)
         {
            *bMuted = bIsMuted;
            sr = SIPX_RESULT_SUCCESS;
         }         
      }
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxAudioIsOutputMuted(const SIPX_INST hInst,
                                                int* bMuted)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioIsOutputMuted");
   OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioIsOutputMuted hInst=%p", hInst);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst && bMuted)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();
      if (pInterface)
      {
         UtlBoolean bIsMuted;
         OsStatus res = pInterface->isAudioOutputMuted(bIsMuted);
         if (res == OS_SUCCESS)
         {
            *bMuted = bIsMuted;
            sr = SIPX_RESULT_SUCCESS;
         }         
      }
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxAudioSetOutputVolume(const SIPX_INST hInst,
                                                  const int iLevel)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioSetOutputVolume");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxAudioSetOutputVolume hInst=%p iLevel=%d",
      hInst, iLevel);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         // Validate Params
         if (iLevel >= OUTPUT_VOLUME_MIN && iLevel <= OUTPUT_VOLUME_MAX)
         {
            // the CpMediaInterfaceFactoryImpl always uses a scale of 0 - 100
            OsStatus status = pInterface->setAudioPCMOutputVolume(iLevel);
            if (status == OS_SUCCESS)
            {
               sr = SIPX_RESULT_SUCCESS;
            }
         }
      }      
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxAudioGetOutputVolume(const SIPX_INST hInst,
                                                  int* iLevel)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioGetOutputVolume");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO, "sipxAudioGetOutputVolume hInst=%p", hInst);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst && iLevel)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         OsStatus status = pInterface->getAudioPCMOutputVolume(*iLevel);
         if (status == OS_SUCCESS)
         {
            sr = SIPX_RESULT_SUCCESS;
         }
      }
   }

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxAudioGetInputEnergy(const SIPX_INST hInst,
                                                 SIPX_VOLUME_METER_TYPE type,
                                                 double* level)
{
   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst && level)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         OsStatus status = pInterface->getAudioInputVolumeMeterReading((MEDIA_VOLUME_METER_TYPE)type,
                                                                       *level);
         if (status == OS_SUCCESS)
         {
            sr = SIPX_RESULT_SUCCESS;
         }
      }
   }

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxAudioGetOutputEnergy(const SIPX_INST hInst,
                                                  SIPX_VOLUME_METER_TYPE type,
                                                  double* level)
{
   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst && level)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         OsStatus status = pInterface->getAudioOutputVolumeMeterReading((MEDIA_VOLUME_METER_TYPE)type,
                                                                        *level);
         if (status == OS_SUCCESS)
         {
            sr = SIPX_RESULT_SUCCESS;
         }
      }
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxAudioSetAECMode(const SIPX_INST hInst,
                                             const SIPX_AEC_MODE mode) 
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioSetAECMode");

   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxAudioSetAECMode hInst=%p mode=%d",
      hInst, mode);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         if (pInterface->setAudioAECMode((MEDIA_AEC_MODE)mode) == OS_SUCCESS)
         {
            sr = SIPX_RESULT_SUCCESS;
         }
      }
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxAudioGetAECMode(const SIPX_INST hInst,
                                             SIPX_AEC_MODE* mode)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioGetAECMode");

   OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
      "sipxAudioGetAECMode hInst=%p",
      hInst);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst && mode)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         MEDIA_AEC_MODE aceMode;
         if (pInterface->getAudioAECMode(aceMode) == OS_SUCCESS)
         {
            *mode = (SIPX_AEC_MODE)aceMode;
            sr = SIPX_RESULT_SUCCESS;
         }
      }
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxAudioSetAGCMode(const SIPX_INST hInst,
                                             const int bEnable) 
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioSetAGCMode");

   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxAudioSetAGCMode hInst=%p enable=%d",
      hInst, bEnable);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         if (pInterface->enableAGC(bEnable) == OS_SUCCESS)
         {
            sr = SIPX_RESULT_SUCCESS;
         }
      }
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxAudioGetAGCMode(const SIPX_INST hInst,
                                             int* bEnabled) 
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioGetAGCMode");

   OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
      "sipxAudioGetAGCMode hInst=%p",
      hInst);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst && bEnabled)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         UtlBoolean bCheck;
         if (pInterface->isAGCEnabled(bCheck) == OS_SUCCESS)
         {
            *bEnabled = bCheck;
            sr = SIPX_RESULT_SUCCESS;
         }
      }      
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxAudioSetNoiseReductionMode(const SIPX_INST hInst,
                                                        const SIPX_NOISE_REDUCTION_MODE mode) 
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioSetNoiseReductionMode");

   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxAudioSetNoiseReductionMode hInst=%p mode=%d",
      hInst, mode);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         if (pInterface->setAudioNoiseReductionMode((MEDIA_NOISE_REDUCTION_MODE)mode) == OS_SUCCESS)
         {
            sr = SIPX_RESULT_SUCCESS;
         }
      }
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxAudioGetNoiseReductionMode(const SIPX_INST hInst,
                                                        SIPX_NOISE_REDUCTION_MODE* mode) 
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioGetNoiseReductionMode");

   OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
      "sipxAudioGetNoiseReductionMode hInst=%p",
      hInst);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst && mode)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         MEDIA_NOISE_REDUCTION_MODE nrMode;
         if (pInterface->getAudioNoiseReductionMode(nrMode) == OS_SUCCESS)
         {
            *mode = (SIPX_NOISE_REDUCTION_MODE)nrMode;
            sr = SIPX_RESULT_SUCCESS;
         }
      }
   }

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxAudioSetVADMode(const SIPX_INST hInst,
                                             int bEnabled) 
{
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxAudioSetVADMode hInst=%p bEnabled=%d",
      hInst, bEnabled);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst)
   {
      CpMediaInterfaceFactory* pInterfaceFactory = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterfaceFactory)
      {
         if (pInterfaceFactory->setVADMode(bEnabled) == OS_SUCCESS)
         {
            SdpCodecFactory::enableCodecVAD(bEnabled); // also enable in SDP codec factory
            sr = SIPX_RESULT_SUCCESS;
         }
      }
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxAudioGetVADMode(const SIPX_INST hInst,
                                             int* bEnabled) 
{
   OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG,
      "sipxAudioGetVADMode hInst=%p",
      hInst);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst && bEnabled)
   {
      CpMediaInterfaceFactory* pInterfaceFactory = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterfaceFactory)
      {
         if (pInterfaceFactory->getVADMode(*bEnabled) == OS_SUCCESS)
         {
            sr = SIPX_RESULT_SUCCESS;
         }
      }
   }

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxAudioGetNumInputDevices(const SIPX_INST hInst,
                                                     int* numDevices)
{
   SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         OsStatus res = pInterface->getAudioInputDeviceCount(*numDevices);
         if (res == OS_SUCCESS)
         {
            rc = SIPX_RESULT_SUCCESS;
         }
      }
   }

   return rc;
}


SIPXTAPI_API SIPX_RESULT sipxAudioGetInputDeviceInfo(const SIPX_INST hInst,
                                                     const int index,
                                                     SIPX_AUDIO_DEVICE* deviceInfo)
{
   SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst && deviceInfo && index >= 0)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         CpAudioDeviceInfo cpDeviceInfo;
         OsStatus res = pInterface->getAudioInputDeviceInfo(index, cpDeviceInfo);
         if (res == OS_SUCCESS)
         {
            SAFE_STRNCPY(deviceInfo->deviceName, cpDeviceInfo.m_deviceName, sizeof(deviceInfo->deviceName));
            SAFE_STRNCPY(deviceInfo->driverName, cpDeviceInfo.m_driverName, sizeof(deviceInfo->deviceName));
            deviceInfo->bIsInput = cpDeviceInfo.m_bIsInput;
            deviceInfo->defaultSampleRate = cpDeviceInfo.m_defaultSampleRate;
            deviceInfo->maxChannels = cpDeviceInfo.m_maxChannels;
            rc = SIPX_RESULT_SUCCESS;
         }
      }
   }

   return rc;
}


SIPXTAPI_API SIPX_RESULT sipxAudioGetNumOutputDevices(const SIPX_INST hInst,
                                                      int* numDevices)
{
   SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         OsStatus res = pInterface->getAudioOutputDeviceCount(*numDevices);
         if (res == OS_SUCCESS)
         {
            rc = SIPX_RESULT_SUCCESS;
         }
      }
   }

   return rc;
}


SIPXTAPI_API SIPX_RESULT sipxAudioGetOutputDeviceInfo(const SIPX_INST hInst,
                                                      const int index,
                                                      SIPX_AUDIO_DEVICE* deviceInfo)
{
   SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst && deviceInfo && index >= 0)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         CpAudioDeviceInfo cpDeviceInfo;
         OsStatus res = pInterface->getAudioOutputDeviceInfo(index, cpDeviceInfo);
         if (res == OS_SUCCESS)
         {
            SAFE_STRNCPY(deviceInfo->deviceName, cpDeviceInfo.m_deviceName, sizeof(deviceInfo->deviceName));
            SAFE_STRNCPY(deviceInfo->driverName, cpDeviceInfo.m_driverName, sizeof(deviceInfo->deviceName));
            deviceInfo->bIsInput = cpDeviceInfo.m_bIsInput;
            deviceInfo->defaultSampleRate = cpDeviceInfo.m_defaultSampleRate;
            deviceInfo->maxChannels = cpDeviceInfo.m_maxChannels;
            rc = SIPX_RESULT_SUCCESS;
         }
      }
   }

   return rc;
}


SIPXTAPI_API SIPX_RESULT sipxAudioSetInputDevice(const SIPX_INST hInst,
                                                 const char* szDevice,
                                                 const char* szDriver)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioSetInputDevice");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxAudioSetInputDevice hInst=%p device=%s",
      hInst, szDevice ? szDevice : "null");

   SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         OsStatus res = pInterface->setAudioInputDevice(szDevice, szDriver);
         if (res == OS_SUCCESS)
         {
            rc = SIPX_RESULT_SUCCESS;
         }
      }
   }

   return rc;
}


SIPXTAPI_API SIPX_RESULT sipxAudioGetInputDevice(const SIPX_INST hInst,
                                                 SIPX_AUDIO_DEVICE* deviceInfo)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioGetInputDevice");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxAudioGetInputDevice hInst=%p", hInst);

   SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst && deviceInfo)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         CpAudioDeviceInfo cpDeviceInfo;
         OsStatus res = pInterface->getCurrentAudioInputDevice(cpDeviceInfo);
         if (res == OS_SUCCESS)
         {
            SAFE_STRNCPY(deviceInfo->deviceName, cpDeviceInfo.m_deviceName, sizeof(deviceInfo->deviceName));
            SAFE_STRNCPY(deviceInfo->driverName, cpDeviceInfo.m_driverName, sizeof(deviceInfo->deviceName));
            deviceInfo->bIsInput = cpDeviceInfo.m_bIsInput;
            deviceInfo->defaultSampleRate = cpDeviceInfo.m_defaultSampleRate;
            deviceInfo->maxChannels = cpDeviceInfo.m_maxChannels;
            rc = SIPX_RESULT_SUCCESS;
         }
      }
   }

   return rc;
}


SIPXTAPI_API SIPX_RESULT sipxAudioSetOutputDevice(const SIPX_INST hInst,
                                                  const char* szDevice,
                                                  const char* szDriver)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioSetOutputDevice");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxAudioSetOutputDevice hInst=%p device=%s",
      hInst, szDevice ? szDevice : "null");

   SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         OsStatus res = pInterface->setAudioOutputDevice(szDevice, szDriver);
         if (res == OS_SUCCESS)
         {
            rc = SIPX_RESULT_SUCCESS;
         }
      }
   }

   return rc;
}


SIPXTAPI_API SIPX_RESULT sipxAudioGetOutputDevice(const SIPX_INST hInst,
                                                  SIPX_AUDIO_DEVICE* deviceInfo)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxAudioGetOutputDevice");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxAudioGetOutputDevice hInst=%p", hInst);

   SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst && deviceInfo)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         CpAudioDeviceInfo cpDeviceInfo;
         OsStatus res = pInterface->getCurrentAudioOutputDevice(cpDeviceInfo);
         if (res == OS_SUCCESS)
         {
            SAFE_STRNCPY(deviceInfo->deviceName, cpDeviceInfo.m_deviceName, sizeof(deviceInfo->deviceName));
            SAFE_STRNCPY(deviceInfo->driverName, cpDeviceInfo.m_driverName, sizeof(deviceInfo->deviceName));
            deviceInfo->bIsInput = cpDeviceInfo.m_bIsInput;
            deviceInfo->defaultSampleRate = cpDeviceInfo.m_defaultSampleRate;
            deviceInfo->maxChannels = cpDeviceInfo.m_maxChannels;
            rc = SIPX_RESULT_SUCCESS;
         }
      }
   }

   return rc;
}

SIPXTAPI_API SIPX_RESULT sipxAudioSetDriverLatency(const SIPX_INST hInst,
                                                   double inputLatency,
                                                   double outputLatency)
{
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO, "sipxAudioSetDriverLatency hInst=%p", hInst);

   SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         OsStatus res = pInterface->setAudioDriverLatency(inputLatency, outputLatency);
         if (res == OS_SUCCESS)
         {
            rc = SIPX_RESULT_SUCCESS;
         }
         else
         {
            rc= SIPX_RESULT_FAILURE;
         }
      }
   }

   return rc;
}

SIPXTAPI_API SIPX_RESULT sipxAudioGetDriverLatency(const SIPX_INST hInst,
                                                   double* inputLatency,
                                                   double* outputLatency)
{
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO, "sipxAudioGetDriverLatency hInst=%p", hInst);

   SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst && inputLatency && outputLatency)
   {
      CpMediaInterfaceFactory* pInterface = pInst->pCallManager->getMediaInterfaceFactory();

      if (pInterface)
      {
         OsStatus res = pInterface->getAudioDriverLatency(*inputLatency, *outputLatency);
         if (res == OS_SUCCESS)
         {
            rc = SIPX_RESULT_SUCCESS;
         }
         else
         {
            rc= SIPX_RESULT_FAILURE;
         }
      }
   }

   return rc;
}
