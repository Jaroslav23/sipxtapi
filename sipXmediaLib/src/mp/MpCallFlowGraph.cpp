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

#include "rtcp/RtcpConfig.h"

// SYSTEM INCLUDES
#include <assert.h>

#if defined(WIN32) && !defined(WINCE) /* [ */
#   include <io.h>
#   include <fcntl.h>
#endif /* WIN32 && !WINCE ] */

#ifdef __pingtel_on_posix__
#include <unistd.h>
#include <fcntl.h>
#endif

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsWriteLock.h"
#include "os/OsEvent.h"
#include "sdp/SdpCodec.h"
#include "os/OsProtectEventMgr.h"
#include "os/OsProtectEvent.h"
#include "os/OsFS.h"
#include "os/OsIntPtrMsg.h"
#include <os/OsTimerNotification.h>
#include <os/OsTimer.h>

#include "mp/MpRtpInputAudioConnection.h"
#include "mp/MpRtpOutputAudioConnection.h"
#include "mp/MpCallFlowGraph.h"
#include "mp/MpMediaTask.h"
#include "mp/MprBridge.h"
#include "mp/MprFromFile.h"
#include "mp/MprFromMic.h"
#include <mp/MpEncoderBase.h>

#ifdef SPEEX_ECHO_CANCELATION
#include "mp/MprSpeexEchoCancel.h"
#endif

#include "mp/MprSpeexPreProcess.h"
#include "mp/MprMixer.h"
#include "mp/MprSplitter.h"
#include "mp/MprToSpkr.h"
#include "mp/MprToneGen.h"

#include "mp/NetInTask.h"
#include "mp/MprRecorder.h"
#include "mp/MpTypes.h"
#include "mp/MpAudioUtils.h"
#include "mp/MpAudioDriverManager.h"
#include "mp/MpAudioStreamInfo.h"
#include "mp/MpAudioDriverBase.h"
#include <mp/MpStopDTMFTimerMsg.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
UtlBoolean MpCallFlowGraph::sbSendInBandDTMF = TRUE;

#ifdef DOING_ECHO_CANCELATION
UtlBoolean MpCallFlowGraph::sbEnableAEC = TRUE;
FLOWGRAPH_AEC_MODE MpCallFlowGraph::ms_AECMode = FLOWGRAPH_AEC_CANCEL;
#else // DOING_ECHO_CANCELATION ][
UtlBoolean MpCallFlowGraph::sbEnableAEC = false;
FLOWGRAPH_AEC_MODE MpCallFlowGraph::ms_AECMode = FLOWGRAPH_AEC_DISABLED;
#endif // DOING_ECHO_CANCELATION ]

#ifdef HAVE_SPEEX // [
UtlBoolean MpCallFlowGraph::sbEnableAGC = TRUE;
UtlBoolean MpCallFlowGraph::sbEnableNoiseReduction = TRUE;
#else // HAVE_SPEEX ][
UtlBoolean MpCallFlowGraph::sbEnableAGC = FALSE;
UtlBoolean MpCallFlowGraph::sbEnableNoiseReduction = FALSE;
#endif // HAVE_SPEEX ]

UtlBoolean MpCallFlowGraph::ms_bEnableInboundInBandDTMF = TRUE;
UtlBoolean MpCallFlowGraph::ms_bEnableInboundRFC2833DTMF = TRUE;

#ifndef O_BINARY
#define O_BINARY 0      // O_BINARY is needed for WIN32 not for VxWorks or Linux
#endif
 
/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MpCallFlowGraph::MpCallFlowGraph(const char* locale,
								         OsMsgQ* pInterfaceNotificationQueue)
