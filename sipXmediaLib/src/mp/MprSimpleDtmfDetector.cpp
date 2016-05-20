//  
// Copyright (C) 2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// Copyright (C) 2007 stipus@stipus.com
// $$ 
////////////////////////////////////////////////////////////////////////////// 

// Author: Keith Kyzivat <kkyzivat AT SIPez DOT com>

// SYSTEM INCLUDES
#include <assert.h>
#include <stdio.h>
#include <math.h>

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

// APPLICATION INCLUDES
#include <mp/MpDefs.h>
#include "mp/MprSimpleDtmfDetector.h"
#include "mp/MpRtpInputAudioConnection.h"
#include "mp/MpResNotification.h"
#include "mp/MprRecorder.h"
#include "os/OsSysLog.h"

// DECODER DEFINES
// don't use directly
#define GOERTZEL_N 92

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// STATIC VARIABLE INITIALIZATIONS

// Class data allocation
double MprSimpleDtmfDetector::ms_freqs_to_detect[] = 
{ 
   697, 770, 852, 941, // DTMF key row
   1209, 1336, 1477, 1633 // DTMF key column
};

uint8_t MprSimpleDtmfDetector::ms_nFreqsToDetect = sizeof(MprSimpleDtmfDetector::ms_freqs_to_detect)/sizeof(double);

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprSimpleDtmfDetector::MprSimpleDtmfDetector(const UtlString& rName,
                                             int samplesPerFrame,
                                             int samplesPerSec)
: MprDtmfDetectorBase(rName, samplesPerFrame, samplesPerSec)
, m_numProcessSamples(0)
, m_sameDtmfDigitCount(0)
{
   m_q1 = new double[ms_nFreqsToDetect];
   m_q2 = new double[ms_nFreqsToDetect];
   m_r = new double[ms_nFreqsToDetect];
   m_coefs = new double[ms_nFreqsToDetect];
   // recalculate the number of samples to process according to sampling rate
   m_numProcessSamples = GOERTZEL_N * (unsigned int)((double)samplesPerSec / SAMPLES_PER_SECOND_8KHZ);
   reset();
}

// Destructor
MprSimpleDtmfDetector::~MprSimpleDtmfDetector()
{
   delete[] m_q1;
   delete[] m_q2;
   delete[] m_r;
   delete[] m_coefs;
}

/* ============================ MANIPULATORS ============================== */

