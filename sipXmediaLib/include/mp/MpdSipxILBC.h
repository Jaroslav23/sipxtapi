//
// Copyright (C) 2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//  
// Copyright (C) 2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifndef _MpdSipxILBC_h_  // [
#define _MpdSipxILBC_h_

#ifdef HAVE_ILBC // [

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/MpDecoderBase.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

/// Internal iLBC codec structure (from iLBC_define.h)
struct iLBC_Dec_Inst_t_;

/// Derived class for iLBC decoder. Capable of decoding both 20ms and 30ms frames regardless of selected mode.
class MpdSipxILBC : public MpDecoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpdSipxILBC(int payloadType, int mode = 30);
     /**<
     *  @param payloadType - (in) RTP payload type associated with this decoder
     */

     /// Destructor
   virtual ~MpdSipxILBC();

     /// Initializes a codec data structure for use as a decoder
   virtual OsStatus initDecode();
     /**<
     *  @returns <b>OS_SUCCESS</b> - Success
     *  @returns <b>OS_NO_MEMORY</b> - Memory allocation failure
     */

     /// Frees all memory allocated to the decoder by <i>initDecode</i>
   virtual OsStatus freeDecode(void);
     /**<
     *  @returns <b>OS_SUCCESS</b> - Success
     *  @returns <b>OS_DELETED</b> - Object has already been deleted
     */

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Decode incoming RTP packet
   virtual int decode(const MpRtpBufPtr &rtpPacket, ///< (in) Pointer to a media buffer
                      unsigned decodedBufferLength, ///< (in) Length of the samplesBuffer (in samples)
                      MpAudioSample *samplesBuffer, ///< (out) Buffer for decoded samples
                      UtlBoolean bIsPLCFrame
                      );
     /**<
     *  @return Number of decoded samples.
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

/* //////////////////////////// PRIVATE /////////////////////////////////// */

private:
   static const MpCodecInfo smCodecInfo20ms;  ///< static information about the 20ms codec version
   static const MpCodecInfo smCodecInfo30ms;  ///< static information about the 30ms codec version

   iLBC_Dec_Inst_t_ *m_pState20; ///< Internal iLBC decoder state for 20ms
   iLBC_Dec_Inst_t_ *m_pState30; ///< Internal iLBC decoder state for 30ms

   int m_mode; ///< iLBC mode. 20 or 30 is a valid value. Currently doesn't have a meaning, since we decode both frame sizes.
   float m_tmpOutBuffer[240]; // use buffer for 30ms frames, even for 20ms
};

#endif // HAVE_ILBC ]

#endif  // _MpdSipxILBC_h_ ]
