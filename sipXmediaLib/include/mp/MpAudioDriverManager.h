//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef MpAudioDriverManager_h__
#define MpAudioDriverManager_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsMutex.h>
#include <utl/UtlBool.h>
#include "mp/MpAudioDriverDefs.h"
#include "mp/MpAudioDeviceInfo.h"
// use STL vector
#include <vector>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
class MpAudioDriverBase;
class MpAudioMixerBase;
class MpAudioStreamInfo;
class UtlString;

// STRUCTS
// TYPEDEFS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/**
* Class responsible for creation and deletion of audio driver instance. Can be used
* to access the current active audio driver and input/output stream Ids
*/
class MpAudioDriverManager
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
    * Gets new or existing instance of MpAudioDriverManager
    *
    * @param bCreate Set to FALSE if new instance shouldn't be created if
    * it doesn't exist
    */
   static MpAudioDriverManager* getInstance(UtlBoolean bCreate = TRUE);

   /**
    * Returns name of current output device and driver name.
    */
   OsStatus getCurrentOutputDevice(MpAudioDeviceInfo& deviceInfo) const;

   /**
    * Returns name of current input device and driver name.
    */
   OsStatus getCurrentInputDevice(MpAudioDeviceInfo& deviceInfo) const;

   /**
   * Sets current output device. NONE disables it, "Default" will select
   * default one. If it fails, it will try not to disable current stream.
   */
   OsStatus setCurrentOutputDevice(const UtlString& device, const UtlString& driverName);

   /**
   * Sets current input device. NONE disables it, "Default" will select
   * default one. If it fails, it will try not to disable current stream.
   */
   OsStatus setCurrentInputDevice(const UtlString& device, const UtlString& driverName);

   /**
    * Returns number of input devices
    */
   int getInputDeviceCount() const;

   /**
    * Returns number of output devices
    */
   int getOutputDeviceCount() const;

   /**
    * Gets information about given input device
    */
   OsStatus getInputDeviceInfo(int deviceIndex, MpAudioDeviceInfo& deviceInfo);

   /**
    * Gets information about given output device
    */
   OsStatus getOutputDeviceInfo(int deviceIndex, MpAudioDeviceInfo& deviceInfo);

   /**
   * Gets information about input audio stream.
   *
   * @param streamInfo Information about requested audio stream
   * @returns OS_SUCCESS if successful
   */
   OsStatus getInputStreamInfo(MpAudioStreamInfo& streamInfo);

   /**
   * Gets information about output audio stream.
   *
   * @param streamInfo Information about requested audio stream
   * @returns OS_SUCCESS if successful
   */
   OsStatus getOutputStreamInfo(MpAudioStreamInfo& streamInfo);

   /**
   * Starts input stream.
   */
   OsStatus startInputStream() const;

   /**
   * Starts output stream.
   */
   OsStatus startOutputStream() const;

   /**
    * Aborts input stream.
    */
   OsStatus abortInputStream() const;

   /**
    * Aborts output stream.
    */
   OsStatus abortOutputStream() const;

   /**
    * Closes input stream.
    */
   OsStatus closeInputStream();

   /**
    * Closes output stream.
    */
   OsStatus closeOutputStream();

   /**
    * Gets name of input mixer
    */
   void getInputMixerName(UtlString& name) const;

   /**
   * Gets name of output mixer
   */
   void getOutputMixerName(UtlString& name) const;

   /**
    * Gets master output volume
    */
   MpAudioVolume getMasterVolume() const;

   /**
    * Sets master output volume
    */
   void setMasterVolume(MpAudioVolume volume);

   /**
    * Gets main output volume
    */
   MpAudioVolume getPCMOutputVolume() const;

   /**
    * Sets main output volume
    */
   void setPCMOutputVolume(MpAudioVolume volume);

   /**
    * Gets input volume
    */
   MpAudioVolume getInputVolume() const;

   /**
    * Sets input volume
    */
   void setInputVolume(MpAudioVolume volume);

   /**
    * Gets output balance (right, left)
    */
   MpAudioBalance getOutputBalance() const;

   /**
    * Sets output balance
    */
   void setOutputBalance(MpAudioBalance balance);

   /**
    * Gets current input volume from volume meter calculated from samples.
    */
   OsStatus getInputVolumeMeterReading(MP_VOLUME_METER_TYPE type, double& volume) const;

   /**
   * Gets current output volume from volume meter calculated from samples.
   */
   OsStatus getOutputVolumeMeterReading(MP_VOLUME_METER_TYPE type, double& volume) const;

   /**
    * Deletes singleton manager. Not threadsafe.
    */
   void release();

   //@}

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{

   MpAudioStreamId getInputAudioStream() const { return m_inputAudioStream; }
   MpAudioStreamId getOutputAudioStream() const { return m_outputAudioStream; }

   MpAudioDeviceIndex getInputDeviceIndex() const { return m_inputDeviceIndex; }
   MpAudioDeviceIndex getOutputDeviceIndex() const { return m_outputDeviceIndex; }

   MpAudioDriverBase* getAudioDriver() const { return m_pAudioDriver; }

   double getInitialInputStreamLatency() const { return m_initialInputStreamLatency; }
   void setInitialInputStreamLatency(double val) { m_initialInputStreamLatency = val; }

   double getInitialOutputStreamLatency() const { return m_initialOutputStreamLatency; }
   void setInitialOutputStreamLatency(double val) { m_initialOutputStreamLatency = val; }

   //@}
   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{
   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /// Constructor.
   MpAudioDriverManager();

   /// Destructor.
   ~MpAudioDriverManager(void);

   /// Copy constructor (not implemented for this class)
   MpAudioDriverManager(const MpAudioDriverManager& rMpAudioDriverManager);

   /// Assignment operator (not implemented for this class)
   MpAudioDriverManager& operator=(const MpAudioDriverManager& rhs);

   /**
    * Returns TRUE if synchronous stream should be used for given host api.
    */
   static UtlBoolean useSynchronousStream(const UtlString& hostApiName);

   static OsMutex ms_mutex;
   static MpAudioDriverManager* ms_pInstance;

   MpAudioDriverBase* m_pAudioDriver; ///< pointer to audio driver
   MpAudioStreamId m_inputAudioStream; ///< ID if input audio stream
   MpAudioDeviceIndex m_inputDeviceIndex; ///< index of current input device
   MpAudioStreamId m_outputAudioStream; ///< ID of output audio stream
   MpAudioDeviceIndex m_outputDeviceIndex; ///< index of current output device

   MpAudioMixerBase* m_inputAudioMixer; ///< mixer for input stream
   MpAudioMixerBase* m_outputAudioMixer; ///< mixer for output stream

   double m_initialInputStreamLatency; ///< latency used when opening new stream in seconds
   double m_initialOutputStreamLatency; ///< latency used when opening new stream in seconds

   std::vector<MpAudioDeviceInfo> m_outputAudioDevices; ///< array to store device info
   std::vector<MpAudioDeviceInfo> m_inputAudioDevices; ///< array to store device info
};

#endif // MpAudioDriverManager_h__