: MpFlowGraphBase(MpMisc.m_audioSamplesPerFrame, MpMisc.m_audioSamplesPerSec)
, mConnTableLock(OsBSem::Q_FIFO, OsBSem::FULL)
, m_pInterfaceNotificationQueue(pInterfaceNotificationQueue)
, m_bPlayingInBandDTMF(FALSE)
, m_bPlayingRfc2833DTMF(FALSE)
, m_pStopDTMFToneTimer(NULL)
#ifdef INCLUDE_RTCP /* [ */
, mulEventInterest(LOCAL_SSRC_COLLISION | REMOTE_SSRC_COLLISION)
#endif /* INCLUDE_RTCP ] */
{
   const int samplesPerFrame = MpMisc.m_audioSamplesPerFrame;
   const int samplesPerSec = MpMisc.m_audioSamplesPerSec;
   UtlBoolean    boolRes;
   MpMediaTask* pMediaTask;
   OsStatus     res;
   int          i;

   for (i=0; i<MAX_CONNECTIONS; i++) mpInputConnections[i] = NULL;
   for (i=0; i<MAX_CONNECTIONS; i++) mpOutputConnections[i] = NULL;
   for (i=0; i<MAX_CONNECTIONS; i++) mpBridgePorts[i] = -1;
   for (i=0; i<MAX_RECORDERS; i++) mpRecorders[i] = NULL;

   // create the resources and add them to the flow graph
   mpBridge           = new MprBridge("Bridge", MAX_CONNECTIONS + 1, samplesPerFrame, samplesPerSec,
                                      TRUE, MprBridge::ALG_LINEAR);
   mpFromFile         = new MprFromFile("FromFile",
                                 samplesPerFrame, samplesPerSec);
#ifndef DISABLE_LOCAL_AUDIO // [
   mpFromMic          = new MprFromMic("FromMic", samplesPerFrame, samplesPerSec);
#ifdef HAVE_SPEEX // [
   mpSpeexPreProcess  = new MprSpeexPreprocess("SpeexPreProcess", samplesPerFrame, samplesPerSec);
#ifdef SPEEX_ECHO_CANCELATION
// audio & echo cancelation is enabled
   int echoQueueLatency = MpCallFlowGraph::estimateEchoQueueLatency(samplesPerSec, samplesPerFrame);
   mpEchoCancel       = new MprSpeexEchoCancel("SpeexEchoCancel",
                                 samplesPerFrame, samplesPerSec, SPEEX_DEFAULT_AEC_FILTER_LENGTH, echoQueueLatency);
   // we have speex and use speex echo canceller, interconnect them
   mpSpeexPreProcess->attachEchoCanceller(mpEchoCancel->getSpeexEchoState());
#endif // SPEEX_ECHO_CANCELATION ]
#endif // HAVE_SPEEX ]
   mpTFsMicMixer      = new MprMixer("TFsMicMixer", 2,
                                 samplesPerFrame, samplesPerSec);
   mpTFsBridgeMixer   = new MprMixer("TFsBridgeMixer", 2,
                                 samplesPerFrame, samplesPerSec);
   mpCallrecMixer     = new MprMixer("CallrecMixer", 2,
                                 samplesPerFrame, samplesPerSec);
   mpToneFileSplitter = new MprSplitter("ToneFileSplitter", 2,
                                 samplesPerFrame, samplesPerSec);
   mpMicCallrecSplitter = new MprSplitter("MicCallrecSplitter", 2,
                                 samplesPerFrame, samplesPerSec);
   mpSpeakerCallrecSplitter = new MprSplitter("SpeakerCallrecSplitter", 2,
                                 samplesPerFrame, samplesPerSec);
   mpToSpkr           = new MprToSpkr("ToSpkr",
                                 samplesPerFrame, samplesPerSec, MpMisc.m_pEchoQ);
#endif // DISABLE_LOCAL_AUDIO ]
   mpToneGen          = new MprToneGen("ToneGen",
                                 samplesPerFrame, samplesPerSec, 
                                 locale);

   res = addResource(*mpBridge);            assert(res == OS_SUCCESS);
   res = addResource(*mpFromFile);          assert(res == OS_SUCCESS);
   res = addResource(*mpToneGen);           assert(res == OS_SUCCESS);

#ifndef DISABLE_LOCAL_AUDIO // [
   res = addResource(*mpFromMic);           assert(res == OS_SUCCESS);
#ifdef DOING_ECHO_CANCELATION // [
   res = addResource(*mpEchoCancel);        assert(res == OS_SUCCESS);
#endif // DOING_ECHO_CANCELATION ]
#ifdef HAVE_SPEEX // [
   res = addResource(*mpSpeexPreProcess);   assert(res == OS_SUCCESS);
#endif // HAVE_SPEEX ]
   res = addResource(*mpTFsMicMixer);       assert(res == OS_SUCCESS);
   res = addResource(*mpTFsBridgeMixer);    assert(res == OS_SUCCESS);
   res = addResource(*mpCallrecMixer);      assert(res == OS_SUCCESS);
   res = addResource(*mpToneFileSplitter);  assert(res == OS_SUCCESS);
   res = addResource(*mpMicCallrecSplitter);      assert(res == OS_SUCCESS);
   res = addResource(*mpSpeakerCallrecSplitter);  assert(res == OS_SUCCESS);
   res = addResource(*mpToSpkr);            assert(res == OS_SUCCESS);
#endif // DISABLE_LOCAL_AUDIO ]

   // create the connections between the resources
   //////////////////////////////////////////////////////////////////////////

#ifndef DISABLE_LOCAL_AUDIO
   // connect: 
   // FromMic[0] -> [0](EchoCancel)[0] -> [0](PreProcessor)[0] -> [1]TFsMicMixer[0] -> [0]MicCallrecSplitter[0] -> [0]Bridge
   //
   
   MpResource *pLastResource; // Last resource in the chain
   pLastResource = mpFromMic;

#ifdef DOING_ECHO_CANCELATION /* [ */
   res = addLink(*pLastResource, 0, *mpEchoCancel, 0);
   assert(res == OS_SUCCESS);
   pLastResource = mpEchoCancel;
#endif /* DOING_ECHO_CANCELATION ] */

#ifdef HAVE_SPEEX // [
   res = addLink(*pLastResource, 0, *mpSpeexPreProcess, 0);
   assert(res == OS_SUCCESS);
   pLastResource = mpSpeexPreProcess;
#endif // HAVE_SPEEX ]

   res = addLink(*pLastResource, 0, *mpTFsMicMixer, 1);
   assert(res == OS_SUCCESS);

   res = addLink(*mpTFsMicMixer, 0, *mpMicCallrecSplitter, 0);
   assert(res == OS_SUCCESS);

   res = addLink(*mpMicCallrecSplitter, 0, *mpBridge, 0);
   assert(res == OS_SUCCESS);

   //////////////////////////////////////////////////////////////////////////
   // connect Bridge[0] -> [0]TFsBridgeMixer[0] -> [0]SpeakerCallrecSplitter[0] -> [0]ToSpkr

   res = addLink(*mpBridge, 0, *mpTFsBridgeMixer, 0);
   assert(res == OS_SUCCESS);

   res = addLink(*mpTFsBridgeMixer, 0, *mpSpeakerCallrecSplitter, 0);
   assert(res == OS_SUCCESS);

   res = addLink(*mpSpeakerCallrecSplitter, 0, *mpToSpkr, 0);
   assert(res == OS_SUCCESS);

   //////////////////////////////////////////////////////////////////////////
   // connect SpeakerCallrecSplitter -> CallrecMixer
   //             MicCallrecSplitter --/ 
   res = addLink(*mpMicCallrecSplitter, 1, *mpCallrecMixer, 0);
   assert(res == OS_SUCCESS);

   res = addLink(*mpSpeakerCallrecSplitter, 1, *mpCallrecMixer, 1);
   assert(res == OS_SUCCESS);

   //////////////////////////////////////////////////////////////////////////
   // connect ToneGen -> FromFile -> ToneFileSplitter -> TFsBridgeMixer
   //                                                 -> TFsMicMixer
   
   res = addLink(*mpToneGen, 0, *mpFromFile, 0);
   assert(res == OS_SUCCESS);

   res = addLink(*mpFromFile, 0, *mpToneFileSplitter, 0);
   assert(res == OS_SUCCESS);

   res = addLink(*mpToneFileSplitter, 0, *mpTFsBridgeMixer, 1);
   assert(res == OS_SUCCESS);

   res = addLink(*mpToneFileSplitter, 1, *mpTFsMicMixer, 0);
   assert(res == OS_SUCCESS);

#else  /* DISABLE_LOCAL_AUDIO ] */

   res = addLink(*mpToneGen, 0, *mpFromFile, 0);
   assert(res == OS_SUCCESS);

   res = addLink(*mpFromFile, 0, *mpBridge, 0);
   assert(res == OS_SUCCESS);

#endif /* DISABLE_LOCAL_AUDIO ] */

   //////////////////////////////////////////////////////////////////////////
   // enable the flow graph (and all of the resources within it)
   res = enable();
   assert(res == OS_SUCCESS);

   // disable the tone generator
   boolRes = mpToneGen->disable();      assert(boolRes);
   mToneGenDefocused = FALSE;

   // disable the from file
   boolRes = mpFromFile->disable();     assert(boolRes);

#ifndef DISABLE_LOCAL_AUDIO // [

   // disable mpCallrecMixer and splitters, they are enabled when we want to start recording
   boolRes = mpCallrecMixer->disable();     assert(boolRes);
   boolRes = mpMicCallrecSplitter->disable();         assert(boolRes);
   boolRes = mpSpeakerCallrecSplitter->disable();     assert(boolRes);

   // disable bridge->ToSpkr mixer
   boolRes = mpTFsBridgeMixer->disable();     assert(boolRes);

   // disable the FromMic, EchoCancel, PreProcess and ToSpkr -- we cannot have focus yet...
   boolRes = mpFromMic->disable();                assert(boolRes);
#ifdef DOING_ECHO_CANCELATION // [
   boolRes = mpEchoCancel->disable();             assert(boolRes);
#endif // DOING_ECHO_CANCELATION ]
#ifdef HAVE_SPEEX // [
   boolRes = mpSpeexPreProcess->disable();        assert(boolRes);
#endif // HAVE_SPEEX ]
   boolRes = mpToSpkr->disable();                 assert(boolRes);

   // The next group of settings turns the mixers into 2-to-1 multiplexers.
   // When disabled, mixers default to passing input 0 to output, and with
   // this setup, when enabled, they pass input 1 to output.
   boolRes = mpTFsMicMixer->setWeight(0, 0);      assert(boolRes);
   boolRes = mpTFsMicMixer->setWeight(1, 1);      assert(boolRes);

   // this mixer will work as a normal mixer
   boolRes = mpTFsBridgeMixer->setWeight(1, 0);   assert(boolRes);
   boolRes = mpTFsBridgeMixer->setWeight(1, 1);   assert(boolRes);

   // set up weights for callrec mixer as they are zeroed in constructor
   // input 0 is from mic
   boolRes = mpCallrecMixer->setWeight(1, 0);   assert(boolRes);
   // input 1 is speaker
   boolRes = mpCallrecMixer->setWeight(1, 1);   assert(boolRes);
#endif // DISABLE_LOCAL_AUDIO ]

#ifdef INCLUDE_RTCP /* [ */
   // All the Media Resource seemed to have been started successfully.
   // Let's now create an RTCP Session so that we may be prepared to
   // report on the RTP connections that shall eventually be associated
   // with this flow graph

   // Let's get the  RTCP Control interface
   IRTCPControl *piRTCPControl = CRTCManager::getRTCPControl();
   assert(piRTCPControl);

   // Create an RTCP Session for this Flow Graph.  Pass the SSRC ID to be
   // used to identify our audio source uniquely within this RTP/RTCP Session.
   mpiRTCPSession = piRTCPControl->CreateSession(rand_timer32());

   // Subscribe for Events associated with this Session
   piRTCPControl->Advise((IRTCPNotify *)this);

   // Release Reference to RTCP Control Interface
   piRTCPControl->Release();
#endif /* INCLUDE_RTCP ] */

#ifndef DISABLE_LOCAL_AUDIO
   // Call Recording..  Always record calls.
   // create Call recorder and connect it to mpCallrecMixer
   mpRecorders[RECORDER_CALL] =
      new MprRecorder("RecordCall", samplesPerFrame, samplesPerSec);
   res = addResource(*(mpRecorders[RECORDER_CALL]));
   assert(res == OS_SUCCESS);
   res = addLink(*mpCallrecMixer, 0, *(mpRecorders[RECORDER_CALL]), 0);
   assert(res == OS_SUCCESS);
#else
   mpRecorders[RECORDER_CALL] =
      new MprRecorder("RecordCall", samplesPerFrame, samplesPerSec);
   res = addResource(*(mpRecorders[RECORDER_CALL]));

   res = addLink(*mpBridge, 0, *(mpRecorders[RECORDER_CALL]), 0);
   assert(res == OS_SUCCESS);
#endif
///////////////////////////////////////////////////////////////////////////////////

   // ask the media processing task to manage the new flow graph
   pMediaTask = MpMediaTask::getMediaTask();
   res = pMediaTask->manageFlowGraph(*this);
   assert(res == OS_SUCCESS);

   // start the flow graph
   res = pMediaTask->startFlowGraph(*this);
   assert(res == OS_SUCCESS);

   Zprintf("mpBridge=0x%X, " "mpConnection=0x%X, " "mpFromFile=0x%X\n" 
       "mpFromMic=0x%X, " "mpTFsMicMixer=0x%X" "mpTFsBridgeMixer=0x%X\n",
      (int) mpBridge, (int) mpConnections[0], (int) mpFromFile, 
      (int) mpFromMic, (int) mpTFsMicMixer, (int) mpTFsBridgeMixer);

#ifdef DOING_ECHO_CANCELATION /* [ */
   Zprintf("mpTFsMicMixer=0x%X, " "mpTFsBridgeMixer=0x%X\n"
      "mpToneFileSplitter=0x%X, " "mpToSpkr=0x%X, " "mpToneGen=0x%X\n"
      "mpEchoCancel=0x%X\n",
      (int) mpTFsMicMixer, (int) mpTFsBridgeMixer, (int) mpToneFileSplitter,
      (int) mpToSpkr, (int) mpToneGen, (int) mpEchoCancel);
#else /* DOING_ECHO_CANCELATION ] [ */
   Zprintf("mpTFsMicMixer=0x%X, " "mpTFsBridgeMixer=0x%X\n"
      "mpToneFileSplitter=0x%X, " "mpToSpkr=0x%X, " "mpToneGen=0x%X\n",
      (int) mpTFsMicMixer, (int) mpTFsBridgeMixer, (int) mpToneFileSplitter,
      (int) mpToSpkr, (int) mpToneGen, 0);
#endif /* DOING_ECHO_CANCELATION ] */
}

