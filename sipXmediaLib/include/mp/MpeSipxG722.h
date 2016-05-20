//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2008-2009 Jaroslav Libak.  All rights reserved.
// Licensed under the LGPL license.
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifndef _MpeSipxG722_h_
#define _MpeSipxG722_h_

#ifdef HAVE_SPAN_DSP /* [ */

// APPLICATION INCLUDES
#include "mp/MpEncoderBase.h"
extern "C" {
#include <spandsp/telephony.h>
#include "spandsp/private/bitstream.h"
#include "spandsp/bitstream.h"
#include "spandsp/g722.h"
#include "spandsp/private/g722.h"
}

/**
 * Derived class for G.722 encoder. We support only 64 kbit/s encoding, since
 * other bitrates also occupy 64kbit/s, and rfc3551 talks only about 64kbit/s.
 */
class MpeSipxG722 : public MpEncoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{
     /// Constructor
   MpeSipxG722(int payloadType);
     /**<
     *  @param payloadType - (in) RTP payload type associated with this encoder
     */

     /// Destructor
   virtual ~MpeSipxG722(void);

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
   virtual OsStatus encode(const MpAudioSample* pAudioSamples,
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
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static const MpCodecInfo ms_codecInfo64; ///< Static information about the codec 64 kbit/s

   static const MpCodecInfo* getCodecInfo();

   g722_encode_state_t* m_pG722state;
};

#endif /* HAVE_SPAN_DSP ] */

#endif  // _MpeSipxG722_h_
