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

#ifndef _MpdSipxG726_h_  /* [ */
#define _MpdSipxG726_h_

#ifdef HAVE_SPAN_DSP /* [ */

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpDecoderBase.h"
extern "C" {
#include <spandsp/telephony.h>
#include "spandsp/private/bitstream.h"
#include "spandsp/bitstream.h"
#include "spandsp/g726.h"
#include "spandsp/private/g726.h"
}

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

/// Derived class for G.726 decoder.
class MpdSipxG726 : public MpDecoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{
   typedef enum
   {
      BITRATE_16,
      BITRATE_24,
      BITRATE_32,
      BITRATE_40,
   } G726_BITRATE;

     /// Constructor
   MpdSipxG726(int payloadType, G726_BITRATE bitRate);
     /**<
     *  @param payloadType - (in) RTP payload type associated with this decoder
     */

     /// Destructor
   virtual ~MpdSipxG726();

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
   static const MpCodecInfo ms_codecInfo16; ///< Static information about the codec 16 kbit/s
   static const MpCodecInfo ms_codecInfo24; ///< Static information about the codec 24 kbit/s
   static const MpCodecInfo ms_codecInfo32; ///< Static information about the codec 32 kbit/s
   static const MpCodecInfo ms_codecInfo40; ///< Static information about the codec 40 kbit/s

   static const MpCodecInfo* getCodecInfo(G726_BITRATE bitRate);

   g726_state_t* m_pG726state;
};

#endif /* HAVE_SPAN_DSP ] */

#endif  // _MpdSipxG726_h_ ]
