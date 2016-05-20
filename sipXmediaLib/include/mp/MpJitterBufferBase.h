//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef MpJitterBufferBase_h__
#define MpJitterBufferBase_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <os/OsIntTypes.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
class MpRtpBufPtr;

// STRUCTS
// TYPEDEFS

typedef enum JitterBufferResult
{
   MP_JITTER_BUFFER_OK = 0, ///< returned when expected frame was found
   MP_JITTER_BUFFER_PLC, ///< returned when PLC frame was returned from jitter buffer
   MP_JITTER_BUFFER_FRAME_SKIP, ///< returned when frame skip was detected. Valid frame is returned.
   MP_JITTER_BUFFER_MISSING, ///< returned when no frame was found, jitter buffer is empty
   MP_JITTER_BUFFER_INCOMPLETE,
   MP_JITTER_BUFFER_ERROR
} JitterBufferResult;

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/**
 * Common class for all jitter buffers.
 */
class MpJitterBufferBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

   /**
    * Jitter buffer constructor
    *
    * @param name Name of codec this jitter buffer is used for i.e "GSM".
    *        Mainly for debugging.
    * @param payloadType RTP Payload type. Jitter buffer will reject
    *        RTP frames for other payload types.
    * @param samplesPerFrame Number of samples per frame used by the codec,
    *        not internal frame size in sipxmedialib. Will be probably
    *        80, 160, 240...
    */
   MpJitterBufferBase(const UtlString& name,
                      int payloadType,
                      unsigned int samplesPerFrame);

   virtual ~MpJitterBufferBase();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

   /**
    * Resets jitter buffer.
    */
   virtual void reset() = 0;

   /**
    * Notifies jitter buffer of frame incrementation. Should be
    * done every time we pull a packet.
    */
   virtual void frameIncrement() = 0;

   /**
    * Pulls one frame from jitter buffer and stores it into supplied MpRtpBufPtr.
    *
    * @param pOutRtp MpRtpBufPtr supplied by caller where we store the frame. Only
    *        payload, payload size, payload type and timestamp can be relied upon.
    */
   virtual JitterBufferResult pull(MpRtpBufPtr &pOutRtp) = 0;

   /**
    * Pushes an RTP frame into jitter buffer. Jitter buffer will throw out
    * a frame with incorrect payload type.
    */
   virtual void push(const MpRtpBufPtr &pRtp) = 0;

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   int getPayloadType();

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

   virtual int getBufferLength() = 0;

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   UtlString m_name;
   int m_payloadType;
   unsigned int m_samplesPerFrame;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // MpJitterBufferBase_h__
