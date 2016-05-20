//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#include "sipXtapiTestAudio.h"

CPPUNIT_TEST_SUITE_REGISTRATION(sipXtapiTestAudio);

#define GAIN_DEFAULT 70
#define VOLUME_DEFAULT 70

sipXtapiTestAudio::sipXtapiTestAudio() : m_hInst1(NULL)
, m_hInst2(NULL)
{

}

void sipXtapiTestAudio::setUp()
{
   CPPUNIT_ASSERT_EQUAL(sipxConfigInitLogging("sipXtapiTests.txt", LOG_LEVEL_DEBUG), SIPX_RESULT_SUCCESS);

   CPPUNIT_ASSERT_EQUAL(sipxInitialize(&m_hInst1, 8000, 8000, 8001, 8050, 32, HINST_ADDRESS, "127.0.0.1"), SIPX_RESULT_SUCCESS);
   sipxConfigSetConnectionIdleTimeout(m_hInst1, 7);

   CPPUNIT_ASSERT_EQUAL(sipxInitialize(&m_hInst2, 9100, 9100, 9101, 9050, 32, HINST2_ADDRESS, "127.0.0.1"), SIPX_RESULT_SUCCESS);
   sipxConfigSetConnectionIdleTimeout(m_hInst2, 7);
}

void sipXtapiTestAudio::tearDown()
{
   SIPX_RESULT rc;

   if (m_hInst1)
   {
      rc = sipxUnInitialize(m_hInst1);

      if (rc != SIPX_RESULT_SUCCESS)
      {
         printf("\nERROR: sipXtapiTestAudio -- Forcing shutdown of m_hInst1 (0x%08X)\n", m_hInst1);
         CPPUNIT_ASSERT_EQUAL(sipxUnInitialize(m_hInst1, true), SIPX_RESULT_SUCCESS);
      }

      m_hInst1 = NULL;
   }

   if (m_hInst2)
   {
      rc = sipxUnInitialize(m_hInst2);

      if (rc != SIPX_RESULT_SUCCESS)
      {
         printf("\nERROR: sipXtapiTestAudio -- Forcing shutdown of m_hInst2 (0x%08X)\n", m_hInst2);
         CPPUNIT_ASSERT_EQUAL(sipxUnInitialize(m_hInst2, true), SIPX_RESULT_SUCCESS);
      }

      m_hInst2 = NULL;
   }
}

