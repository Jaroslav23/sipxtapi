//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef CpAudioDeviceInfo_h__
#define CpAudioDeviceInfo_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>

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
* Class for holding information about audio device
*/
class CpAudioDeviceInfo
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Constructor.
   CpAudioDeviceInfo();

   /// Destructor.
   virtual ~CpAudioDeviceInfo(void);
   //@}

   /// Copy constructor
   CpAudioDeviceInfo(const CpAudioDeviceInfo& rCpAudioDeviceInfo);

   /// Assignment operator
   CpAudioDeviceInfo& operator=(const CpAudioDeviceInfo& rhs);

   UtlString m_deviceName; ///< device name
   UtlString m_driverName; ///< name of driver for this device

   int m_maxChannels; ///< maximum channels supported
   double m_defaultSampleRate; ///< default sample rate
   UtlBoolean m_bIsInput; ///< whether it is input device
};

#endif // CpAudioDeviceInfo_h__
