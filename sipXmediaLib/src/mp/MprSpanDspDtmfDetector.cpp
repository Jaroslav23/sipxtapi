//  
// Copyright (C) 2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// Copyright (C) 2009 Jaroslav Libak
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifdef HAVE_SPAN_DSP /* [ */

// SYSTEM INCLUDES
#include <assert.h>
#include <stdio.h>
#include <math.h>

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

// APPLICATION INCLUDES
#include "mp/MprSpanDspDtmfDetector.h"
#include "mp/MpRtpInputAudioConnection.h"
#include "mp/MpResNotification.h"
#include "mp/MprRecorder.h"
#include "os/OsSysLog.h"

// DECODER DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprSpanDspDtmfDetector::MprSpanDspDtmfDetector(const UtlString& rName,
                                               int samplesPerFrame,
                                               int samplesPerSec)
: MprDtmfDetectorBase(rName, samplesPerFrame, samplesPerSec)
, m_pDtmfState(NULL)
{
   m_pDtmfState = dtmf_rx_init(NULL, NULL, NULL); // don't use digit callback
   assert(m_pDtmfState != NULL);
   dtmf_rx_set_realtime_callback(m_pDtmfState, tone_report_func_t, (void*)this); // real time callback reports both start & stop
   dtmf_rx_parms(m_pDtmfState, 1, -1, -1, -99); // enable dial tone filtering, leave the rest at default values
}

// Destructor
MprSpanDspDtmfDetector::~MprSpanDspDtmfDetector()
{
   dtmf_rx_free(m_pDtmfState);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean MprSpanDspDtmfDetector::doProcessFrame(MpBufPtr inBufs[],
                                               MpBufPtr outBufs[],
                                               int inBufsSize,
                                               int outBufsSize,
                                               UtlBoolean isEnabled,
                                               int samplesPerFrame,
                                               int samplesPerSecond)
{
   // Don't process frame if disabled
   if (!isEnabled)
   {
      return TRUE;
   }

   // Check input buffer
   assert(inBufsSize == 1);
   if(!inBufs[0].isValid())
   {
      return TRUE;
   }

   // Get samples from buffer
   const MpAudioSample* input = ((MpAudioBufPtr)inBufs[0])->getSamplesPtr();
   int numSamples = ((MpAudioBufPtr)inBufs[0])->getSamplesNumber();

   int unprocessedSamples = dtmf_rx(m_pDtmfState, input, numSamples);
   /*for (int i = 0; i < numSamples; i++)
   {
      UtlBoolean bCheckResult = processSample(input[i]);
      if (bCheckResult)
      {
         if (shouldSendNotification())
         {
            notify(MP_RES_DTMF_INBAND_NOTIFICATION, m_currentDtmfDigit);
         }
      }
   }*/

   return TRUE;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void MprSpanDspDtmfDetector::tone_report_func_t(void *user_data, int code, int level, int delay)
{
   MprSpanDspDtmfDetector* pDtmfDetector = (MprSpanDspDtmfDetector*)user_data;
   if (pDtmfDetector)
   {
      char decodedDigit = -1;
      switch(code)
      {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
         decodedDigit = code - '0'; // '0' - 0, '1' - 1 etc
         break;
      case 'A':
      case 'B':
      case 'C':
      case 'D':
         decodedDigit = code - 53; // 'A' - 12, 'B' - 13 etc
         break;
      case '*':
         decodedDigit = 10;
         break;
      case '#':
         decodedDigit = 11;
         break;
      default:
         ;
      }

      if (decodedDigit >= 0)
      {
         pDtmfDetector->onDtmfTone(decodedDigit, level, delay);
      }
   }
}

void MprSpanDspDtmfDetector::onDtmfTone(char dtmfDigitCode, int level, int delay)
{
   notify(MP_RES_DTMF_INBAND_NOTIFICATION, dtmfDigitCode);
}

/* ============================ FUNCTIONS ================================= */

#endif /* HAVE_SPAN_DSP ] */