// Destructor
MpCallFlowGraph::~MpCallFlowGraph()
{
   MpMediaTask* pMediaTask;
   OsStatus     res;
   int          i;

#ifdef INCLUDE_RTCP /* [ */
   // Let's terminate the RTCP Session in preparation for call teardown

   // Let's get the  RTCP Control interface
   IRTCPControl *piRTCPControl = CRTCManager::getRTCPControl();
   assert(piRTCPControl);

   // Unsubscribe for Events associated with this Session
   piRTCPControl->Unadvise((IRTCPNotify *)this);

   // Terminate the RTCP Session
   piRTCPControl->TerminateSession(mpiRTCPSession);

   // Release Reference to RTCP Control Interface
   piRTCPControl->Release();
#endif /* INCLUDE_RTCP ] */

   // unmanage the flow graph
   pMediaTask = MpMediaTask::getMediaTask();
   res = pMediaTask->unmanageFlowGraph(*this);
   assert(res == OS_SUCCESS);

   assert(!pMediaTask->isManagedFlowGraph(this));

   // $$$ I believe that we should just be able to delete the flow graph
   // $$$ at this point, but for now let's get rid of all the connections
   // $$$ and resources first.

   // remove the links between the resources
   res = removeLink(*mpBridge, 0);           assert(res == OS_SUCCESS);

#ifndef DISABLE_LOCAL_AUDIO
   res = removeLink(*mpFromMic, 0);          assert(res == OS_SUCCESS);
#ifdef DOING_ECHO_CANCELATION // [
   res = removeLink(*mpEchoCancel, 0);       assert(res == OS_SUCCESS);
#endif // DOING_ECHO_CANCELATION ]
#ifdef HAVE_SPEEX // [
   res = removeLink(*mpSpeexPreProcess, 0);  assert(res == OS_SUCCESS);
#endif // HAVE_SPEEX ]
   res = removeLink(*mpTFsMicMixer, 0);      assert(res == OS_SUCCESS);
   res = removeLink(*mpTFsBridgeMixer, 0);   assert(res == OS_SUCCESS);
   res = removeLink(*mpToneGen, 0);          assert(res == OS_SUCCESS);
   res = removeLink(*mpFromFile, 0);         assert(res == OS_SUCCESS);
   res = removeLink(*mpToneFileSplitter, 0); assert(res == OS_SUCCESS);
   res = removeLink(*mpToneFileSplitter, 1); assert(res == OS_SUCCESS);

   // remove links of call recording
   res = removeLink(*mpCallrecMixer, 0); assert(res == OS_SUCCESS);
   res = removeLink(*mpMicCallrecSplitter, 0); assert(res == OS_SUCCESS);
   res = removeLink(*mpMicCallrecSplitter, 1); assert(res == OS_SUCCESS);
   res = removeLink(*mpSpeakerCallrecSplitter, 0); assert(res == OS_SUCCESS);
   res = removeLink(*mpSpeakerCallrecSplitter, 1); assert(res == OS_SUCCESS);

   // now remove (and destroy) the resources
   res = removeResource(*mpFromMic);
   assert(res == OS_SUCCESS);
   delete mpFromMic;
   mpFromMic = NULL;

#ifdef HAVE_SPEEX // [
   res = removeResource(*mpSpeexPreProcess);
   assert(res == OS_SUCCESS);
   delete mpSpeexPreProcess;
#endif // HAVE_SPEEX ]
#ifdef DOING_ECHO_CANCELATION // [
   res = removeResource(*mpEchoCancel);
   assert(res == OS_SUCCESS);
   delete mpEchoCancel;
#endif // DOING_ECHO_CANCELATION ]

   res = removeResource(*mpTFsMicMixer);
   assert(res == OS_SUCCESS);
   delete mpTFsMicMixer;

   res = removeResource(*mpTFsBridgeMixer);
   assert(res == OS_SUCCESS);
   delete mpTFsBridgeMixer;

   res = removeResource(*mpToneFileSplitter);
   assert(res == OS_SUCCESS);
   delete mpToneFileSplitter;

   res = removeResource(*mpToSpkr);
   assert(res == OS_SUCCESS);
   delete mpToSpkr;

   // kill call recording resources
   res = removeResource(*mpMicCallrecSplitter);
   assert(res == OS_SUCCESS);
   delete mpMicCallrecSplitter;

   res = removeResource(*mpSpeakerCallrecSplitter);
   assert(res == OS_SUCCESS);
   delete mpSpeakerCallrecSplitter;

   res = removeResource(*mpCallrecMixer);
   assert(res == OS_SUCCESS);
   delete mpCallrecMixer;

#else // DISABLE_LOCAL_AUDIO ]

   res = removeLink(*mpToneGen, 0);          assert(res == OS_SUCCESS);
   res = removeLink(*mpFromFile, 0);         assert(res == OS_SUCCESS);

#endif // DISABLE_LOCAL_AUDIO ]

  for ( i = 0; i < MAX_RECORDERS; i++)
  {
      if (NULL != mpRecorders[i])
      {
         res = removeResource(*mpRecorders[i]);
         assert(res == OS_SUCCESS);
         delete mpRecorders[i];
         mpRecorders[i] = NULL;
      }
   }

   res = removeResource(*mpToneGen);
   assert(res == OS_SUCCESS);
   delete mpToneGen;

   res = removeResource(*mpFromFile);
   assert(res == OS_SUCCESS);
   delete mpFromFile;

   res = removeResource(*mpBridge);
   assert(res == OS_SUCCESS);
   delete mpBridge;

   delete m_pStopDTMFToneTimer;
   m_pStopDTMFToneTimer = NULL;
}

/* ============================ MANIPULATORS ============================== */

