//
// Copyright (C) 2007 Jaroslav Libak
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef MpPortAudioStream_h__
#define MpPortAudioStream_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsTime.h>
#include <utl/UtlBool.h>
#include <mp/MpPortAudioStreamBase.h>
#include "portaudio.h"

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
* Representation of asynchronous portlib audio stream. For synchronous streams, it is not
* needed. Stream can be output/input/full duplex. It is only used for callback. Callback
* and main thread share the same buffer using lockless algorithm. The basis of the algorithm
* is that each thread writes to its own volatile variable, and reads from other threads variable.
* No 2 threads write to the same variable. When writing to buffer, writing pointer can never
* exceed or equal reading pointer, and when reading, reading pointer cannot exceed or equal
* to writing pointer. Thus threads will always use disjoint memory blocks.
* Statistics about input/output underflows and overflows are computed and can be printed if needed.
*/
class MpAsyncPortAudioStream : public MpPortAudioStreamBase
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Constructor.
   MpAsyncPortAudioStream(MpAudioStreamId streamId,
                          int outputChannelCount,
                          int inputChannelCount,
                          MpAudioDriverSampleFormat outputSampleFormat,
                          MpAudioDriverSampleFormat inputSampleFormat,
                          double sampleRate,
                          unsigned long framesPerBuffer);

   /// Destructor.
   ~MpAsyncPortAudioStream(void);
   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

   /**
    * audio callback according to portaudio spec. In user data it receives
    * pointer to MpPortAudioStream instance
    */
   static int streamCallback(const void *input,
                             void *output,
                             unsigned long frameCount,
                             const PaStreamCallbackTimeInfo* timeInfo,
                             PaStreamCallbackFlags statusFlags,
                             void *userData );

   /**
    * Reads given number of frames into buffer. Buffer must be allocated memory.
    *
    * @param buffer Allocated memory to read data into
    * @param frames Number of frames to read
    * @returns OS_SUCCESS if successful
    */
   virtual OsStatus readStream(void *buffer,
                               unsigned long frames);

   /**
    * Writes given number of frames into stream from buffer.
    *
    * @param buffer Buffer with samples
    * @param frames Number of frames to write to stream
    * @returns OS_SUCCESS if successful
    */
   virtual OsStatus writeStream(const void *buffer,
                                unsigned long frames);

   /**
    * Prints overflow/underflow statistics.
    */
   void printStatistics();

   /**
    * Resets internal stream buffers to 0 and resets statistics. Needs to be done
    * after stream is stopped or aborted. Not thread safe.
    */
   virtual void resetStream();

   //@}

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{

   /**
    * Returns input latency in seconds of the stream.
    */
   virtual double getInputLatency() const;

   /**
    * Returns output latency in seconds of the stream.
    */
   virtual double getOutputLatency() const;

   //@}
   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{
   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /// Copy constructor (not implemented for this class)
   MpAsyncPortAudioStream(const MpAsyncPortAudioStream& rMpPortAudioStream);

   /// Assignment operator (not implemented for this class)
   MpAsyncPortAudioStream& operator=(const MpAsyncPortAudioStream& rhs);

   /**
    * Stream callback of this instance to support multiple streams.
    */
   int instanceStreamCallback(const void *input,
                              void *output,
                              unsigned long frameCount,
                              const PaStreamCallbackTimeInfo* timeInfo,
                              PaStreamCallbackFlags statusFlags);

   /**
    * Gets maximum bytes that can be copied from input to output pos without exceeding it.
    * It is useful when preventing inputPos exceeding outputPos.
    */
   int getCopyableBytes(unsigned int inputPos, unsigned int outputPos, unsigned int maxPos) const;

   /**
    * Gets actual number of frames in input buffer
    */
   int getInputBufferFrameCount();

   /**
    * Gets actual number of frames in output buffer
    */
   int getOutputBufferFrameCount();

   unsigned long m_virtualFramesPerBuffer; ///< only used for initializing m_inputWritePos and m_outputWritePos

   void* m_pInputBuffer; ///< buffer for storing recorded samples
   void* m_pOutputBuffer; ///< buffer for storing frames going to speaker
   unsigned int m_inputBufferSize; ///< size of input buffer
   unsigned int m_outputBufferSize; ///< size of output buffer
   volatile unsigned int m_inputWritePos; ///< position in input buffer for writing
   volatile unsigned int m_inputReadPos; ///< position in input buffer for reading
   volatile unsigned int m_outputWritePos; ///< position in output buffer for writing
   volatile unsigned int m_outputReadPos; ///< position in output buffer for reading

   unsigned int m_outputBufferOverflow;
   unsigned int m_outputBufferUnderflow;
   unsigned int m_inputBufferOverflow;
   unsigned int m_inputBufferUnderflow;

   volatile bool m_inputBufferPrefetchMode; ///< whether input buffer is in prefetch mode
   volatile bool m_outputBufferPrefetchMode; ///< whether output buffer is in prefetch mode
   unsigned int m_inputPrefetchCount; ///< input buffer latency in frames
   unsigned int m_outputPrefetchCount; ///< output buffer latency in frames
   bool m_bFrameRecorded; ///< whether at least one frame has been recorded
   bool m_bFramePushed; ///< whether at least one frame has been pushed

   int m_streamReadWriteCount;
   int m_callbackCallCount;
};

#endif // MpPortAudioStream_h__
