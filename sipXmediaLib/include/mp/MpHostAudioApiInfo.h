//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef MpHostAudioApiInfo_h__
#define MpHostAudioApiInfo_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include "mp/MpAudioDriverDefs.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
// STRUCTS
// TYPEDEFS

typedef int MpHostAudioApiIndex;

typedef enum MpHostAudioApiTypeId
{
   MP_AUDIOAPI_INDEVELOPMENT = 0,
   MP_AUDIOAPI_DIRECTSOUND = 1,
   MP_AUDIOAPI_MME = 2,
   MP_AUDIOAPI_ASIO = 3,
   MP_AUDIOAPI_SOUNDMANAGER = 4,
   MP_AUDIOAPI_COREAUDIO = 5,
   MP_AUDIOAPI_OSS = 7,
   MP_AUDIOAPI_ALSA = 8,
   MP_AUDIOAPI_AL = 9,
   MP_AUDIOAPI_BEOS = 10,
   MP_AUDIOAPI_WDMKS = 11,
   MP_AUDIOAPI_JACK = 12,
   MP_AUDIOAPI_WASAPI = 13,
   MP_AUDIOAPI_AUDIOSCIENCEHPI = 14
} MpHostAudioApiTypeId;

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/**
* Class describing host audio API implemented by certain audio driver.
*/
class MpHostAudioApiInfo
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Constructor.
   MpHostAudioApiInfo();

   /// Constructor.
   MpHostAudioApiInfo(MpHostAudioApiTypeId typeId,
                      const UtlString& name,
                      int deviceCount,
                      MpAudioDeviceIndex defaultInputDevice,
                      MpAudioDeviceIndex defaultOutputDevice);

   /// Destructor.
   virtual ~MpHostAudioApiInfo(void);

   /// Copy constructor
   MpHostAudioApiInfo(const MpHostAudioApiInfo& rMpHostAudioApiInfo);

   /// Assignment operator
   MpHostAudioApiInfo& operator=(const MpHostAudioApiInfo& rhs);

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{
   //@}

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{

   MpHostAudioApiTypeId getTypeId() const { return m_typeId; }
   void setTypeId(MpHostAudioApiTypeId val) { m_typeId = val; }

   UtlString getName() const { return m_name; }
   void setName(const UtlString& val) { m_name = val; }

   int getDeviceCount() const { return m_deviceCount; }
   void setDeviceCount(int val) { m_deviceCount = val; }

   MpAudioDeviceIndex getDefaultInputDevice() const { return m_defaultInputDevice; }
   void setDefaultInputDevice(MpAudioDeviceIndex val) { m_defaultInputDevice = val; }

   MpAudioDeviceIndex getDefaultOutputDevice() const { return m_defaultOutputDevice; }
   void setDefaultOutputDevice(MpAudioDeviceIndex val) { m_defaultOutputDevice = val; }

   //@}
   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{
   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   MpHostAudioApiTypeId m_typeId; ///< unique identifier of host API
   UtlString m_name; ///< textual description of host api
   int m_deviceCount; ///< number of devices that are available to this host API
   MpAudioDeviceIndex m_defaultInputDevice; ///< index of default input device
   MpAudioDeviceIndex m_defaultOutputDevice; ///< index of default output device
};

#endif // MpHostAudioApiInfo_h__
