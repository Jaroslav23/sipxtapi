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

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "mp/MpBuf.h"
#include "mp/MprFromMic.h"
#include "mp/MpAudioDriverManager.h"
#include "mp/MpAudioDriverBase.h"
#include "mp/MpAudioDriverDefs.h"
#include "mp/MpMisc.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprFromMic::MprFromMic(const UtlString& rName,
                       int samplesPerFrame,
                       int samplesPerSec)
: MpAudioResource(rName, 0, 1, 1, 1, samplesPerFrame, samplesPerSec)
, m_framesProcessed(0)
{
}

// Destructor
MprFromMic::~MprFromMic()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


UtlBoolean MprFromMic::doProcessFrame(MpBufPtr inBufs[],
                                      MpBufPtr outBufs[],
                                      int inBufsSize,
                                      int outBufsSize,
                                      UtlBoolean isEnabled,
                                      int samplesPerFrame,
                                      int samplesPerSecond)
{
   MpAudioBufPtr out;

   // We need one output buffer
   if (outBufsSize != 1) 
      return FALSE;

   // Don't waste the time if output is not connected
   if (!isOutputConnected(0))
      return TRUE;

   // One more frame processed
   m_framesProcessed++;

   if (isEnabled) 
   {
      MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
      if (pAudioManager)
      {
         MpAudioStreamId streamId = pAudioManager->getInputAudioStream();
         MpAudioDriverBase* pAudioDriver = pAudioManager->getAudioDriver();
         if (streamId && pAudioDriver)
         {
            MpAudioBufPtr micFrame = MpMisc.m_pRawAudioPool->getBuffer();
            if (micFrame.isValid())
            {
               micFrame->setSamplesNumber(MpMisc.m_audioSamplesPerFrame);

               OsStatus res = pAudioDriver->readStream(streamId,
                                    micFrame->getSamplesWritePtr(),
                                    MpMisc.m_audioSamplesPerFrame);

               if (res == OS_SUCCESS || res == OS_UNDERFLOW)
               {
                  // if we are totally out of data, OS_PREFETCH will be returned
                  out.swap(micFrame);
                  out->setSpeechType(MP_SPEECH_UNKNOWN); // speech is detected in Speex preprocessor
               }
            }
         }
      }
   }
   else
   {
      // if disabled just pass input further
      out = inBufs[0];
   }

   outBufs[0] = out;

   return TRUE;
}

/* ============================ FUNCTIONS ================================= */
