//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// Copyright (C) 2006-2007 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2008-2009 Jaroslav Libak.  All rights reserved.
// Licensed under the LGPL license.
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifndef _MpdSipxG722_h_  /* [ */
#define _MpdSipxG722_h_

#ifdef HAVE_SPAN_DSP /* [ */

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpDecoderBase.h"
extern "C" {
#include <spandsp/telephony.h>
#include "spandsp/private/bitstream.h"
#include "spandsp/bitstream.h"
#include "spandsp/g722.h"
#include "spandsp/private/g722.h"
}

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

/// Derived class for G.722 decoder. Decodes only 64 kbit/s.
class MpdSipxG722 : public MpDecoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{
     /// Constructor
   MpdSipxG722(int payloadType);
     /**<
     *  @param payloadType - (in) RTP payload type associated with this decoder
     */

     /// Destructor
   virtual ~MpdSipxG722();

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

     /// Decode incoming RTP packet
   virtual int decode(const MpRtpBufPtr &pPacket, ///< (in) Pointer to a media buffer
                      unsigned decodedBufferLength, ///< (in) Length of the samplesBuffer (in samples)
                      MpAudioSample *samplesBuffer, ///< (out) Buffer for decoded samples
                      UtlBoolean bIsPLCFrame
                     );
     /**<
     *  @return Number of decoded samples.
     */

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

/* //////////////////////////// PRIVATE /////////////////////////////////// */

private:
   static const MpCodecInfo ms_codecInfo64; ///< Static information about the codec 64 kbit/s

   static const MpCodecInfo* getCodecInfo();

   g722_decode_state_t* m_pG722state;
};

#endif /* HAVE_SPAN_DSP ] */

#endif  // _MpdSipxG722_h_ ]
