//  
// Copyright (C) 2006 SIPez LLC. 
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
#include <assert.h>

// APPLICATION INCLUDES

#include <os/OsMsg.h>
#include <os/OsMsgPool.h>

#include "mp/MpBuf.h"
#include "mp/MprToSpkr.h"
#include "mp/MpBufferMsg.h"
#include "mp/MpMediaTask.h"
#include "mp/MpAudioDriverManager.h"
#include "mp/MpAudioDriverBase.h"
#include "mp/MpAudioDriverDefs.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprToSpkr::MprToSpkr(const UtlString& rName,
                     int samplesPerFrame,
                     int samplesPerSec,
                     OsMsgQ *pEchoQ)
: MpAudioResource(rName, 1, 1, 0, 1, samplesPerFrame, samplesPerSec)
, mpEchoQ(pEchoQ)
{
}

// Destructor
MprToSpkr::~MprToSpkr()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprToSpkr::doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame,
                                     int samplesPerSecond)
{
   // We have only one input
   if (inBufsSize != 1)
   {      
      return FALSE;
   }

   // Do processing if enabled and if data is available
   if (isEnabled && inBufs[0].isValid()) 
   {
      MpAudioBufPtr out;
      // Own input buffer
      out = inBufs[0];

      MpAudioSample* shpSamples = out->getSamplesWritePtr();
      int iLength = out->getSamplesNumber();

      MpAudioDriverManager* pAudioManager = MpAudioDriverManager::getInstance();
      if (pAudioManager)
      {
         MpAudioStreamId streamId = pAudioManager->getOutputAudioStream();
         MpAudioDriverBase* pAudioDriver = pAudioManager->getAudioDriver();
         if (streamId && pAudioDriver)
         {
            pAudioDriver->writeStream(streamId, shpSamples, iLength);
         }
      }

      // Push data to the output, if connected.
      if (isOutputConnected(0))
      {
         outBufs[0] = out;
      }

      if (mpEchoQ)
      {
         // if queue is not full, add message
         if (mpEchoQ->numMsgs() < mpEchoQ->maxMsgs())
         {
            // Post a copy of this message to the mpEchoQ so that it
            // can be used in AEC calculations.
            MpBufferMsg AECMsg(MpBufferMsg::ACK_EOSTREAM);
            MpAudioBufPtr echoBuf;

            // clone buffer
            echoBuf = out.clone();

            // Buffer is moved to the message. ob pointer is invalidated.
            AECMsg.ownBuffer(echoBuf);

            mpEchoQ->send(AECMsg, OsTime::NO_WAIT_TIME);
         }         
      }      
   }
   else 
   {
      // Push data to the output, if connected.
      if (isOutputConnected(0))
      {
         outBufs[0] = inBufs[0];
      }
   }

   return TRUE;
}

/* ============================ FUNCTIONS ================================= */
