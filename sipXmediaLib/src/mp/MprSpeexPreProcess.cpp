//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006 ProfitFuel Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_SPEEX // [

// SYSTEM INCLUDES
#include <speex/speex_echo.h>
#include <speex/speex_preprocess.h>
#include <speex/speex_types.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "mp/MpBuf.h"
#include "mp/MprSpeexPreProcess.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprSpeexPreprocess::MprSpeexPreprocess(const UtlString& rName,
                          int samplesPerFrame, int samplesPerSec)
: MpAudioResource(rName, 1, 2, 1, 1, samplesPerFrame, samplesPerSec)
, m_bVadEnabled(FALSE)
{
   // Initialize Speex preprocessor state
   mpPreprocessState = speex_preprocess_state_init(samplesPerFrame, samplesPerSec);
}

// Destructor
MprSpeexPreprocess::~MprSpeexPreprocess()
{
   if (mpPreprocessState)
   {
      // detach echo canceller
      speex_preprocess_ctl(mpPreprocessState, SPEEX_PREPROCESS_SET_ECHO_STATE, NULL);
      // free mpPreprocessState
      speex_preprocess_state_destroy(mpPreprocessState);
      mpPreprocessState = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean MprSpeexPreprocess::setAGC (UtlBoolean enable)
{
   MpFlowGraphMsg msg(SET_AGC, this, NULL, NULL, enable);
   return (postMessage(msg) == OS_SUCCESS);
}

UtlBoolean MprSpeexPreprocess::setNoiseReduction (UtlBoolean enable)
{
   MpFlowGraphMsg msg(SET_NOISE_REDUCTION, this, NULL, NULL, enable);
   return (postMessage(msg) == OS_SUCCESS);
}

UtlBoolean MprSpeexPreprocess::setVAD(UtlBoolean enable)
{
   MpFlowGraphMsg msg(SET_VAD, this, NULL, NULL, enable);
   return (postMessage(msg) == OS_SUCCESS);
}

UtlBoolean MprSpeexPreprocess::attachEchoCanceller(SpeexEchoState* pEchoState)
{
   if (pEchoState && mpPreprocessState)
   {
      // attach echo canceller
      speex_preprocess_ctl(mpPreprocessState, SPEEX_PREPROCESS_SET_ECHO_STATE, pEchoState);
   }
   
   return FALSE;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprSpeexPreprocess::doProcessFrame(MpBufPtr inBufs[],
                                              MpBufPtr outBufs[],
                                              int inBufsSize,
                                              int outBufsSize,
                                              UtlBoolean isEnabled,
                                              int samplesPerFrame,
                                              int samplesPerSecond)
{
   MpAudioBufPtr   inputBuffer;
   spx_int32_t*    echoResidue=NULL;
   bool res = false;

   // We don't need to do anything if we don't have an output.
   if (outBufsSize != 1)
      return FALSE;

   // Get incoming data
   inputBuffer.swap(inBufs[0]);
   int isVoiceActive = 1;

   // If the object is not enabled or we don't have valid input, pass input to output
   if (  isEnabled
      && inputBuffer.isValid()
      && (inputBuffer->getSamplesNumber() == samplesPerFrame))
   {
      // This buffer will be modified in place. Make sure we're the only owner.
      res = inputBuffer.requestWrite();
      assert(res);

      // Get Echo residue if we have any
      isVoiceActive = speex_preprocess_run(mpPreprocessState, (spx_int16_t*)inputBuffer->getSamplesPtr());
   }

   if (inputBuffer.isValid())
   {
      if (m_bVadEnabled)
      {
         if (isVoiceActive)
         {
            inputBuffer->setSpeechType(MP_SPEECH_ACTIVE);
         }
         else
         {
            inputBuffer->setSpeechType(MP_SPEECH_SILENT);
         }
      }
      else
      {
         inputBuffer->setSpeechType(MP_SPEECH_UNKNOWN);
      }
   }

   outBufs[0].swap(inputBuffer);
   return TRUE;
}


// Handle messages for this resource.
UtlBoolean MprSpeexPreprocess::handleMessage(MpFlowGraphMsg& rMsg)
{
   int msgType;

   msgType = rMsg.getMsg();
   switch (msgType)
   {
   case SET_AGC:
      return handleSetAGC(rMsg.getInt1());
      break;
   case SET_NOISE_REDUCTION:
      return handleSetNoiseReduction(rMsg.getInt1());
      break;
   case SET_VAD:
      return handleSetVAD(rMsg.getInt1());
      break;
   default:
      return MpAudioResource::handleMessage(rMsg);
      break;
   }
}

// Handle the SET_AGC message.
UtlBoolean MprSpeexPreprocess::handleSetAGC(UtlBoolean enable)
{
   speex_preprocess_ctl(mpPreprocessState, SPEEX_PREPROCESS_SET_AGC, &enable);
   return TRUE;
}

// Handle the SET_NOISE_REDUCTION message.
UtlBoolean MprSpeexPreprocess::handleSetNoiseReduction(UtlBoolean enable)
{
   speex_preprocess_ctl(mpPreprocessState, SPEEX_PREPROCESS_SET_DENOISE, &enable);
   return TRUE;
}

UtlBoolean MprSpeexPreprocess::handleSetVAD(UtlBoolean enable)
{
   m_bVadEnabled = enable;
   speex_preprocess_ctl(mpPreprocessState, SPEEX_PREPROCESS_SET_VAD, &m_bVadEnabled);
   return TRUE;
}

/* ============================ FUNCTIONS ================================= */
#endif // HAVE_SPEEX ]