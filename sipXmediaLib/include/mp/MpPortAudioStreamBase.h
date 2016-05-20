//
// Copyright (C) 2007 Jaroslav Libak
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef MpPortAudioStreamBase_h__
#define MpPortAudioStreamBase_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <utl/UtlDefs.h>
#include <utl/UtlContainable.h>
#include "mp/MpAudioDriverDefs.h"

// DEFINES
#define MIN_PORT_SAMPLE_RATE 200

// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS
class MpVolumeMeterBase;

/**
 * Base class for synchronous and asynchronous port audio streams.
 */
class MpPortAudioStreamBase : public UtlContainable
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   static const UtlContainableType TYPE; /** < Class type used for runtime checking */ 

   /* ============================ CREATORS ================================== */

   /** Constructor */
   MpPortAudioStreamBase(MpAudioStreamId streamId,
                         int outputChannelCount,
                         int inputChannelCount,
                         MpAudioDriverSampleFormat outputSampleFormat,
                         MpAudioDriverSampleFormat inputSampleFormat,
                         double sampleRate,
                         unsigned long framesPerBuffer);

   /** Destructor */
   virtual ~MpPortAudioStreamBase();

   /* ============================ MANIPULATORS ============================== */

   /**
   * Reads given number of frames into buffer. Buffer must be allocated memory.
   *
   * @param buffer Allocated memory to read data into
   * @param frames Number of frames to read
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus readStream(void *buffer,
                               unsigned long frames) = 0;

   /**
   * Writes given number of frames into stream from buffer.
   *
   * @param buffer Buffer with samples
   * @param frames Number of frames to write to stream
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus writeStream(const void *buffer,
                                unsigned long frames) = 0;

   /**
   * Resets internal stream buffers to 0 and resets statistics. Needs to be done
   * after stream is stopped or aborted. Not thread safe.
   */
   virtual void resetStream();

   /* ============================ ACCESSORS ================================= */

   /**
   * Calculate a unique hash code for this object.  If the equals
   * operator returns true for another object, then both of those
   * objects must return the same hashcode.
   */    
   virtual unsigned hash() const;

   /**
   * Get the ContainableType for a UtlContainable derived class.
   */
   virtual UtlContainableType getContainableType() const;

   /**
    * Returns input latency in seconds of the stream.
    */
   virtual double getInputLatency() const = 0;

   /**
    * Returns output latency in seconds of the stream.
    */
   virtual double getOutputLatency() const = 0;

   /**
    * Gets latency of the input port audio stream
    * which excludes latency of the wrapper.
    */
   double getInputPortStreamLatency() const { return m_inputPortStreamLatency; }
   void setInputPortStreamLatency(double val) { m_inputPortStreamLatency = val; }

   /**
    * Gets latency of the output port audio stream
    * which excludes latency of the wrapper.
    */
   double getOutputPortStreamLatency() const { return m_outputPortStreamLatency; }
   void setOutputPortStreamLatency(double val) { m_outputPortStreamLatency = val; }

   /**
    * Returns sample rate of stream in Hz.
    */
   double getSampleRate() const;

   /* ============================ INQUIRY =================================== */

   /**
   * Compare the this object to another like-objects.  Results for 
   * designating a non-like object are undefined.
   *
   * @returns 0 if equal, < 0 if less then and >0 if greater.
   */
   virtual int compareTo(UtlContainable const* inVal) const;

   /**
   * Gets volume for input stream calculated from samples.
   */
   double getInputStreamVolume(MP_VOLUME_METER_TYPE type) const;

   /**
   * Gets volume for output stream calculated from samples.
   */
   double getOutputStreamVolume(MP_VOLUME_METER_TYPE type) const;

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   MpAudioStreamId m_streamId; ///< internal id of port audio stream
   int m_outputChannelCount; ///< number of output channels
   int m_inputChannelCount; ///< number of input channels
   MpAudioDriverSampleFormat m_outputSampleFormat; ///< sample format of output
   MpAudioDriverSampleFormat m_inputSampleFormat; ///< sample format of input
   double m_sampleRate; ///< sample rate for stream
   unsigned long m_framesPerBuffer; ///< frames per buffer for stream

   unsigned int m_inputSampleSize; ///< size of input sample in bytes per channel
   unsigned int m_outputSampleSize; ///< size of output sample in bytes per channel

   double m_inputPortStreamLatency; ///< latency in seconds of the input port audio stream
   double m_outputPortStreamLatency; ///< latency in seconds of the output port audio stream

   MpVolumeMeterBase* m_inputVolumeMeter; ///< volume meter for input
   MpVolumeMeterBase* m_outputVolumeMeter; ///< volume meter for output

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // MpPortAudioStreamBase_h__