// Notification that this flow graph has just been granted the focus.
// Enable our microphone and speaker resources
OsStatus MpCallFlowGraph::gainFocus(void)
{
   UtlBoolean    boolRes;
#ifndef DISABLE_LOCAL_AUDIO // ]
   // also send interface notification if local audio is not disabled
   sendInterfaceNotification(MP_NOTIFICATION_AUDIO, MP_NOTIFICATION_FOCUS_GAINED);

   // enable the FromMic, (EchoCancel), (PreProcessor), and ToSpkr -- we have focus
   boolRes = mpFromMic->enable();
   assert(boolRes);

#ifdef DOING_ECHO_CANCELATION // [
   if (sbEnableAEC)
   {
      boolRes = mpEchoCancel->enable();
      assert(boolRes);
   }
#endif // DOING_ECHO_CANCELATION ]

   UtlBoolean bVADEnabled = MpEncoderBase::isVADEnabled();
#ifdef HAVE_SPEEX // [
   if (sbEnableAGC || sbEnableNoiseReduction || sbEnableAEC || bVADEnabled)
   {
     boolRes = mpSpeexPreProcess->enable();
     assert(boolRes);
     mpSpeexPreProcess->setAGC(sbEnableAGC);
     mpSpeexPreProcess->setNoiseReduction(sbEnableNoiseReduction);
     mpSpeexPreProcess->setVAD(bVADEnabled);
   }
#endif // HAVE_SPEEX ]

   boolRes = mpToSpkr->enable();        assert(boolRes);

#ifdef DOING_ECHO_CANCELATION // [
   if (!mpTFsMicMixer->isEnabled())  
   {
      boolRes = mpEchoCancel->disable();
      assert(boolRes);
   }
#endif // DOING_ECHO_CANCELATION ]

#endif // DISABLE_LOCAL_AUDIO

   // Re-enable the tone as it is now being heard
   if(mToneGenDefocused)
   {
      mpToneGen->enable();
      mToneGenDefocused = FALSE;
   }

   Nprintf("MpBFG::gainFocus(0x%X)\n", (int) this, 0,0,0,0,0);
   return OS_SUCCESS;
}

// Notification that this flow graph has just lost the focus.
// Disable our microphone and speaker resources
OsStatus MpCallFlowGraph::loseFocus(void)
{
   UtlBoolean    boolRes;

#ifndef DISABLE_LOCAL_AUDIO // [
   // also send interface notification if local audio is not disabled
   sendInterfaceNotification(MP_NOTIFICATION_AUDIO, MP_NOTIFICATION_FOCUS_LOST);
   // disable the FromMic, (EchoCancel), (PreProcessor) and ToSpkr --
   // we no longer have the focus.

   boolRes = mpFromMic->disable();       assert(boolRes);
#ifdef DOING_ECHO_CANCELATION // [
   boolRes = mpEchoCancel->disable();    assert(boolRes);
#endif // DOING_ECHO_CANCELATION ]
#ifdef HAVE_SPEEX // [
   boolRes = mpSpeexPreProcess->disable(); assert(boolRes);
#endif // HAVE_SPEEX ]
   boolRes = mpToSpkr->disable();        assert(boolRes);

#endif //DISABLE_LOCAL_AUDIO ]

   // If the tone gen is not needed while we are out of focus disable it
   // as it is using resources while it is not being heard.
   if(mpToneGen->isEnabled() )
	  // && 
      // mpTFsBridgeMixer->isEnabled()) // Local tone too
      // Should also disable when remote tone and no connections
      // || (!mp???Mixer->isEnabled() && noConnections)) 
   {
      // osPrintf("Defocusing tone generator\n");
      mpToneGen->disable();
      mToneGenDefocused = TRUE;
   }

   Nprintf("MpBFG::loseFocus(0x%X)\n", (int) this, 0,0,0,0,0);
   return OS_SUCCESS;
}

// Start playing the indicated tone.
void MpCallFlowGraph::startTone(int toneId, int toneOptions, int duration)
{
   UtlBoolean boolRes;
   OsStatus  res;

   if ((sbSendInBandDTMF && (toneOptions & TONE_TO_NET)) ||
      (toneOptions & TONE_TO_SPKR))
   {
      // enable inband DTMF generator
      res = mpToneGen->startTone(toneId);
      assert(res == OS_SUCCESS);
      m_bPlayingInBandDTMF = TRUE;
   }

#ifndef DISABLE_LOCAL_AUDIO
   // Also play locally if requested
   if (toneOptions & TONE_TO_SPKR)
   {
      boolRes = mpTFsBridgeMixer->enable();
      assert(boolRes);
   }
#endif

   if (!sbSendInBandDTMF && (toneOptions & TONE_TO_NET))
   {
      m_bPlayingRfc2833DTMF = TRUE;
      // Notify outbound leg of all connections that we are playing a tone
      for (int i = 0; i < MAX_CONNECTIONS; i++)
      {
         if (mpOutputConnections[i]) 
         {
            // starts RFC 2833 tone
            mpOutputConnections[i]->startTone(toneId);
         }
      }
   }

#ifndef DISABLE_LOCAL_AUDIO
   if (!sbSendInBandDTMF)
   {
      // we don't want inband in RTP, pass only mic
      boolRes = mpTFsMicMixer->enable();
      assert(boolRes);
   }
   else
   {
      // we want inband in RTP, don't pass from mic and pass from tonegen
      boolRes = mpTFsMicMixer->disable();
      assert(boolRes);
   }

#ifdef DOING_ECHO_CANCELATION // [
   boolRes = mpEchoCancel->disable();  assert(boolRes);
#endif // DOING_ECHO_CANCELATION ]
#endif // DISABLE_LOCAL_AUDIO ]

   if (duration > 0)
   {
      MpStopDTMFTimerMsg msg;
      OsTimerNotification* pNotification = new OsTimerNotification(*getMsgQ(), msg);
      delete m_pStopDTMFToneTimer; // get rid of any previous timer
      m_pStopDTMFToneTimer = new OsTimer(pNotification);
      OsTime timerTime(duration); // time in milliseconds
      m_pStopDTMFToneTimer->oneshotAfter(timerTime); // start timer
   }
}

// Stop playing the tone (applies to all tone destinations).
void MpCallFlowGraph::stopTone()
{
   OsStatus  res;
   int i;
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_STOP_TONE, NULL,
      NULL, NULL, 0, 0);

   if (m_bPlayingInBandDTMF)
   {
      res = mpToneGen->stopTone();
      assert(res == OS_SUCCESS);
      m_bPlayingInBandDTMF = FALSE;
   }
   res = postMessage(msg);

   if (m_bPlayingRfc2833DTMF)
   {
      m_bPlayingRfc2833DTMF = FALSE;
      // stop sending RFC 2833 DTMF
      for (i = 0; i < MAX_CONNECTIONS; i++)
      {
         if (mpOutputConnections[i])
            mpOutputConnections[i]->stopTone();
      }
   }
}

int MpCallFlowGraph::closeRecorders(void)
{
   int ret = 0;
   int i;

   if (NULL == this) {
      MpMediaTask* pMT = MpMediaTask::getMediaTask();
      MpCallFlowGraph* pIF = (MpCallFlowGraph*) pMT->getFocus();
      if (NULL != pIF) return pIF->closeRecorders();
      return 0;
   }
   for (i=0; i<MAX_RECORDERS; i++) {
      if (mpRecorders[i]) {
         mpRecorders[i]->closeRecorder();
         ret++;
      }
   }
   return ret;
}

OsStatus MpCallFlowGraph::record(int timeMS,
                                 int silenceLength,
                                 const char* callRecordFile /*= NULL*/,
                                 int toneOptions /*= 0*/,
                                 int repeat /*= 0*/,
                                 OsNotification* completion /*= NULL*/,
                                 MprRecorder::RecordFileFormat format)
{
   // set up call recorder
   if (callRecordFile && *callRecordFile)
   {
      setupRecorder(RECORDER_CALL, callRecordFile,
                    timeMS, silenceLength, completion, format);
   }

   return startRecording();
}

OsStatus MpCallFlowGraph::startRecording()
{
   OsStatus  res = OS_SUCCESS;
   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_START_RECORD);
   res = postMessage(msg);
   return(res);
}

