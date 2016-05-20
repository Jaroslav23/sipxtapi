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

#ifndef _MpeIPPG729i_h_
#define _MpeIPPG729i_h_

#ifdef HAVE_INTEL_IPP // [

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpEncoderBase.h"

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

/// Derived class for Intel IPP G.729I encoder (Annex D/E).
class MpeIPPG729i: public MpEncoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpeIPPG729i(int payloadType, int bitRate);
     /**<
     *  @param payloadType - (in) RTP payload type associated with this encoder
     */

     /// Destructor
   virtual ~MpeIPPG729i();

     /// Initializes a codec data structure for use as an encoder
   virtual OsStatus initEncode(void);
     /**<
     *  @returns <b>OS_SUCCESS</b> - Success
     *  @returns <b>OS_NO_MEMORY</b> - Memory allocation failure
     */

     /// Frees all memory allocated to the encoder by <i>initEncode</i>
   virtual OsStatus freeEncode(void);
     /**<
     *  @returns <b>OS_SUCCESS</b> - Success
     *  @returns <b>OS_DELETED</b> - Object has already been deleted
     */

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

     /// Encode audio samples
   virtual OsStatus encode(const short* pAudioSamples,
                           const int numSamples,
                           int& rSamplesConsumed,
                           unsigned char* pCodeBuf,
                           const int bytesLeft,
                           int& rSizeInBytes,
                           UtlBoolean& sendNow,
                           MpSpeechType& speechType);
     /**<
     *  Processes the array of audio samples.  If sufficient samples to encode
     *  a frame are now available, the encoded data will be written to the
     *  <i>pCodeBuf</i> array.  The number of bytes written to the
     *  <i>pCodeBuf</i> array is returned in <i>rSizeInBytes</i>.
     *
     *  @param pAudioSamples - (in) Pointer to array of PCM samples
     *  @param numSamples - (in) number of samples at pAudioSamples
     *  @param rSamplesConsumed - (out) Number of samples encoded
     *  @param pCodeBuf - (out) Pointer to array for encoded data
     *  @param bytesLeft - (in) number of bytes available at pCodeBuf
     *  @param rSizeInBytes - (out) Number of bytes written to the <i>pCodeBuf</i> array
     *  @param sendNow - (out) if true, the packet is complete, send it.
     *  @param speechType - (in, out) Audio type (e.g., unknown, silence, comfort noise)
     *
     *  @returns <b>OS_SUCCESS</b> - Success
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

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static const MpCodecInfo smCodecInfo6400;  ///< static information about the codec 6400
   static const MpCodecInfo smCodecInfo8000;  ///< static information about the codec 8000
   static const MpCodecInfo smCodecInfo11800;  ///< static information about the codec 11800

   static const MpCodecInfo* getCodecInfo(int bitRate);

   LoadedCodec *codec;  ///< Loaded codec info
   Ipp8s* inputBuffer;
   Ipp8u* outputBuffer;
};

#endif // HAVE_INTEL_IPP ]

#endif  // _MpeIPPG729i_h_
