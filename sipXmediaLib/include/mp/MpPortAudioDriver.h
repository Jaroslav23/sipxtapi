//
// Copyright (C) 2007 Jaroslav Libak
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef MpPortAudioDriver_h__
#define MpPortAudioDriver_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsMutex.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlString.h>
#include "mp/MpAudioDriverBase.h"
#include "mp/MpHostAudioApiInfo.h"
#include "mp/MpAudioDeviceInfo.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
class MpAudioMixerBase;

// STRUCTS
// TYPEDEFS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/**
* Wrapper for Portaudio V19 driver. It cannot be created directly.
* Use MpAudioDriverFactory to create it. Only 1 instance can exist
* at time.
*/
class MpPortAudioDriver : public MpAudioDriverBase
{
   /**
    * Make MpAudioDriverFactory a friend so it can create this instance
    */
   friend class MpAudioDriverFactory;
   friend class MpPortAudioMixer;

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
   * Returns name of driver. Threadsafe.
   */
   virtual const UtlString& getDriverName() const;

   /**
   * Returns driver version string. Threadsafe.
   */
   virtual const UtlString& getDriverVersion() const;

   /**
    * Returns number of host apis available for this driver. Portaudio
    * will usually have multiple apis for each platform.
    *
    * @param count Number of host apis if successful
    * @returns OS_SUCCESS if successful
    */
   virtual OsStatus getHostApiCount(int& count) const;

   /**
    * Returns index of default driver host api.
    *
    * @param apiIndex Index of default host api if successful
    * @returns OS_SUCCESS if successful
    */
   virtual OsStatus getDefaultHostApi(MpHostAudioApiIndex& apiIndex) const;

   /**
    * Gets information about given host API. Portaudio
    * implements several host apis.
    *
    * @param hostApiIndex Index of host api to get information for.
    * @param apiInfo
    */
   virtual OsStatus getHostApiInfo(MpHostAudioApiIndex hostApiIndex,
                                   MpHostAudioApiInfo& apiInfo) const;