// Setup recording on one recorder
UtlBoolean MpCallFlowGraph::setupRecorder(RecorderChoice which,
                                          const char* audioFileName,
                                          int timeMS, 
                                          int silenceLength,
                                          OsNotification* event,
                                          MprRecorder::RecordFileFormat format)
{
   int file = -1;
   OsStatus  res = OS_INVALID_ARGUMENT;

   if (NULL == mpRecorders[which])
   {
      return FALSE;
   }

   if (NULL != audioFileName)
   {
        file = open(audioFileName, O_BINARY | O_CREAT | O_RDWR, 0600);
   }

   if (-1 < file)
   {
      if (format == MprRecorder::WAV_PCM_16)
      {
          writeWAVHeader(file);
      }

      res = mpRecorders[which]->setup(file, format, timeMS, silenceLength, (OsEvent*)event);
   }
   else
   {
      OsSysLog::add(FAC_AUDIO, PRI_ERR,
         "setupRecorder failed to open file %s, error code is %i",
         audioFileName, errno);
   }
   return (file != -1 && res == OS_SUCCESS);
}

// Start playing the indicated audio file.
OsStatus MpCallFlowGraph::playFile(const char* audioFileName,
                                   UtlBoolean repeat,
                                   int toneOptions,
                                   void* pCookie)
{
   OsStatus  res;

   // Turn on notifications from the fromFile resource, as they'll be
   // needed when the file stops playing, so CallFlowGraph can do 
   // it's cleanup.  (The old method was to have the resource directly
   // call stuff in the CallFlowGraph -- a big nono in terms of separation)
   MpResource::setAllNotificationsEnabled(TRUE, mpFromFile->getName(), *getMsgQ());

   res = mpFromFile->playFile(audioFileName, repeat, pCookie);

   if (res == OS_SUCCESS)
   {
      MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_START_PLAY, NULL,
                      NULL, NULL, toneOptions, 0);
      res = postMessage(msg);
   }
   return(res);
}

// Start playing the indicated audio buffer.
OsStatus MpCallFlowGraph::playBuffer(void* audioBuf, 
                                     size_t bufSize,
                                     int type, 
                                     UtlBoolean repeat,
                                     int toneOptions,
                                     void* pCookie)
{
   OsStatus  res;

   // Turn on notifications from the fromFile resource, as they'll be
   // needed when the buffer stops playing, so CallFlowGraph can do 
   // it's cleanup.  (The old method was to have the resource directly
   // call stuff in the CallFlowGraph -- a big nono in terms of separation)
   MpResource::setAllNotificationsEnabled(TRUE, mpFromFile->getName(), *getMsgQ());

   res = mpFromFile->playBuffer(audioBuf, bufSize, type, repeat, pCookie);

   if (res == OS_SUCCESS)
   {
      MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_START_PLAY, NULL,
                      NULL, NULL, toneOptions, 0);
      res = postMessage(msg);
   }
   return(res);
}

// Stop playing the audio file.
void MpCallFlowGraph::stopFile()
{
   OsStatus  res;

   // mpFromFile->disable();

   res = mpFromFile->stopPlayback();
   assert(res == OS_SUCCESS);

   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_STOP_PLAY, NULL,
      NULL, NULL, 0, 0);
   res = postMessage(msg);

   // Shut off the tone generator/play sound input to the Mixer resource
   // boolRes = mpTFsMicMixer->enable();      assert(boolRes);
   // boolRes = mpTFsBridgeMixer->enable();   assert(boolRes);
}

OsStatus MpCallFlowGraph::pausePlayback()
{
   OsStatus returnCode = OS_FAILED;

   if (mpFromFile)
   {
      returnCode = mpFromFile->pausePlayback();
      assert(returnCode == OS_SUCCESS);
   }
   
   return returnCode;
}

OsStatus MpCallFlowGraph::resumePlayback()
{
   OsStatus returnCode = OS_FAILED;

   if (mpFromFile)
   {
      returnCode = mpFromFile->resumePlayback();
      assert(returnCode == OS_SUCCESS);
   }

   return returnCode;
}

MpConnectionID MpCallFlowGraph::createConnection(OsMsgQ* pConnectionNotificationQueue)
{
   int            i;
   MpConnectionID found = -1;
   int            bridgePort;
   MpRtpInputAudioConnection*  pInputConnection;
   MpRtpOutputAudioConnection*  pOutputConnection;

   mConnTableLock.acquire();
   for (i=1; i<MAX_CONNECTIONS; i++) 
   {
      if (NULL == mpInputConnections[i] &&
          NULL == mpOutputConnections[i]) 
      {
         mpInputConnections[i] = (MpRtpInputAudioConnection*) -1;
         mpOutputConnections[i] = (MpRtpOutputAudioConnection*) -1;
         found = i;
         i = MAX_CONNECTIONS;
      }
   }
   
   if (found < 0) {
      mConnTableLock.release();
      return -1;
   }
   UtlString inConnectionName("InputConnection-");
   UtlString outConnectionName("OutputConnection-");
   char numBuf[20];
   SNPRINTF(numBuf, sizeof(numBuf), "%d", found);
   inConnectionName.append(numBuf);
   outConnectionName.append(numBuf);

   mpInputConnections[found] = 
       new MpRtpInputAudioConnection(inConnectionName,
                                     found,
                                     pConnectionNotificationQueue,
                                     ms_bEnableInboundInBandDTMF,
                                     ms_bEnableInboundRFC2833DTMF,
                                     getSamplesPerFrame(), 
                                     getSamplesPerSec());
   mpOutputConnections[found] = 
       new MpRtpOutputAudioConnection(outConnectionName,
                                      found, 
                                      pConnectionNotificationQueue,
                                      getSamplesPerFrame(), 
                                      getSamplesPerSec());

   pInputConnection = mpInputConnections[found];
   pOutputConnection = mpOutputConnections[found];

   bridgePort = mpBridge->reserveFirstUnconnectedInput();

   if (bridgePort < 0) 
   {
      delete pInputConnection;
      delete pOutputConnection;
      mpInputConnections[found] = NULL;
      mpOutputConnections[found] = NULL;
      mConnTableLock.release();
      return -1;
   }

   mpBridgePorts[found] = bridgePort;
   mConnTableLock.release();

   pInputConnection->enable();
   pOutputConnection->enable();

   OsStatus stat = addResource(*pInputConnection);
   assert(OS_SUCCESS == stat);
   stat = addResource(*pOutputConnection);
   assert(OS_SUCCESS == stat);

   stat = addLink(*mpBridge, bridgePort, *pOutputConnection, 0);
   assert(OS_SUCCESS == stat);
   stat = addLink(*pInputConnection, 0, *mpBridge, bridgePort);
   assert(OS_SUCCESS == stat);

   return found;
}

OsStatus MpCallFlowGraph::deleteConnection(MpConnectionID connID)
{
   OsWriteLock    lock(mRWMutex);

   UtlBoolean      handled;
   OsStatus       ret;

//   osPrintf("deleteConnection(%d)\n", connID);
   assert((0 < connID) && (connID < MAX_CONNECTIONS));

   if ((NULL == mpInputConnections[connID]) || 
      (((MpRtpInputAudioConnection*) -1) == mpInputConnections[connID]))
         return OS_INVALID_ARGUMENT;

   if ((NULL == mpOutputConnections[connID]) || 
      (((MpRtpOutputAudioConnection*) -1) == mpOutputConnections[connID]))
         return OS_INVALID_ARGUMENT;

   MpFlowGraphMsg msg(MpFlowGraphMsg::FLOWGRAPH_REMOVE_CONNECTION, NULL,
                      NULL, NULL, connID);

   if (isStarted())
   {
      ret = postMessage(msg);
      if (OS_SUCCESS != ret)
	  {
         	  OsSysLog::add(FAC_CP, PRI_DEBUG, "MpCallFlowGraph::deleteConnection COULD NOT POST MESSAGE !" );
      }
      return ret;
   }
   else
   {
      handled = handleMessage(msg);
      if (handled)
          return OS_SUCCESS;
      else
          return OS_UNSPECIFIED;
   }
}

