//
// Copyright (C) 2007 Jaroslav Libak
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef MpAudioStreamParameters_h__
#define MpAudioStreamParameters_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpAudioDriverDefs.h"

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
* Container for checking supported audio quality
*/
class MpAudioStreamParameters
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Constructor.
   MpAudioStreamParameters();

   /// Constructor.
   MpAudioStreamParameters(MpAudioDeviceIndex deviceIndex,
                           int channelCount,
                           MpAudioDriverSampleFormat sampleFormat,
                           double suggestedLatency);

   /// Destructor.
   virtual ~MpAudioStreamParameters(void);
   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{
   //@}

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{

   MpAudioDeviceIndex getDeviceIndex() const { return m_deviceIndex; }
   void setDeviceIndex(MpAudioDeviceIndex val) { m_deviceIndex = val; }

   int getChannelCount() const { return m_channelCount; }
   void setChannelCount(int val) { m_channelCount = val; }

   MpAudioDriverSampleFormat getSampleFormat() const { return m_sampleFormat; }
   void setSampleFormat(MpAudioDriverSampleFormat val) { m_sampleFormat = val; }

   double getSuggestedLatency() const { return m_suggestedLatency; }
   void setSuggestedLatency(double val) { m_suggestedLatency = val; }

   //@}
   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{
   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   MpAudioDeviceIndex m_deviceIndex; ///< index of audio device we check quality for
   int m_channelCount; ///< requested number of channels
   MpAudioDriverSampleFormat m_sampleFormat; ///< requested sample format @see MpAudioDriverDefs.h
   double m_suggestedLatency; ///< requested latency in seconds
};

#endif // MpAudioStreamParameters_h__
