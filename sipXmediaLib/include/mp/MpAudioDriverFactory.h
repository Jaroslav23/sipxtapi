//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef MpAudioDriverFactory_h__
#define MpAudioDriverFactory_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
class MpAudioDriverBase;

// STRUCTS
// TYPEDEFS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/**
* Description
*/
class MpAudioDriverFactory
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /**
    * Enumeration of all supported drivers. AUDIO_DRIVER_LAST is for iteration
    * of all available drivers in user code.
    */
   enum AudioDriverImplementation
   {
      AUDIO_DRIVER_PORTAUDIO = 0,
      AUDIO_DRIVER_LAST
   };

   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{
   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

   /**
    * Creates audio driver implementation. 
    *
    * @returns instance if it can be created or NULL if error occurred - i.e no
    *          more instances are allowed
    */
   static MpAudioDriverBase* createAudioDriver(AudioDriverImplementation implementation);

   /**
    * Used to get driver name and version in a string.
    *
    * @returns String containing name and version of given driver implementation.
    */
   static UtlString getDriverNameVersion(AudioDriverImplementation implementation);

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

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /// Private constructor.
   MpAudioDriverFactory();

   /// Private destructor.
   ~MpAudioDriverFactory(void);

   /// Copy constructor (not implemented for this class)
   MpAudioDriverFactory(const MpAudioDriverFactory& rMpAudioDriverFactory);

   /// Assignment operator (not implemented for this class)
   MpAudioDriverFactory& operator=(const MpAudioDriverFactory& rhs);
};

#endif // MpAudioDriverFactory_h__
