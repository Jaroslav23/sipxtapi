//
// Copyright (C) 2005 Pingtel Corp.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2008-2009 Jaroslav Libak.  All rights reserved.
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpdIPPG7221_h_  /* [ */
#define _MpdIPPG7221_h_

#ifdef HAVE_INTEL_IPP // [

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpDecoderBase.h"

extern "C" {
#include "usc.h"
#include "util.h"
#include "loadcodec.h"
}

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

/**
 * Derived class for Intel IPP G.722.1 decoder.
 */
class MpdIPPG7221: public MpDecoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
     /// Constructor
   MpdIPPG7221(int payloadType, int bitrate);
     /**<
     *  @param payloadType - (in) RTP payload type associated with this decoder
     */

     /// Destructor
   virtual ~MpdIPPG7221(void);

     /// Initializes a codec data structure for use as a decoder
   virtual OsStatus initDecode();
     /**<
     *  @param pConnection - (in) Pointer to the MpConnection container
     *  @returns <b>OS_SUCCESS</b> - Success
     *  @returns <b>OS_NO_MEMORY</b> - Memory allocation failure
     */

     /// Frees all memory allocated to the decoder by <i>initDecode</i>
   virtual OsStatus freeDecode(void);
     /**<
     *  @returns <b>OS_SUCCESS</b> - Success
     *  @returns <b>OS_DELETED</b> - Object has already been deleted
     */

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

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static const MpCodecInfo ms_codecInfo16000;  ///< static information about the codec 16000
   static const MpCodecInfo ms_codecInfo24000;  ///< static information about the codec 24000
   static const MpCodecInfo ms_codecInfo32000;  ///< static information about the codec 32000

   static const MpCodecInfo* getCodecInfo(int bitrate);

   /**
    * Configures USC_PCMStream.
    */
   void configureBitStream();

   LoadedCodec *m_pCodec; ///< Loaded codec info. Decodes 12800 and 16000. 9600 does not work.

   USC_PCMStream PCMStream;    ///< Destination data structure
   USC_Bitstream Bitstream;    ///< Source data structure
};

#endif // HAVE_INTEL_IPP ]

#endif  // _MpdSipxG729_h_ ]
