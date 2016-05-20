//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef MpJitterBufferSpeex_h__
#define MpJitterBufferSpeex_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsMutex.h>
#include <mp/MpJitterBufferBase.h>
#include <mp/MpRtpBuf.h>
#include <speex/speex_jitter.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
// STRUCTS
// TYPEDEFS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/**
 * Wrapper for speex generic jitter buffer.
 */
class MpJitterBufferSpeex : public MpJitterBufferBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

   /**
    * Speex jitter buffer constructor
    *
    * @param name Name of codec this jitter buffer is used for i.e "GSM".
    *        Mainly for debugging.
    * @param payloadType RTP Payload type. Jitter buffer will reject
    *        RTP frames for other payload types.
    * @param frameSize Number of samples per frame used by the codec,
    *        not internal frame size in sipxmedialib. Will be probably
    *        80, 160, 240...
    */
   MpJitterBufferSpeex(const UtlString& name,
                       int payloadType,
                       unsigned int frameSize);

   /**
    * Speex jitter buffer destructor. Will free internal resources.
    */
   virtual ~MpJitterBufferSpeex();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

   /**
    * Resets speex jitter buffer.
    */
   virtual void reset();

   /**
    * Notifies speex jitter buffer of frame incrementation. Should be
    * done every time we pull a packet.
    */
   virtual void frameIncrement();

   /**
    * Pulls one frame from jitter buffer and stores it into supplied MpRtpBufPtr.
    *
    * @param pOutRtp MpRtpBufPtr supplied by caller where we store the frame. Only
    *        payload, payload size, payload type and timestamp are initialized.
    */
   virtual JitterBufferResult pull(MpRtpBufPtr &pOutRtp);

   /**
    * Pushes an RTP frame into jitter buffer. Jitter buffer will throw out
    * a frame with incorrect payload type.
    */
   virtual void push(const MpRtpBufPtr &pRtp);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{
//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

   virtual int getBufferLength();

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   JitterBuffer* m_pJitterBuffer; ///< pointer to speex jitter buffer
   RtpSRC m_lastSSRC;   ///< last SSRC, if it changes we need to reset jitter buffer
   bool m_bFirstFrame;  ///< whether we have yet to receive the 1st frame

   int m_bufferLength;  ///< length of speex jitter buffer, this is very inaccurate!!

   OsMutex m_speexLock;   ///< lock for push/pull, as speex is not thread safe
};

#endif // MpJitterBufferSpeex_h__
