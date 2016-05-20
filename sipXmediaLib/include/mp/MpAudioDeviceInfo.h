//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef MpAudioDeviceInfo_h__
#define MpAudioDeviceInfo_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include "mp/MpHostAudioApiInfo.h"

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
* Description
*/
class MpAudioDeviceInfo
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Constructor.
   MpAudioDeviceInfo();

  /// Destructor.
   virtual ~MpAudioDeviceInfo(void);

   /// Copy constructor
   MpAudioDeviceInfo(const MpAudioDeviceInfo& rMpAudioDeviceInfo);

   /// Assignment operator
   MpAudioDeviceInfo& operator=(const MpAudioDeviceInfo& rhs);

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{
   //@}

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{

   UtlString getName() const { return m_name; }
   void setName(const UtlString& val) { m_name = val; }

   MpHostAudioApiIndex getHostApi() const { return m_hostApi; }
   void setHostApi(MpHostAudioApiIndex val) { m_hostApi = val; }

   UtlString getHostApiName() const { return m_hostApiName; }
   void setHostApiName(const UtlString& val) { m_hostApiName = val; }

   int getMaxInputChannels() const { return m_maxInputChannels; }
   void setMaxInputChannels(int val) { m_maxInputChannels = val; }

   int getMaxOutputChannels() const { return m_maxOutputChannels; }
   void setMaxOutputChannels(int val) { m_maxOutputChannels = val; }

   double getDefaultLowInputLatency() const { return m_defaultLowInputLatency; }
   void setDefaultLowInputLatency(double val) { m_defaultLowInputLatency = val; }

   double getDefaultLowOutputLatency() const { return m_defaultLowOutputLatency; }
   void setDefaultLowOutputLatency(double val) { m_defaultLowOutputLatency = val; }

   double getDefaultHighInputLatency() const { return m_defaultHighInputLatency; }
   void setDefaultHighInputLatency(double val) { m_defaultHighInputLatency = val; }

   double getDefaultHighOutputLatency() const { return m_defaultHighOutputLatency; }
   void setDefaultHighOutputLatency(double val) { m_defaultHighOutputLatency = val; }

   double getDefaultSampleRate() const { return m_defaultSampleRate; }
   void setDefaultSampleRate(double val) { m_defaultSampleRate = val; }

   //@}
   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{
   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   UtlString m_name; ///< device name
   MpHostAudioApiIndex m_hostApi; ///< host api index this device belongs to (not type id)
   UtlString m_hostApiName; ///< textual description of host api

   int m_maxInputChannels; ///< maximum input channels supported
   int m_maxOutputChannels; ///< maximum output channels supported

   double m_defaultLowInputLatency; ///< latency for interactive performance
   double m_defaultLowOutputLatency;

   double m_defaultHighInputLatency; ///< latency for non-interactive applications
   double m_defaultHighOutputLatency;

   double m_defaultSampleRate;
};

#endif // MpAudioDeviceInfo_h__
