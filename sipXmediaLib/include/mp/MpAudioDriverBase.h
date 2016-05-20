//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef MpAudioDriverBase_h__
#define MpAudioDriverBase_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <utl/UtlString.h>
#include "mp/MpHostAudioApiInfo.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
class MpAudioDeviceInfo;
class MpAudioStreamInfo;
class MpAudioStreamParameters;
class MpAudioMixerBase;

// STRUCTS
// TYPEDEFS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/**
* Base class for all audio drivers. Constructor and destructor are private.
* Instance is created via MpAudioDriverFactory which will create instance by
* a private static method createInstance.
*/
class MpAudioDriverBase
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{
   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

   /**
    * Returns name of driver.
    */
   virtual const UtlString& getDriverName() const  = 0;

   /**
    * Returns driver version string.
    */
   virtual const UtlString& getDriverVersion() const = 0;

   /**
    * Returns number of host apis available for this driver.
    * Some drivers will have only 1 host API. Portaudio is an
    * exception.
    *
    * @param count Number of host apis if successful
    * @returns OS_SUCCESS if successful
    */
   virtual OsStatus getHostApiCount(int& count) const = 0;

   /**
   * Returns index of default driver host API.
   *
   * @param apiIndex Index of default host api if successful
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus getDefaultHostApi(MpHostAudioApiIndex& apiIndex) const = 0;

   /**
   * Gets information about given host API.
   *
   * @param hostApiIndex Index of host api to get information for.
   * @param apiInfo
   */
   virtual OsStatus getHostApiInfo(MpHostAudioApiIndex hostApiIndex,
                                   MpHostAudioApiInfo& apiInfo) const = 0;

   /**
   * Converts host type id to host api index. 
   *
   * @param hostApiTypeId Id of host api belonging to MpHostAudioApiTypeId enum
   * @param hostApiIndex Index of host api
   */
   virtual OsStatus hostApiTypeIdToHostApiIndex(MpHostAudioApiTypeId hostApiTypeId,
                                                MpHostAudioApiIndex& hostApiIndex) const = 0;


   /**
   * Converts host api device index to driver device index. These indexes can differ.
   *
   * @param hostApiIndex Index of host api
   * @param hostApiDeviceIndex Index of device for certain host api
   * @param deviceIndex Global index of audio device
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus hostApiDeviceIndexToDeviceIndex(MpHostAudioApiIndex hostApiIndex,
                                                    int hostApiDeviceIndex,
                                                    MpAudioDeviceIndex& deviceIndex) const = 0;

   /**
   * Returns the number of available devices.
   * 
   * @param deviceCount Number of available devices
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus getDeviceCount(MpAudioDeviceIndex& deviceCount) const = 0;

   /**
   * Returns index of default input device
   *
   * @param deviceIndex Index of default input device
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus getDefaultInputDevice(MpAudioDeviceIndex& deviceIndex) const = 0;

   /**
   * Returns index of default output device
   *
   * @param deviceIndex Index of default output device
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus getDefaultOutputDevice(MpAudioDeviceIndex& deviceIndex) const = 0;

   /**
   * Returns information about given audio device.
   *
   * @param deviceIndex Index of audio device.
   * @param deviceInfo Information about audio device.
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus getDeviceInfo(MpAudioDeviceIndex deviceIndex,
                                  MpAudioDeviceInfo& deviceInfo) const = 0;


   /**
   * Inquire whether given format is supported.
   *
   * @param inputParameters MpAudioStreamParameters instance with required parameters.
   *                        NULL if audio stream should be output only
   * @param outputParameters MpAudioStreamParameters instance with required parameters.
   *                         NULL if audio stream should be input only
   * @param sampleRate Requested sample rate
   * @return OS_SUCCESS if the stream can be created
   */
   virtual OsStatus isFormatSupported(const MpAudioStreamParameters* inputParameters,
                                      const MpAudioStreamParameters* outputParameters,
                                      double sampleRate) const = 0;

   /**
    * Returns mixer for given audio stream. This mixer can then be used
    * to set volume on the stream. For half-duplex streams, create only
    * 1 mixer with mixerIndex 0. For full-duplex streams, it might be
    * necessary to create more mixers, depending on the sound card.
    * In such case, create a mixer and use its methods to find out
    * whether more mixers can be created.
    *
    * @param stream Audio stream to create mixer for
    * @param mixerIndex Index of mixer. Some sound cards might have more than 1.
    *        default is 0.
    * @returns Returned audio mixer implementation
    */
   virtual MpAudioMixerBase* getMixerForStream(MpAudioStreamId stream,
                                               int mixerIndex) const = 0;

   /**
   * Opens new stream with requested parameters. Stream can be output, input
   * or full duplex.
   * 
   * @param stream Id of stream that will be returned if stream is created. Needs
   *               to be used when pushing or pulling frames from driver.
   * @param inputParameters Parameters of input stream or NULL.
   * @param outputParameters Parameters of output stream or NULL.
   * @param sampleRate Sample rate for stream
   * @param framesPerBuffer How many frames will be pushed/pulled in buffer
   * @param streamFlags Various stream flags.
   * @param synchronous Whether stream is synchronous (blocking)
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus openStream(MpAudioStreamId* stream,
                               const MpAudioStreamParameters *inputParameters,
                               const MpAudioStreamParameters *outputParameters,
                               double sampleRate,
                               unsigned long framesPerBuffer,
                               MpAudioStreamFlags streamFlags,
                               UtlBoolean synchronous) = 0;

   /**
   * Opens stream for default output/input device. 
   *
   * @param stream Id of stream that will be returned if stream is created. Needs
   *               to be used when pushing or pulling frames from driver.
   * @param numInputChannels Number of input channels requested or 0.
   * @param numOutputChannels Number of output channels requested or 0.
   * @param sampleFormat Sample format, @see MpAudioDriverDefs.h
   * @param sampleRate Sample rate for stream
   * @param framesPerBuffer How many frames will be pushed/pulled in buffer
   * @param synchronous Whether stream is synchronous (blocking)
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus openDefaultStream(MpAudioStreamId* stream,
                                      int numInputChannels,
                                      int numOutputChannels,
                                      MpAudioDriverSampleFormat sampleFormat,
                                      double sampleRate,
                                      unsigned long framesPerBuffer,
                                      UtlBoolean synchronous) = 0;

   /**
   * Stops and closes audio stream. After this call, stream is no longer valid.
   *
   * @param stream Id of stream we want to stop and close.
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus closeStream(MpAudioStreamId stream) = 0;

   /**
   * Starts given stream. Stream must be started before
   * user can push/pull data into/from it.
   *
   * @param stream Id of audio stream
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus startStream(MpAudioStreamId stream) = 0;

   /**
   * Stops given stream after all current audio is played.
   *
   * @param stream Id of audio stream
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus stopStream(MpAudioStreamId stream) = 0;

   /**
   * Aborts given stream prematurely. Audio will be stopped immediately.
   *
   * @param stream Id of audio stream
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus abortStream(MpAudioStreamId stream) = 0;

   /**
   * Whether stream is stopped. Stream is stopped after stopStream or abortStream
   * 
   * @param stream Id of audio stream
   * @param isStopped Whether stream is stopped
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus isStreamStopped(MpAudioStreamId stream, UtlBoolean& isStopped) const = 0;

   /**
   * Whether stream is active. Stream is active after startStream and before 
   * stopStream or abortStream.
   *
   * @param stream Id of audio stream
   * @param isActive Whether stream is active
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus isStreamActive(MpAudioStreamId stream, UtlBoolean& isActive) const = 0;

   /**
   * Gets information about given audio stream.
   *
   * @param stream Id of audio stream
   * @param streamInfo Information about requested audio stream
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus getStreamInfo(MpAudioStreamId stream, MpAudioStreamInfo& streamInfo) const = 0;

   /**
   * Gets real stream time. It can be used to synchronize different
   * streams.
   *
   * @param stream Id of audio stream
   * @param streamTime Real time of stream.
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus getStreamTime(MpAudioStreamId stream, double& streamTime) const = 0;

   /**
   * Returns CPU load for given audio stream.
   *
   * @param stream Id of audio stream
   * @param cpuLoad Returned CPU load
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus getStreamCpuLoad(MpAudioStreamId stream, double& cpuLoad) const = 0;

   /**
   * Read from a given stream given number of frames. Stream may be synchronous or
   * asynchronous.
   *
   * @param stream Id of audio stream
   * @param buffer Buffer to read into. Note that different sample sizes can be set
   *               on stream.
   * @param frames Number of frames to read into buffer. The actual number of bytes
   *               copied depends on sample size.
   * @returns OS_SUCCESS if successful, OS_OVERFLOW if successful but some data
   *          was lost before
   */
   virtual OsStatus readStream(MpAudioStreamId stream,
                               void *buffer,
                               unsigned long frames) = 0;

   /**
   * Write into a given stream given number of frames. Stream may be synchronous or
   * asynchronous.
   *
   * @param stream Id of audio stream
   * @param buffer Buffer to write from. Note that different sample sizes can be set
   *               on stream.
   * @param frames Number of frames to written from buffer. The actual number of bytes
   *               copied depends on sample size.
   * @returns OS_SUCCESS if successful
   */   
   virtual OsStatus writeStream(MpAudioStreamId stream,
                                const void *buffer,
                                unsigned long frames) = 0;

   /**
   * Gets number of frames that can be read from a synchronous audio stream without
   * blocking.
   *
   * @param stream Id of audio stream
   * @param framesAvailable Number of frames that can be read without blocking
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus getStreamReadAvailable(MpAudioStreamId stream, long& framesAvailable) const = 0;

   /**
   * Gets number of frames that can be written to a synchronous audio stream without
   * blocking.
   *
   * @param stream Id of audio stream
   * @param framesAvailable Number of frames that can be written without blocking
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus getStreamWriteAvailable(MpAudioStreamId stream, long& framesAvailable) const = 0;

   /**
   * Gets size of sample in bytes for given sample format.
   *
   * @param format Sample format. See @MpAudioDriverDefs.h
   * @param sampleSize Returned size of sample
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus getSampleSize(MpAudioDriverSampleFormat format, int& sampleSize) const  = 0;

   /**
    * Gets volume meter reading for input of given stream.
    */
   virtual OsStatus getInputVolumeMeterReading(MpAudioStreamId stream,
                                               double& volume,
                                               MP_VOLUME_METER_TYPE type) const = 0;

   /**
   * Gets volume meter reading for output of given stream.
   */
   virtual OsStatus getOutputVolumeMeterReading(MpAudioStreamId stream,
                                                double& volume,
                                                MP_VOLUME_METER_TYPE type) const = 0;

   /**
    * Deletes audio driver. Use instead of deleting it directly. Implemented
    * by subclass.
    */
   virtual void release() = 0;

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

   /// Constructor.
   MpAudioDriverBase();

   /// Destructor.
   virtual ~MpAudioDriverBase(void);

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /// Copy constructor (not implemented for this class)
   MpAudioDriverBase(const MpAudioDriverBase& rMpAudioDriverBase);

   /// Assignment operator (not implemented for this class)
   MpAudioDriverBase& operator=(const MpAudioDriverBase& rhs);
};

#endif // MpAudioDriverBase_h__