   /**
    * Converts host type id to host api index. 
    *
    * @param hostApiTypeId Id of host api belonging to MpHostAudioApiTypeId enum
    * @param hostApiIndex Index of host api
    * @returns OS_SUCCESS if successful
    */
   virtual OsStatus hostApiTypeIdToHostApiIndex(MpHostAudioApiTypeId hostApiTypeId,
                                                MpHostAudioApiIndex& hostApiIndex) const;

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
                                                    MpAudioDeviceIndex& deviceIndex) const;

   /**
    * Returns the number of available devices.
    * 
    * @param deviceCount Number of available devices
    * @returns OS_SUCCESS if successful
    */
   virtual OsStatus getDeviceCount(MpAudioDeviceIndex& deviceCount) const;

   /**
    * Returns index of default input device
    *
    * @param deviceIndex Index of default input device
    * @returns OS_SUCCESS if successful
    */
   virtual OsStatus getDefaultInputDevice(MpAudioDeviceIndex& deviceIndex) const;

   /**
   * Returns index of default output device
   *
   * @param deviceIndex Index of default output device
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus getDefaultOutputDevice(MpAudioDeviceIndex& deviceIndex) const;

   /**
    * Returns information about given audio device.
    *
    * @param deviceIndex Index of audio device.
    * @param deviceInfo Information about audio device.
    * @returns OS_SUCCESS if successful
    */
   virtual OsStatus getDeviceInfo(MpAudioDeviceIndex deviceIndex,
                                  MpAudioDeviceInfo& deviceInfo) const;

   /**
   * Returns mixer for given audio stream. This mixer can then be used
   * to set volume on the stream. For half-duplex streams, create only
   * 1 mixer with mixerIndex 0. For full-duplex streams, it might be
   * necessary to create more mixers, depending on the sound card.
   * In such case, create a mixer and use its methods to find out
   * whether more mixers can be created. Returned
   *
   * @param stream Audio stream to create mixer for
   * @param mixerIndex Index of mixer. Some sound cards might have more than 1.
   *        default is 0.
   * @returns Returned audio mixer implementation
   */
   virtual MpAudioMixerBase* getMixerForStream(MpAudioStreamId stream,
                                               int mixerIndex) const;

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
                               UtlBoolean synchronous);


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
                                      UtlBoolean synchronous);

   /**
    * Stops and closes audio stream. After this call, stream is no longer valid.
    *
    * @param stream Id of stream we want to stop and close.
    * @returns OS_SUCCESS if successful
    */
   virtual OsStatus closeStream(MpAudioStreamId stream);

   /**
    * Starts given stream. Stream must be started before
    * user can push/pull data into/from it.
    *
    * @param stream Id of audio stream
    * @returns OS_SUCCESS if successful
    */
   virtual OsStatus startStream(MpAudioStreamId stream);

   /**
    * Stops given stream after all current audio is played.
    *
    * @param stream Id of audio stream
    * @returns OS_SUCCESS if successful
    */
   virtual OsStatus stopStream(MpAudioStreamId stream);

   /**
    * Aborts given stream prematurely. Audio will be stopped immediately.
    *
    * @param stream Id of audio stream
    * @returns OS_SUCCESS if successful
    */
   virtual OsStatus abortStream(MpAudioStreamId stream);

   /**
    * Gets information about given audio stream.
    *
    * @param stream Id of audio stream
    * @param streamInfo Information about requested audio stream
    * @returns OS_SUCCESS if successful
    */
   virtual OsStatus getStreamInfo(MpAudioStreamId stream, MpAudioStreamInfo& streamInfo) const;

   /**
    * Gets real stream time. It can be used to synchronize different
    * streams.
    *
    * @param stream Id of audio stream
    * @param streamTime Real time of stream.
    * @returns OS_SUCCESS if successful
    */
   virtual OsStatus getStreamTime(MpAudioStreamId stream, double& streamTime) const;

   /**
    * Returns CPU load for given audio stream.
    *
    * @param stream Id of audio stream
    * @param cpuLoad Returned CPU load
    * @returns OS_SUCCESS if successful
    */
   virtual OsStatus getStreamCpuLoad(MpAudioStreamId stream, double& cpuLoad) const;

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
                               unsigned long frames);

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
                                unsigned long frames);


   /**
   * Gets number of frames that can be read from a synchronous audio stream without
   * blocking.
   *
   * @param stream Id of audio stream
   * @param framesAvailable Number of frames that can be read without blocking
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus getStreamReadAvailable(MpAudioStreamId stream, long& framesAvailable) const;

   /**
    * Gets number of frames that can be written to a synchronous audio stream without
    * blocking.
    *
    * @param stream Id of audio stream
    * @param framesAvailable Number of frames that can be written without blocking
    * @returns OS_SUCCESS if successful
    */
   virtual OsStatus getStreamWriteAvailable(MpAudioStreamId stream, long& framesAvailable) const;

   /**
    * Gets size of sample in bytes for given sample format.
    *
    * @param format Sample format. See @MpAudioDriverDefs.h
    * @param sampleSize Returned size of sample
    * @returns OS_SUCCESS if successful
    */
   virtual OsStatus getSampleSize(MpAudioDriverSampleFormat format, int& sampleSize) const;

   /**
   * Gets volume meter reading for input of given stream.
   *
   * @param stream Id of audio stream
   * @param volume Current volume calculated from samples.
   */
   virtual OsStatus getInputVolumeMeterReading(MpAudioStreamId stream,
                                               double& volume,
                                               MP_VOLUME_METER_TYPE type) const;

   /**
   * Gets volume meter reading for output of given stream.
   *
   * @param stream Id of audio stream
   * @param volume Current volume calculated from samples.
   */
   virtual OsStatus getOutputVolumeMeterReading(MpAudioStreamId stream,
                                                double& volume,
                                                MP_VOLUME_METER_TYPE type) const;

   /**
   * Deletes audio driver. Use instead of deleting it directly.
   * Not threadsafe.
   */
   virtual void release();

   //@}

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{
   //@}
   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{

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
                                      double sampleRate) const;

   /**
   * Whether stream is stopped. Stream is stopped after stopStream or abortStream
   * 
   * @param stream Id of audio stream
   * @param isStopped Whether stream is stopped
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus isStreamStopped(MpAudioStreamId stream, UtlBoolean& isStopped) const;

   /**
   * Whether stream is active. Stream is active after startStream and before 
   * stopStream or abortStream.
   *
   * @param stream Id of audio stream
   * @param isActive Whether stream is active
   * @returns OS_SUCCESS if successful
   */
   virtual OsStatus isStreamActive(MpAudioStreamId stream, UtlBoolean& isActive) const;

   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /// Private constructor.
   MpPortAudioDriver();

   /// Private destructor.
   virtual ~MpPortAudioDriver(void);

   /// Copy constructor (not implemented for this class)
   MpPortAudioDriver(const MpPortAudioDriver& rMpAudioDriver);

   /// Assignment operator (not implemented for this class)
   MpPortAudioDriver& operator=(const MpPortAudioDriver& rhs);

   /**
    * Creates instance of this class. To be used only by MpAudioDriverFactory.
    * Threadsafe.
    *
    * @returns instance if it can be created or NULL if no more instances are allowed.
    */
   static MpPortAudioDriver* createInstance();

   /**
    * Resets stream if its asynchronous.
    */
   void resetStreamInternal(MpAudioStreamId stream);

   /**
    * Checks whether this stream exists.
    */
   UtlBoolean isStreamValid(MpAudioStreamId stream) const;

   static unsigned int ms_instanceCounter; ///< counter of MpPortAudioDriver instances
   static OsMutex ms_driverMutex; ///< mutex for protection of audio driver calls
   static UtlString ms_driverName; ///< name of driver
   static UtlString ms_driverVersion; ///< version of driver

   UtlHashMap m_audioStreamMap; ///< stream callbacks map
};

#endif // MpPortAudioDriver_h__
