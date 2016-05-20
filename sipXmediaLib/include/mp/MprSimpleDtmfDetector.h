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

#ifndef _MprDecodeInBandDtmf_h_
#define _MprDecodeInBandDtmf_h_

// APPLICATION INCLUDES
#include <mp/MprDtmfDetectorBase.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class MpRtpInputAudioConnection;

/**
*  Simple inband DTMF detector using Goertzel algorithm from wikipedia.
*/
class MprSimpleDtmfDetector : public MprDtmfDetectorBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   friend class MpRtpInputAudioConnection;

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     //:Constructor
   MprSimpleDtmfDetector(const UtlString& rName, 
                         int samplesPerFrame,
                         int samplesPerSec);

     //:Destructor
   virtual ~MprSimpleDtmfDetector();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
   /// Reset the state of the detector.
   void reset();
   /**<
   *  Reset all accumulators, last detected DTMF, and recalculate coefficients.
   */

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{
//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame = 80,
                                     int samplesPerSecond = 8000);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /// Handle messages for this resource.
   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);

     /// Copy constructor (not implemented for this class)
   MprSimpleDtmfDetector(const MprSimpleDtmfDetector& rhs);

     /// Assignment operator (not implemented for this class)
   MprSimpleDtmfDetector& operator=(const MprSimpleDtmfDetector& rhs);

   /// Calculate coefficients needed for the goertzel algorithm
   void calcCoeffs();
   /**<
   *  These coefficients are dependent on the sample rate, so new coefficients
   *  need to be calculated whenever the sample rate changes.
   *
   *  From Wikipedia:
   *
   * coef = 2.0 * cos( (2.0 * PI * k) / (float)GOERTZEL_N)) ;
   * Where k = (int) (0.5 + ((float)GOERTZEL_N * target_freq) / SAMPLING_RATE));
   *
   * More simply: coef = 2.0 * cos( (2.0 * PI * target_freq) / SAMPLING_RATE );
   */

   /// Validate the detected frequencies detected by processSample.
   void dtmfValidation();
   /**<
   *  This looks at the detected frequencies, and determines if it conforms
   *  to DTMF rules as specified by bell, and others -- i.e. can work well
   *  with PSTN.
   */

   /// Process a sample through the detector.
   UtlBoolean processSample(const MpAudioSample sample);
   /**<
   *  When getNumProcessSamples() samples are processed, a DTMF tone is either
   *  detected or not.  In either case, the stored last DTMF tone detected is
   *  overwritten with the current detection value.
   *  
   *  @return /p FALSE if this has not yet processed a multiple of getNumProcessSamples(), 
   *          /p TRUE if this has processed a multiple of getNumProcessSamples()
   *  
   */

   /** Returns TRUE if DTMF notification should be sent. */
   UtlBoolean shouldSendNotification();

   unsigned m_numProcessSamples;
   uint32_t m_sampleCount;

   static double ms_freqs_to_detect[]; 
   static uint8_t ms_nFreqsToDetect;

   double* m_q1;
   double* m_q2;
   double* m_r;

   double* m_coefs;

   char m_currentDtmfDigit; ///< remembers current detected DTMF digit
   char m_lastDtmfDigit; ///< remembers last detected DTMF digit
   int m_sameDtmfDigitCount;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprDecodeInBandDtmf_h_