OsStatus MpCallFlowGraph::muteInput(MpConnectionID connID)
{
	if(connID >= 0 && connID < MAX_CONNECTIONS && mpBridgePorts[connID] != -1 && mpBridge != NULL)
	{
		MpBridgeGain gains[MAX_CONNECTIONS];
		for(int i = 0; i < MAX_CONNECTIONS; i++)
		{
			gains[i] = MP_BRIDGE_GAIN_MUTED;
		}

		return mpBridge->setMixWeightsForInput(mpBridgePorts[connID], MAX_CONNECTIONS, gains);
	}

	return OS_FAILED;
}

OsStatus MpCallFlowGraph::unmuteInput(MpConnectionID connID)
{
	if(connID >= 0 && connID < MAX_CONNECTIONS && mpBridgePorts[connID] != -1 && mpBridge != NULL)
	{
		MpBridgeGain gains[MAX_CONNECTIONS];
		for(int i = 0; i < MAX_CONNECTIONS; i++)
		{
			gains[i] = MP_BRIDGE_GAIN_PASSTHROUGH;
		}
		gains[mpBridgePorts[connID]] = MP_BRIDGE_GAIN_MUTED;

		return mpBridge->setMixWeightsForInput(mpBridgePorts[connID], MAX_CONNECTIONS, gains);
	}

	return OS_FAILED;
}


// Start sending RTP and RTCP packets.
void MpCallFlowGraph::startSendRtp(OsSocket& rRtpSocket,
                                    OsSocket& rRtcpSocket,
                                    MpConnectionID connID,
                                    SdpCodec* pPrimaryCodec,
                                    SdpCodec* pDtmfCodec)
{
   if(mpOutputConnections[connID])
   {
       MpRtpOutputAudioConnection::startSendRtp(*(getMsgQ()),
                                                mpOutputConnections[connID]->getName(),
                                                rRtpSocket, 
                                                rRtcpSocket,
                                                pPrimaryCodec, 
                                                pDtmfCodec);
   }
}

// Stop sending RTP and RTCP packets.
void MpCallFlowGraph::stopSendRtp(MpConnectionID connID)
{
   // postPone(40); // testing...
   if(mpOutputConnections[connID])
   {
       if(mpOutputConnections[connID])
       {
           MpRtpOutputAudioConnection::stopSendRtp(*(getMsgQ()),
                                                mpOutputConnections[connID]->getName());
       }
   }
}

void MpCallFlowGraph::startReceiveRtp(const SdpCodecList& sdpCodecList,
                                       OsSocket& rRtpSocket,
                                       OsSocket& rRtcpSocket,
                                       MpConnectionID connID)
{
    if(mpInputConnections[connID])
    {
        MpRtpInputAudioConnection::startReceiveRtp(*(getMsgQ()),
                                mpInputConnections[connID]->getName(),
                                sdpCodecList, 
                                rRtpSocket, 
                                rRtcpSocket);
    }
}

// Stop receiving RTP and RTCP packets.
void MpCallFlowGraph::stopReceiveRtp(MpConnectionID connID)
{
    if(mpInputConnections[connID])
    {
        MpRtpInputAudioConnection::stopReceiveRtp(*(getMsgQ()),
                                mpInputConnections[connID]->getName());
    }
}

void MpCallFlowGraph::setInterfaceNotificationQueue(OsMsgQ* pInterfaceNotificationQueue)
{
	if (!m_pInterfaceNotificationQueue)
	{
		m_pInterfaceNotificationQueue = pInterfaceNotificationQueue;
	}
}

void MpCallFlowGraph::sendInterfaceNotification(MpNotificationMsgMedia msgMedia,
                                                MpNotificationMsgType msgSubType,
                                                intptr_t msgData1,
                                                intptr_t msgData2)
{
   if (m_pInterfaceNotificationQueue)
   {
      // create message and send it to interface notification queue
      OsIntPtrMsg interfaceMsg(OsMsg::MP_INTERFACE_NOTF_MSG, (unsigned char)msgMedia,
                              (intptr_t)msgSubType, msgData1, msgData2);
      m_pInterfaceNotificationQueue->send(interfaceMsg, OsTime::NO_WAIT_TIME);
   }
}

OsStatus MpCallFlowGraph::setConnectionIdleTimeout(const int idleTimeout)
{
   MpRtpInputAudioConnection::setConnectionIdleTimeout(idleTimeout);
   return OS_SUCCESS;
}

// Enables/Disable the transmission of inband DTMF audio
UtlBoolean MpCallFlowGraph::enableSendInbandDTMF(UtlBoolean bEnable)
{
   UtlBoolean bSave = sbSendInBandDTMF;
   sbSendInBandDTMF = bEnable;
   return bSave;
}

UtlBoolean MpCallFlowGraph::isSendInbandDTMFEnabled()
{
   return sbSendInBandDTMF;
}

// Get Echo Cancelation Mode.
UtlBoolean MpCallFlowGraph::getAECMode(FLOWGRAPH_AEC_MODE& mode)
{
   UtlBoolean bReturn = true;
   mode = ms_AECMode;
   return bReturn;
}

// Set Echo Cancelation Mode.
UtlBoolean MpCallFlowGraph::setAECMode(FLOWGRAPH_AEC_MODE mode)
{
   // Selecting an unsupporetd mode will return false, but no error.
   // Not sure if this is the correct behavior.
   UtlBoolean bReturn = false;
   switch (mode) {
   case FLOWGRAPH_AEC_CANCEL:
   case FLOWGRAPH_AEC_CANCEL_AUTO:
#ifdef DOING_ECHO_CANCELATION // [
      sbEnableAEC = true;
      ms_AECMode = mode;
      bReturn = true;
#endif // DOING_ECHO_CANCELATION ]
      break;

   case FLOWGRAPH_AEC_SUPPRESS:
#ifdef SIPX_ECHO_CANCELATION // [
      sbEnableAEC = true;
      bReturn = true;
      ms_AECMode = mode;
#endif // SIPX_ECHO_CANCELATION ]
      break;

   case FLOWGRAPH_AEC_DISABLED:
      ms_AECMode = mode;
      sbEnableAEC = false;
      bReturn = true;
      break;
   }

   // Should post a message to make changes effective, but we are static and
   // don't have an instance.
   return bReturn;
}

UtlBoolean MpCallFlowGraph::getAGC(UtlBoolean& bEnabled)
{
   UtlBoolean bReturn = TRUE;
   bEnabled = sbEnableAGC;
   return bReturn;
}

UtlBoolean MpCallFlowGraph::setAGC(UtlBoolean bEnable)
{
   UtlBoolean bReturn = FALSE;
#ifdef HAVE_SPEEX // [
   sbEnableAGC = bEnable;
   bReturn = true;
#endif // HAVE_SPEEX ]
   return bReturn;
}

UtlBoolean MpCallFlowGraph::getAudioNoiseReduction(UtlBoolean& bEnabled)
{
   UtlBoolean bReturn = TRUE;
   bEnabled = sbEnableNoiseReduction;
   return bReturn;
}

UtlBoolean MpCallFlowGraph::setAudioNoiseReduction(UtlBoolean bEnable)
{
   UtlBoolean bReturn = FALSE;
#ifdef HAVE_SPEEX // [
   sbEnableNoiseReduction = bEnable;
   bReturn = true;
#endif // HAVE_SPEEX ]
   return bReturn;
}

UtlBoolean MpCallFlowGraph::isVADEnabled()
{
   return MpEncoderBase::isVADEnabled();
}

void MpCallFlowGraph::enableVAD(UtlBoolean bEnable)
{
   MpEncoderBase::enableVAD(bEnable);
}

UtlBoolean MpCallFlowGraph::enableInboundInBandDTMF(UtlBoolean enable)
{
   ms_bEnableInboundInBandDTMF = enable;
   return TRUE;
}

UtlBoolean MpCallFlowGraph::enableInboundRFC2833DTMF(UtlBoolean enable)
{
   ms_bEnableInboundRFC2833DTMF = enable;
   return TRUE;
}

#ifdef INCLUDE_RTCP /* [ */

/* ======================== CALLBACK METHODS ============================= */

/**
 *
 * Method Name:  LocalSSRCCollision()
 *
 *
 * Inputs:      IRTCPConnection *piRTCPConnection - Interface to associated
 *                                                   RTCP Connection
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
 *              .
 *               
 * Usage Notes: 
 *
 */
