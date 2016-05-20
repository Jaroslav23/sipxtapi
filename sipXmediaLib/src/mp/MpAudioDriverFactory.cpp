//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include <utl/UtlDefs.h>
#include "mp/MpAudioDriverFactory.h"
#include "mp/MpAudioDriverBase.h"
#include "mp/MpPortAudioDriver.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

MpAudioDriverBase* MpAudioDriverFactory::createAudioDriver(AudioDriverImplementation implementation)
{
   OsSysLog::add(FAC_AUDIO, PRI_DEBUG, "createAudioDriver implementation=%d", (int)implementation);

   switch(implementation)
   {
#ifndef DISABLE_LOCAL_AUDIO
   case AUDIO_DRIVER_PORTAUDIO:
      return MpPortAudioDriver::createInstance();
#endif
   default:
      // illegal implementation type
      return NULL;
   }
}

UtlString MpAudioDriverFactory::getDriverNameVersion(AudioDriverImplementation implementation)
{
   switch(implementation)
   {
#ifndef DISABLE_LOCAL_AUDIO
   case AUDIO_DRIVER_PORTAUDIO:
      {
         UtlString nameVersion = MpPortAudioDriver::ms_driverName + " " + MpPortAudioDriver::ms_driverVersion;
         return nameVersion;
      }
#endif
   default:
      // illegal implementation type
      return UtlString("Error, unknown audio driver name");
   }
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


MpAudioDriverFactory::MpAudioDriverFactory()
{

}

MpAudioDriverFactory::~MpAudioDriverFactory(void)
{

}


/* ============================ FUNCTIONS ================================= */
