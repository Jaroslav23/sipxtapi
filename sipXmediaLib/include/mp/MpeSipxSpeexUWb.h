//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 Hector Izquierdo Seliva. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// Copyright (C) 2008-2009 Jaroslav Libak.  All rights reserved.
// Licensed under the LGPL license.
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifndef _MpeSipxSpeexUWb_h_
#define _MpeSipxSpeexUWb_h_

#ifdef HAVE_SPEEX /* [ */

// APPLICATION INCLUDES
#include "mp/MpEncoderBase.h"
#include <speex/speex.h>
#include <speex/speex_header.h>
#include <speex/speex_stereo.h>
#include <speex/speex_preprocess.h>

/// Derived class for Speex encoder.
class MpeSipxSpeexUWb: public MpEncoderBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpeSipxSpeexUWb(int payloadType, int mode = 6);
     /**<
     *  @param payloadType - (in) RTP payload type associated with this encoder
     *  @param mode - (in) Speex encoder mode: 
     *                     <pre>
     *                     mode = 0 - 5,750 bps - Barely intelligible (mostly for comfort noise)
     *                     mode = 1 - 7,550 bps - Very noticeable artifacts/noise, poor intelligibility
     *                     mode = 2 - 9,550 bps - Very noticeable artifacts/noise, good intelligibility
     *                     mode = 3 - 11,600 bps - Artifacts/noise sometimes annoying
     *                     mode = 4 - 14,600 bps - Artifacts/noise usually noticeable
     *                     mode = 5 - 18,600 bps - Artifacts/noise sometimes noticeable
     *                     mode = 6 - 22,400 bps - Need good headphones to tell the difference
     *                     mode = 7 - 25,600 bps - Need good headphones to tell the difference
     *                     mode = 8 - 29,600 bps - Hard to tell the difference even with good headphones
     *                     mode = 9 - 36,000 bps - Hard to tell the difference even with good headphones
     *                     mode = 10 - 44,000 bps - Completely transparent for voice, good quality music
     *                     </pre>
     *                     If not supported mode will be passed, default mode
     *                     will be used.
     */

     /// Destructor
   virtual ~MpeSipxSpeexUWb(void);

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
   static const MpCodecInfo smCodecInfo; ///< Static information about the codec

   SpeexBits mBits;         ///< Bits used by speex to store information
   void *mpEncoderState;    ///< State of the encoder   
   int mSampleRate;         ///< Sample rate
   int mMode;               ///< Speex mode
   int mDoVad;              ///< Set to 1 to enable voice activity detection
   int mDoDtx;              ///< Set to 1 to enable discontinuous transmission
   int mDoVbr;              ///< Set to 1 to enable variable bitrate mode
   spx_int16_t mpBuffer[640]; ///< Buffer used to store input samples
   int mBufferLoad;          ///< How much data there is in the byffer
   bool mDoPreprocess;         ///< Should we do preprocess or not
   SpeexPreprocessState *mpPreprocessState; ///< Preprocessor state
   int mDoDenoise;             ///< Denoises the input
   int mDoAgc;                 ///< Automatic Gain Control

};

#endif /* HAVE_SPEEX ] */

#endif  // _MpeSipxSpeexUWb_h_