void MpCallFlowGraph::LocalSSRCCollision(IRTCPConnection  *piRTCPConnection, 
                                         IRTCPSession     *piRTCPSession)
{

//  Ignore those events that are for a session other than ours
    if(mpiRTCPSession != piRTCPSession)
    {
//      Release Interface References
        piRTCPConnection->Release();
        piRTCPSession->Release();
        return;
    }

// We have a collision with our local SSRC.  We will remedy this by
// generating a new SSRC
    mpiRTCPSession->ReassignSSRC(rand_timer32(),
                         (unsigned char *)"LOCAL SSRC COLLISION");

// We must inform all connections associated with this session to change their
// SSRC
    mConnTableLock.acquire();
    for (int iConnection = 1; iConnection < MAX_CONNECTIONS; iConnection++) 
    {
      if (mpOutputConnections[iConnection]->getRTCPConnection()) 
      {
//       Set the new SSRC
         mpOutputConnections[iConnection]->
                          reassignSSRC((int)mpiRTCPSession->GetSSRC());
         break;
      }
   }
   mConnTableLock.release();

// Release Interface References
   piRTCPConnection->Release();
   piRTCPSession->Release();

   return;
}


/**
 *
 * Method Name:  RemoteSSRCCollision()
 *
 *
 * Inputs:      IRTCPConnection *piRTCPConnection - Interface to associated
 *                                                   RTCP Connection
 *              IRTCPSession    *piRTCPSession    - Interface to associated
 *                                                   RTCP Session
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
void MpCallFlowGraph::RemoteSSRCCollision(IRTCPConnection  *piRTCPConnection, 
                                          IRTCPSession     *piRTCPSession)
{

//  Ignore those events that are for a session other than ours
    if(mpiRTCPSession != piRTCPSession)
    {
//      Release Interface References
        piRTCPConnection->Release();
        piRTCPSession->Release();
        return;
    }

// According to standards, we are supposed to ignore remote sites that
// have colliding SSRC IDS.
    mConnTableLock.acquire();
    for (int iConnection = 1; iConnection < MAX_CONNECTIONS; iConnection++) 
    {
      if (mpInputConnections[iConnection] &&
          mpInputConnections[iConnection]->getRTCPConnection() == piRTCPConnection) 
      {
// We are supposed to ignore the media of the latter of two terminals
// whose SSRC collides
         MpRtpInputAudioConnection::stopReceiveRtp(*(getMsgQ()),
                                mpInputConnections[iConnection]->getName());
         break;
      }
   }
   mConnTableLock.release();

// Release Interface References
   piRTCPConnection->Release();
   piRTCPSession->Release();


}
#endif /* INCLUDE_RTCP ] */

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */

UtlBoolean MpCallFlowGraph::isInboundInBandDTMFEnabled()
{
   return ms_bEnableInboundInBandDTMF;
}