void MprSimpleDtmfDetector::reset()
{
   m_sampleCount = 0;
   m_currentDtmfDigit = -1;
   m_lastDtmfDigit = -1;
   m_sameDtmfDigitCount = 0;
   int i;
   for(i=0; i< ms_nFreqsToDetect; i++)
   {
      m_q1[i] = 0;
      m_q2[i] = 0;
      m_r[i] = 0;
      m_coefs[i] = 0;
   }

   // Now calculate new coefficients 
   calcCoeffs();
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean MprSimpleDtmfDetector::doProcessFrame(MpBufPtr inBufs[],
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

   for (int i = 0; i < numSamples; i++)
   {
      UtlBoolean bCheckResult = processSample(input[i]);
      if (bCheckResult)
      {
         if (shouldSendNotification())
         {
            notify(MP_RES_DTMF_INBAND_NOTIFICATION, m_currentDtmfDigit);
         }
      }
   }

   return TRUE;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprSimpleDtmfDetector::handleMessage(MpFlowGraphMsg& rMsg)
{
   switch (rMsg.getMsg())
   {
   case 0: // to make compiler stop complaining
   default:
      return MpAudioResource::handleMessage(rMsg);
      break;
   }
   return TRUE;
}

void MprSimpleDtmfDetector::calcCoeffs()
{
   int n;

   for(n = 0; n < ms_nFreqsToDetect; n++)
   {
      m_coefs[n] = 2.0 * cos(2.0 * 3.141592654 * ms_freqs_to_detect[n] / getSamplesPerSec());
   }
}

void MprSimpleDtmfDetector::dtmfValidation()
{
   int row, col, passed_tests;
   int peak_count, max_index;
   double maxval, t;
   int i;
   char row_col_ascii_codes[4][4] = 
   {
      {1,  2, 3,  12}, // 1, 2, 3, A
      {4,  5, 6,  13}, // 4, 5, 6, B
      {7,  8, 9,  14}, // 7, 8, 9, C
      {10, 0, 11, 15}  // *, 0, #, D
   };

   // Find the largest in the row group.
   row = 0;
   maxval = 0.0;
   for ( i=0; i<4; i++ )
   {
      if ( m_r[i] > maxval )
      {
         maxval = m_r[i];
         row = i;
      }
   }

   // Find the largest in the column group.
   col = 4;
   maxval = 0.0;
   for ( i=4; i<8; i++ )
   {
      if ( m_r[i] > maxval )
      {
         maxval = m_r[i];
         col = i;
      }
   }

   // Check for minimum energy
   if ( m_r[row] < 4.0e5 )   // 2.0e5 ... 1.0e8 no change
   {
      // row frequency energy is not high enough
   }
   else if ( m_r[col] < 4.0e5 )
   {
      // column frequency energy is not high enough
   }
   else
   {
      passed_tests = TRUE;

      // Twist check
      // CEPT => twist < 6dB
      // AT&T => forward twist < 4dB and reverse twist < 8dB
      //  -ndB < 10 log10( v1 / v2 ), where v1 < v2
      //  -4dB < 10 log10( v1 / v2 )
      //  -0.4  < log10( v1 / v2 )
      //  0.398 < v1 / v2
      //  0.398 * v2 < v1
      if ( m_r[col] > m_r[row] )
      {
         // Normal twist
         max_index = col;
         if ( m_r[row] < (m_r[col] * 0.398) )    // twist > 4dB results in error
         {
            passed_tests = FALSE;
         }
      }
      else // if ( r[row] > r[col] )
      {
         // Reverse twist
         max_index = row;
         if ( m_r[col] < (m_r[row] * 0.158) )    // twist > 8db results in error
         {
            passed_tests = FALSE;
         }
      }

      // Signal to noise test
      // AT&T states that the noise must be 16dB down from the signal.
      // Here we count the number of signals above the threshold and
      // there ought to be only two.
      if ( m_r[max_index] > 1.0e9 )
      {
         t = m_r[max_index] * 0.158;
      }
      else
      {
         t = m_r[max_index] * 0.010;
      }

      peak_count = 0;
      for ( i=0; i<8; i++ )
      {
         if ( m_r[i] > t )
            peak_count++;
      }
      if ( peak_count > 2 )
      {
         passed_tests = FALSE;
      }

      // Set the last detected DTMF tone.
      m_currentDtmfDigit = passed_tests ? row_col_ascii_codes[row][col-4] : -1;
   }
}

UtlBoolean MprSimpleDtmfDetector::processSample(const MpAudioSample sample)
{
   UtlBoolean ret = FALSE;
   double q0;
   uint32_t i;

   m_sampleCount++;
   for ( i=0; i<ms_nFreqsToDetect; i++ )
   {
      q0 = m_coefs[i] * m_q1[i] - m_q2[i] + sample;
      m_q2[i] = m_q1[i];
      m_q1[i] = q0;
   }

   if (m_sampleCount == m_numProcessSamples)
   {
      for ( i=0; i<ms_nFreqsToDetect; i++ )
      {
         m_r[i] = (m_q1[i] * m_q1[i]) + (m_q2[i] * m_q2[i]) - (m_coefs[i] * m_q1[i] * m_q2[i]);
         m_q1[i] = 0.0;
         m_q2[i] = 0.0;
      }
      dtmfValidation();
      m_sampleCount = 0;
      ret = TRUE;
   }
   return ret;
}

UtlBoolean MprSimpleDtmfDetector::shouldSendNotification()
{
   // TRIGGER NOTIFICATION
   // If a new digit has been found
   if (m_lastDtmfDigit == m_currentDtmfDigit)
   {
      m_sameDtmfDigitCount++;

      if (m_sameDtmfDigitCount == 4 && m_currentDtmfDigit != -1)
      {
         return TRUE;
      }
   }
   else
   {
      if (m_currentDtmfDigit == -1)
      {
         if (m_sameDtmfDigitCount >= 4)
         {
            m_sameDtmfDigitCount++;
         }
         else
         {
            m_sameDtmfDigitCount = 0;
         }

         if (m_sameDtmfDigitCount >= 30)
         {
            m_lastDtmfDigit = m_currentDtmfDigit;
            m_sameDtmfDigitCount = 0;
         }
      }
      else
      {
         m_lastDtmfDigit = m_currentDtmfDigit;
         m_sameDtmfDigitCount = 0;
      }
   }

   return FALSE;
}

/* ============================ FUNCTIONS ================================= */

