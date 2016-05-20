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

#ifndef _MprSpanDspDtmfDetector_h_
#define _MprSpanDspDtmfDetector_h_

#ifdef HAVE_SPAN_DSP /* [ */

// APPLICATION INCLUDES
#include <mp/MprDtmfDetectorBase.h>
extern "C" {
#include "spandsp/telephony.h"
#include "spandsp/super_tone_rx.h"
#include "spandsp/dtmf.h"
}

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
*  Inband DTMF detector using Span DSP library.
*/
class MprSpanDspDtmfDetector : public MprDtmfDetectorBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   friend class MpRtpInputAudioConnection;

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     //:Constructor
   MprSpanDspDtmfDetector(const UtlString& rName, 
                          int samplesPerFrame,
                          int samplesPerSec);

     //:Destructor
   virtual ~MprSpanDspDtmfDetector();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
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

     /// Copy constructor (not implemented for this class)
   MprSpanDspDtmfDetector(const MprSpanDspDtmfDetector& rhs);

     /// Assignment operator (not implemented for this class)
   MprSpanDspDtmfDetector& operator=(const MprSpanDspDtmfDetector& rhs);

   static void tone_report_func_t(void *user_data, int code, int level, int delay);

   /**
    * Called when DTMF tone starts or stops. Code is translated from original char value,
    * so digit '0' will be 0 etc.
    */
   void onDtmfTone(char dtmfDigitCode, int level, int delay);

   dtmf_rx_state_t* m_pDtmfState;
};

/* ============================ INLINE METHODS ============================ */

#endif /* HAVE_SPAN_DSP ] */

#endif  // _MprDecodeInBandDtmf_h_