/**
* Test valid bounds: min gain, mid gain, and max gain. 
*/
void sipXtapiTestAudio::testGainAPI() 
{
   for (int iStressFactor = 0; iStressFactor < STRESS_FACTOR; iStressFactor++)
   {
      printf("\ntestGainAPI (%2d of %2d)", iStressFactor + 1, STRESS_FACTOR);

      for (int i = 0; i < 5; i++)
      {
         int iGainLevel = 0;    

         // Set to min
         CPPUNIT_ASSERT_EQUAL(sipxAudioSetInputVolume(m_hInst1, INPUT_VOLUME_MIN), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(sipxAudioGetInputVolume(m_hInst1, &iGainLevel), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(iGainLevel, INPUT_VOLUME_MIN);

         // Set to default
         CPPUNIT_ASSERT_EQUAL(sipxAudioSetInputVolume(m_hInst1, GAIN_DEFAULT), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(sipxAudioGetInputVolume(m_hInst1, &iGainLevel), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(iGainLevel, GAIN_DEFAULT);

         // set to max
         CPPUNIT_ASSERT_EQUAL(sipxAudioSetInputVolume(m_hInst1, INPUT_VOLUME_MAX), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(sipxAudioGetInputVolume(m_hInst1, &iGainLevel), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(iGainLevel, INPUT_VOLUME_MAX);

         // set to max again
         CPPUNIT_ASSERT_EQUAL(sipxAudioSetInputVolume(m_hInst1, INPUT_VOLUME_MAX), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(sipxAudioGetInputVolume(m_hInst1, &iGainLevel), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(iGainLevel, INPUT_VOLUME_MAX);
      }
   }
}

/**
* Verify mute state and that gain is not modified
*/
void sipXtapiTestAudio::testMuteAPI()
{
   for (int iStressFactor = 0; iStressFactor < STRESS_FACTOR; iStressFactor++)
   {
      int iGainLevel;
      int bMuted;

      printf("\ntestMuteAPI (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

      for (int i = 0; i < 5; i++)
      {
         // Set gain to known value
         CPPUNIT_ASSERT_EQUAL(sipxAudioSetInputVolume(m_hInst1, GAIN_DEFAULT), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(sipxAudioGetInputVolume(m_hInst1, &iGainLevel), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(iGainLevel, GAIN_DEFAULT);

         // Test Mute API
         CPPUNIT_ASSERT_EQUAL(sipxAudioMuteInput(m_hInst1, true), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(sipxAudioIsInputMuted(m_hInst1, &bMuted), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(bMuted, TRUE);
         CPPUNIT_ASSERT_EQUAL(sipxAudioGetInputVolume(m_hInst1, &iGainLevel), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(iGainLevel, 0); // when muted gain is 0

         // Test Unmute API, gain should be restored
         CPPUNIT_ASSERT_EQUAL(sipxAudioMuteInput(m_hInst1, false), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(sipxAudioIsInputMuted(m_hInst1, &bMuted), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(bMuted, FALSE);
         CPPUNIT_ASSERT_EQUAL(sipxAudioGetInputVolume(m_hInst1, &iGainLevel), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(iGainLevel, GAIN_DEFAULT);

         // Test Unmute again
         CPPUNIT_ASSERT_EQUAL(sipxAudioMuteInput(m_hInst1, false), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(sipxAudioIsInputMuted(m_hInst1, &bMuted), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(bMuted, FALSE);
         CPPUNIT_ASSERT_EQUAL(sipxAudioGetInputVolume(m_hInst1, &iGainLevel), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(iGainLevel, GAIN_DEFAULT);
      }
   }
}

/*
* Test valid bounds: min, mid, and max. 
*/
void sipXtapiTestAudio::testVolumeAPI() 
{
   for (int iStressFactor = 0; iStressFactor < STRESS_FACTOR; iStressFactor++)
   {
      int iLevel;

      printf("\ntestVolumeAPI (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

      for (int i = 0; i < 5; i++)
      {
         CPPUNIT_ASSERT_EQUAL(sipxAudioSetOutputVolume(m_hInst1, VOLUME_DEFAULT), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(sipxAudioGetOutputVolume(m_hInst1, &iLevel), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(iLevel, VOLUME_DEFAULT);

         CPPUNIT_ASSERT_EQUAL(sipxAudioSetOutputVolume(m_hInst1, OUTPUT_VOLUME_MIN), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(sipxAudioGetOutputVolume(m_hInst1, &iLevel), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(iLevel, OUTPUT_VOLUME_MIN);

         CPPUNIT_ASSERT_EQUAL(sipxAudioSetOutputVolume(m_hInst1, OUTPUT_VOLUME_MAX), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(sipxAudioGetOutputVolume(m_hInst1, &iLevel), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(iLevel, OUTPUT_VOLUME_MAX);

         CPPUNIT_ASSERT_EQUAL(sipxAudioSetMasterVolume(m_hInst1, VOLUME_DEFAULT), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(sipxAudioGetMasterVolume(m_hInst1, &iLevel), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(iLevel, VOLUME_DEFAULT);

         CPPUNIT_ASSERT_EQUAL(sipxAudioSetMasterVolume(m_hInst1, OUTPUT_VOLUME_MIN), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(sipxAudioGetMasterVolume(m_hInst1, &iLevel), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(iLevel, OUTPUT_VOLUME_MIN);

         CPPUNIT_ASSERT_EQUAL(sipxAudioSetMasterVolume(m_hInst1, OUTPUT_VOLUME_MAX), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(sipxAudioGetMasterVolume(m_hInst1, &iLevel), SIPX_RESULT_SUCCESS);
         CPPUNIT_ASSERT_EQUAL(iLevel, OUTPUT_VOLUME_MAX);
      }
   }
}

void sipXtapiTestAudio::testAudioSettings()
{
   int i;

   for (int iStressFactor = 0; iStressFactor < STRESS_FACTOR; iStressFactor++)
   {
      printf("\ntestAudioSettings (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

      // Test Enable AEC

      SIPX_AEC_MODE mode;
      CPPUNIT_ASSERT_EQUAL(sipxAudioSetAECMode(m_hInst1, SIPX_AEC_DISABLED), SIPX_RESULT_SUCCESS);
      CPPUNIT_ASSERT_EQUAL(sipxAudioGetAECMode(m_hInst1, &mode), SIPX_RESULT_SUCCESS);
      CPPUNIT_ASSERT_EQUAL(mode, SIPX_AEC_DISABLED);

      /*    not supported with sipxmedialib
      CPPUNIT_ASSERT_EQUAL(sipxAudioSetAECMode(m_hInst1, SIPX_AEC_SUPPRESS), SIPX_RESULT_SUCCESS);
      CPPUNIT_ASSERT_EQUAL(sipxAudioGetAECMode(m_hInst1, &mode), SIPX_RESULT_SUCCESS);
      CPPUNIT_ASSERT_EQUAL(mode, SIPX_AEC_SUPPRESS);*/

      CPPUNIT_ASSERT_EQUAL(sipxAudioSetAECMode(m_hInst1, SIPX_AEC_CANCEL), SIPX_RESULT_SUCCESS);
      CPPUNIT_ASSERT_EQUAL(sipxAudioGetAECMode(m_hInst1, &mode), SIPX_RESULT_SUCCESS);
      CPPUNIT_ASSERT_EQUAL(mode, SIPX_AEC_CANCEL);

      CPPUNIT_ASSERT_EQUAL(sipxAudioSetAECMode(m_hInst1, SIPX_AEC_CANCEL_AUTO), SIPX_RESULT_SUCCESS);
      CPPUNIT_ASSERT_EQUAL(sipxAudioGetAECMode(m_hInst1, &mode), SIPX_RESULT_SUCCESS);
      CPPUNIT_ASSERT_EQUAL(mode, SIPX_AEC_CANCEL_AUTO);

      // Test Noise Reduction
      SIPX_NOISE_REDUCTION_MODE nrMode;
      CPPUNIT_ASSERT_EQUAL(sipxAudioSetNoiseReductionMode(m_hInst1, SIPX_NOISE_REDUCTION_DISABLED), SIPX_RESULT_SUCCESS);
      CPPUNIT_ASSERT_EQUAL(sipxAudioGetNoiseReductionMode(m_hInst1, &nrMode), SIPX_RESULT_SUCCESS);
      CPPUNIT_ASSERT_EQUAL(nrMode, SIPX_NOISE_REDUCTION_DISABLED);

      CPPUNIT_ASSERT_EQUAL(sipxAudioSetNoiseReductionMode(m_hInst1, SIPX_NOISE_REDUCTION_LOW), SIPX_RESULT_SUCCESS);
      CPPUNIT_ASSERT_EQUAL(sipxAudioGetNoiseReductionMode(m_hInst1, &nrMode), SIPX_RESULT_SUCCESS);
      //CPPUNIT_ASSERT_EQUAL(nrMode, SIPX_NOISE_REDUCTION_LOW); // only works with GIPS
      CPPUNIT_ASSERT_EQUAL(nrMode, SIPX_NOISE_REDUCTION_HIGH); // correct for sipxmedialib

      CPPUNIT_ASSERT_EQUAL(sipxAudioSetNoiseReductionMode(m_hInst1, SIPX_NOISE_REDUCTION_MEDIUM), SIPX_RESULT_SUCCESS);
      CPPUNIT_ASSERT_EQUAL(sipxAudioGetNoiseReductionMode(m_hInst1, &nrMode), SIPX_RESULT_SUCCESS);
      //CPPUNIT_ASSERT_EQUAL(nrMode, SIPX_NOISE_REDUCTION_MEDIUM); // only works with GIPS
      CPPUNIT_ASSERT_EQUAL(nrMode, SIPX_NOISE_REDUCTION_HIGH); // correct for sipxmedialib

      CPPUNIT_ASSERT_EQUAL(sipxAudioSetNoiseReductionMode(m_hInst1, SIPX_NOISE_REDUCTION_HIGH), SIPX_RESULT_SUCCESS);
      CPPUNIT_ASSERT_EQUAL(sipxAudioGetNoiseReductionMode(m_hInst1, &nrMode), SIPX_RESULT_SUCCESS);
      CPPUNIT_ASSERT_EQUAL(nrMode, SIPX_NOISE_REDUCTION_HIGH);

      int numOfDevices = 0;
      char* null = NULL;
#define BUFFER_SIZE 200

      // Test sipxAudioGetNumInputDevices
      CPPUNIT_ASSERT_EQUAL(sipxAudioGetNumInputDevices(NULL, NULL), SIPX_RESULT_INVALID_ARGS);
      CPPUNIT_ASSERT_EQUAL(sipxAudioGetNumInputDevices(m_hInst1, &numOfDevices), SIPX_RESULT_SUCCESS);
      SIPX_AUDIO_DEVICE audioDevice;

      for(i = 0; i < numOfDevices; i++) 
      {
         CPPUNIT_ASSERT_EQUAL(sipxAudioGetInputDeviceInfo(m_hInst1, i, &audioDevice), SIPX_RESULT_SUCCESS);
      }

      CPPUNIT_ASSERT_EQUAL(sipxAudioGetInputDeviceInfo(0, 0, 0), SIPX_RESULT_INVALID_ARGS);
      CPPUNIT_ASSERT_EQUAL(sipxAudioGetInputDeviceInfo(0, 0, &audioDevice), SIPX_RESULT_INVALID_ARGS);
      // checks if there are no more devices
      CPPUNIT_ASSERT_EQUAL(sipxAudioGetInputDeviceInfo(m_hInst1, numOfDevices, &audioDevice), SIPX_RESULT_INVALID_ARGS);
      CPPUNIT_ASSERT_EQUAL(sipxAudioGetInputDeviceInfo(m_hInst1, -1, &audioDevice), SIPX_RESULT_INVALID_ARGS);

      numOfDevices = 0;

      // Test sipxAudioGetNumOutputDevices
      CPPUNIT_ASSERT_EQUAL(sipxAudioGetNumOutputDevices(NULL, NULL), SIPX_RESULT_INVALID_ARGS);
      CPPUNIT_ASSERT_EQUAL(sipxAudioGetNumOutputDevices(m_hInst1, &numOfDevices), SIPX_RESULT_SUCCESS);
      for(i = 0; i < numOfDevices; i++) 
      {
         CPPUNIT_ASSERT_EQUAL(sipxAudioGetOutputDeviceInfo(m_hInst1, i, &audioDevice), SIPX_RESULT_SUCCESS);
      }

      CPPUNIT_ASSERT_EQUAL(sipxAudioGetOutputDeviceInfo(0, 0, 0), SIPX_RESULT_INVALID_ARGS);
      CPPUNIT_ASSERT_EQUAL(sipxAudioGetOutputDeviceInfo(0, 0, &audioDevice), SIPX_RESULT_INVALID_ARGS);
      // checks if there are no more devices
      CPPUNIT_ASSERT_EQUAL(sipxAudioGetOutputDeviceInfo(m_hInst1, numOfDevices, &audioDevice), SIPX_RESULT_INVALID_ARGS);
      CPPUNIT_ASSERT_EQUAL(sipxAudioGetOutputDeviceInfo(m_hInst1, -1, &audioDevice), SIPX_RESULT_INVALID_ARGS);
   }
}

