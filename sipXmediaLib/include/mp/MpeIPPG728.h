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

#ifndef _MpeIPPG728_h_
#define _MpeIPPG728_h_

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

/**
 * Derived class for Intel IPP G.728. Capable of encoding to 9.6 kbit/s, 12.8 kbit/s
 * and 16 kbit/s. rfc3551 mentions only 16 kbit/s therefore we encode using this rate.
 */
class MpeIPPG728: public MpEncoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpeIPPG728(int payloadType, int bitRate = 16000);
     /**<
     *  @param payloadType - (in) RTP payload type associated with this encoder
     */

     /// Destructor
   virtual ~MpeIPPG728();

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
   static const MpCodecInfo ms_codecInfo9600;  ///< static information about the codec 9600
   static const MpCodecInfo ms_codecInfo12800;  ///< static information about the codec 12800
   static const MpCodecInfo ms_codecInfo16000;  ///< static information about the codec 16000

   static const MpCodecInfo* getCodecInfo(int bitRate);

   LoadedCodec *m_pCodec;  ///< Loaded codec info
   Ipp8s* m_pInputBuffer;
   Ipp8u* m_pOutputBuffer;
};

#endif // HAVE_INTEL_IPP ]

#endif  // _MpeIPPG728_h_