UtlBoolean MpCallFlowGraph::isInboundRFC2833DTMFEnabled()
{
   return ms_bEnableInboundRFC2833DTMF;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
int MpCallFlowGraph::estimateEchoQueueLatency(int samplesPerSec, int samplesPerFrame)
{
#define MIN_DRIVER_LATENCY 4

#ifndef DISABLE_LOCAL_AUDIO
   MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
   int echoQueueLatency = 0;

   if (pAudioManager)
   {
      MpAudioStreamId inputStreamId = pAudioManager->getInputAudioStream();
      MpAudioStreamId outputStreamId = pAudioManager->getOutputAudioStream();
      MpAudioDriverBase* pAudioDriver = pAudioManager->getAudioDriver();

      if (pAudioDriver)
      {
         if (inputStreamId)
         {
            MpAudioStreamInfo streamInfo;
            OsStatus res = pAudioDriver->getStreamInfo(inputStreamId, streamInfo);
            if (res == OS_SUCCESS)
            {
               double inputLatencySec = streamInfo.getInputLatency(); // input latency in seconds
               double inputLatencyFrames = inputLatencySec * samplesPerSec / samplesPerFrame;
               if (inputLatencyFrames == 0)
               {
                  // just in case some driver is broken
                  inputLatencyFrames = MIN_DRIVER_LATENCY;
               }
               
               echoQueueLatency += inputLatencyFrames;
            }
            else
            {
               echoQueueLatency += MIN_DRIVER_LATENCY;
            }            
         }
         else
         {
            echoQueueLatency += MIN_DRIVER_LATENCY;
         }
         if (outputStreamId)
         {
            MpAudioStreamInfo streamInfo;
            OsStatus res = pAudioDriver->getStreamInfo(outputStreamId, streamInfo);
            if (res == OS_SUCCESS)
            {
               double outputLatencySec = streamInfo.getOutputLatency(); // input latency in seconds
               double outputLatencyFrames = outputLatencySec * samplesPerSec / samplesPerFrame;
               if (outputLatencyFrames == 0)
               {
                  // just in case some driver is broken
                  outputLatencyFrames = MIN_DRIVER_LATENCY;
               }

               echoQueueLatency += outputLatencyFrames;
            }
            else
            {
               echoQueueLatency += MIN_DRIVER_LATENCY;
            }            
         }
         else
         {
            echoQueueLatency += MIN_DRIVER_LATENCY;
         }
      }
   }

   return echoQueueLatency;
#else
   return 1;
#endif
}

UtlBoolean MpCallFlowGraph::writeWAVHeader(int handle)
{
    UtlBoolean retCode = FALSE;
    char tmpbuf[80];
    short bitsPerSample = 16;

    short sampleSize = sizeof(MpAudioSample); 
    short compressionCode = 1; //PCM
    short numChannels = 1; 
    unsigned long samplesPerSecond = MpMisc.m_audioSamplesPerSec;
    unsigned long averageSamplePerSec = samplesPerSecond*sampleSize;
    short blockAlign = sampleSize*numChannels; 
    unsigned long bytesWritten = 0;

    //write RIFF & length
    //8 bytes written
    strcpy(tmpbuf,"RIFF");
    unsigned long length = 0;
    bytesWritten += write(handle, tmpbuf, (unsigned)strlen(tmpbuf));
    bytesWritten += write(handle, (char*)&length, sizeof(length)); //filled in on close
    
    //write WAVE & length
    //8 bytes written
    strcpy(tmpbuf,"WAVE");
    bytesWritten += write(handle, tmpbuf, (unsigned)strlen(tmpbuf));
//    bytesWritten += write(handle,&length, sizeof(length)); //filled in on close

    //write fmt & length
    //8 bytes written
    strcpy(tmpbuf,"fmt ");
    length = 16;
    bytesWritten += write(handle, tmpbuf, (unsigned)strlen(tmpbuf));
    bytesWritten += write(handle, (char*)&length,sizeof(length)); //filled in on close
    
    //now write each piece of the format
    //16 bytes written
    bytesWritten += write(handle, (char*)&compressionCode, sizeof(compressionCode));
    bytesWritten += write(handle, (char*)&numChannels, sizeof(numChannels));
    bytesWritten += write(handle, (char*)&samplesPerSecond, sizeof(samplesPerSecond));
    bytesWritten += write(handle, (char*)&averageSamplePerSec, sizeof(averageSamplePerSec));
    bytesWritten += write(handle, (char*)&blockAlign, sizeof(blockAlign));
    bytesWritten += write(handle, (char*)&bitsPerSample, sizeof(bitsPerSample));


    //write data and length
    strcpy(tmpbuf,"data");
    length = 0;
    bytesWritten += write(handle, tmpbuf, (unsigned)strlen(tmpbuf));
    bytesWritten += write(handle, (char*)&length, sizeof(length)); //filled in on close
    
    //total length at this point should be 44 bytes
    if (bytesWritten == 44)
        retCode = TRUE;

    return retCode;

}

// Handles an incoming message for the flow graph.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpCallFlowGraph::handleMessage(OsMsg& rMsg)
{
   UtlBoolean retCode;

   retCode = FALSE;
   char msgType = rMsg.getMsgType();

   if (msgType == OsMsg::OS_TIMER_MSG)
   {
      const MpTimerMsg* pMsg = dynamic_cast<const MpTimerMsg*>(&rMsg);
      if (pMsg)
      {
         return handleTimerMessage(*pMsg);
      }
   }
   else if (msgType == OsMsg::MP_FLOWGRAPH_MSG)
   {
      MpFlowGraphMsg* pMsg = dynamic_cast<MpFlowGraphMsg*>(&rMsg);
      if (pMsg)
      {
         //
         // Handle Normal Flow Graph Messages
         //
         switch (pMsg->getMsg())
         {
         case MpFlowGraphMsg::FLOWGRAPH_REMOVE_CONNECTION:
            retCode = handleRemoveConnection(*pMsg);
            break;
         case MpFlowGraphMsg::FLOWGRAPH_START_PLAY:
            retCode = handleStartPlay(*pMsg);
            break;
         case MpFlowGraphMsg::FLOWGRAPH_START_RECORD:
            retCode = handleStartRecord(*pMsg);
            break;
         case MpFlowGraphMsg::FLOWGRAPH_STOP_RECORD:
            // osPrintf("\n++++++ recording stopped\n");
            // retCode = handleStopRecord(rMsg);
            break;
         case MpFlowGraphMsg::FLOWGRAPH_STOP_PLAY:
         case MpFlowGraphMsg::FLOWGRAPH_STOP_TONE:
            retCode = handleStopToneOrPlay();
            break;
         case MpFlowGraphMsg::ON_MPRRECORDER_ENABLED:
            retCode = handleOnMprRecorderEnabled(*pMsg);
            break;
         case MpFlowGraphMsg::ON_MPRRECORDER_DISABLED:
            retCode = handleOnMprRecorderDisabled(*pMsg);
            break;
         default:
            retCode = MpFlowGraphBase::handleMessage(*pMsg);
            break;
         }
      }
   }

   return retCode;
}

UtlBoolean MpCallFlowGraph::handleTimerMessage(const MpTimerMsg& rMsg)
{
   switch ((MpTimerMsg::SubTypeEnum)rMsg.getMsgSubType())
   {
   case MpTimerMsg::MP_STOP_DTMF_TONE_TIMER:
      return handleStopDTMFTimerMessage((const MpStopDTMFTimerMsg&)rMsg);
   default:
      break;
   }

   return FALSE;
}

UtlBoolean MpCallFlowGraph::handleStopDTMFTimerMessage(const MpStopDTMFTimerMsg& rMsg)
{
   // don't delete the timer which fired us, since it could have been replaced by another timer

   // stop sending DTMF tone (in-band or rfc2833)
   stopTone();

   return TRUE;
}

// Handle the FLOWGRAPH_REMOVE_CONNECTION message.
// Returns TRUE if the message was handled, otherwise FALSE.
UtlBoolean MpCallFlowGraph::handleRemoveConnection(MpFlowGraphMsg& rMsg)
{
   MpConnectionID connID = rMsg.getInt1();
   MpRtpInputAudioConnection* pInputConnection;
   MpRtpOutputAudioConnection* pOutputConnection;
   UtlBoolean    res;

   // Don't think this is needed as we make the ports available by
   // releasing the ports on the connected resources
   //mpBridge->disconnectPort(connID);
   mConnTableLock.acquire();
   pInputConnection = mpInputConnections[connID];
   pOutputConnection = mpOutputConnections[connID];
   mpInputConnections[connID] = NULL;
   mpOutputConnections[connID] = NULL;
   mpBridgePorts[connID] = -1;
   mConnTableLock.release();

   // now remove synchronous resources from flow graph
   if(pInputConnection)
   {
       res = handleRemoveResource(pInputConnection);
       assert(res);
       delete pInputConnection;
   }

   if(pOutputConnection)
   {
       res = handleRemoveResource(pOutputConnection);
       assert(res);
       delete pOutputConnection;
   }

   return TRUE;
}

UtlBoolean MpCallFlowGraph::handleStartPlay(MpFlowGraphMsg& rMsg)
{
   UtlBoolean boolRes;
   int toneOptions = rMsg.getInt1();

   boolRes = mpFromFile->enable();          assert(boolRes);

#ifndef DISABLE_LOCAL_AUDIO // [
   // Also play locally if requested
   if (toneOptions & TONE_TO_SPKR)
   {
      boolRes = mpTFsBridgeMixer->enable();   assert(boolRes);
   }

   if (toneOptions & TONE_TO_NET)
   {
      // Play the file audio through the Mixer resource,
      // shutting off the other audio input
      boolRes = mpTFsMicMixer->disable();   assert(boolRes);
   }
#ifdef DOING_ECHO_CANCELATION // [
   boolRes = mpEchoCancel->disable();  assert(boolRes);
#endif // DOING_ECHO_CANCELATION ]
#endif // DISABLE_LOCAL_AUDIO ]
   return TRUE;
}

UtlBoolean MpCallFlowGraph::handleStartRecord(MpFlowGraphMsg& rMsg)
{
   for (int i = 0; i < MAX_RECORDERS; i++)
   {
      if (mpRecorders[i])
      {
         mpRecorders[i]->begin();
      }
   }
   return TRUE;
}

UtlBoolean MpCallFlowGraph::handleStopToneOrPlay()
{
   UtlBoolean boolRes;

#ifdef DOING_ECHO_CANCELATION /* [ */
   MpMediaTask* pMediaTask;

   pMediaTask = MpMediaTask::getMediaTask();
#endif /* DOING_ECHO_CANCELATION ] */

#ifndef DISABLE_LOCAL_AUDIO // [
   // only pass bridge input, and not tonegen/fileplay
   boolRes = mpTFsBridgeMixer->disable();
   assert(boolRes);
   // only pass from mic, and not tonegen/fileplay
   boolRes = mpTFsMicMixer->enable();
   assert(boolRes);

#ifdef DOING_ECHO_CANCELATION // [
   if (sbEnableAEC && (this == pMediaTask->getFocus()))
   {
      boolRes = mpEchoCancel->enable();
      assert(boolRes);
   }
#endif // DOING_ECHO_CANCELATION ]
#endif // DISABLE_LOCAL_AUDIO ]
   return TRUE;
}

UtlBoolean MpCallFlowGraph::handleOnMprRecorderEnabled(MpFlowGraphMsg& rMsg)
{
   UtlBoolean boolRes; 
   int status = rMsg.getInt1();
   MprRecorder* pRecorder = (MprRecorder*) rMsg.getPtr1();

#ifndef DISABLE_LOCAL_AUDIO

   // if this call recorder, also enable required resources
   if (pRecorder && pRecorder == mpRecorders[RECORDER_CALL])
   {
      boolRes = mpCallrecMixer->enable();
      assert(boolRes);

      boolRes = mpMicCallrecSplitter->enable();
      assert(boolRes);

      boolRes = mpSpeakerCallrecSplitter->enable();
      assert(boolRes);

      sendInterfaceNotification(MP_NOTIFICATION_AUDIO, MP_NOTIFICATION_RECORDING_STARTED, 0);
   }

#endif

   return TRUE;
}

UtlBoolean MpCallFlowGraph::handleOnMprRecorderDisabled(MpFlowGraphMsg& rMsg)
{
   UtlBoolean boolRes;
   int status = rMsg.getInt1();
   MprRecorder* pRecorder = (MprRecorder*) rMsg.getPtr1();

#ifndef DISABLE_LOCAL_AUDIO

   if (pRecorder && pRecorder == mpRecorders[RECORDER_CALL])
   {
      // also disable mpCallrecMixer and splitters

      /* checks just in case this gets called when
      the resources have already been deleted
      */
      if (mpCallrecMixer)
      {
         boolRes = mpCallrecMixer->disable();
         assert(boolRes);
      }
      if (mpMicCallrecSplitter)
      {
         boolRes = mpMicCallrecSplitter->disable();
         assert(boolRes);
      }
      if (mpSpeakerCallrecSplitter)
      {
         boolRes = mpSpeakerCallrecSplitter->disable();
         assert(boolRes);
      }

      sendInterfaceNotification(MP_NOTIFICATION_AUDIO, MP_NOTIFICATION_RECORDING_STOPPED, 0);
   }

#endif

   return TRUE;
}

/* ============================ FUNCTIONS ================================= */



