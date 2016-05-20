//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef MpAudioMixerBase_h__
#define MpAudioMixerBase_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlBool.h>
#include "mp/MpAudioDriverDefs.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
class UtlString;

// STRUCTS
// TYPEDEFS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/**
* Base class for all audio mixers
*/
class MpAudioMixerBase
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Destructor.
   virtual ~MpAudioMixerBase(void);
   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

   /**
    * Returns number of mixers available for the same device this mixer
    * was created for.
    */
   virtual int getNumMixers() const = 0;

   /**
    * Returns name of mixer name for given mixer id.
    */
   virtual void getMixerName(UtlString& name, int i) const = 0;

   /**
   * Master (output) volume
   */
   virtual MpAudioVolume getMasterVolume() const = 0;
   virtual void setMasterVolume(MpAudioVolume volume) = 0;

   /**
   * Main output volume
   */
   virtual MpAudioVolume getPCMOutputVolume() const = 0;
   virtual void setPCMOutputVolume(MpAudioVolume volume) = 0;
   virtual UtlBoolean supportsPCMOutputVolume() const = 0;

   /**
   * All output volumes
   */
   virtual int getNumOutputVolumes() const = 0;
   virtual void getOutputVolumeName(UtlString& name, int i) const = 0;
   virtual MpAudioVolume getOutputVolume(int i) const = 0;
   virtual void setOutputVolume(int i, MpAudioVolume volume) = 0;

   /**
   * Input source
   */
   virtual int getNumInputSources() const = 0;
   virtual void getInputSourceName(UtlString& name, int i) const = 0;
   virtual int getCurrentInputSource() const = 0; /* may return -1 == none */
   virtual void setCurrentInputSource(int i) = 0;

   /**
   * Input volume
   */
   virtual MpAudioVolume getInputVolume() const = 0;
   virtual void setInputVolume(MpAudioVolume volume) = 0;

   /**
   * Balance
   */
   virtual UtlBoolean supportsOutputBalance() const = 0;
   virtual MpAudioBalance getOutputBalance() const = 0;
   virtual void setOutputBalance(MpAudioBalance balance) = 0;

   /**
   * Playthrough
   */
   virtual UtlBoolean supportsPlaythrough() const = 0;
   virtual MpAudioVolume getPlaythrough() const = 0;
   virtual void setPlaythrough(MpAudioVolume volume) = 0;

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
   MpAudioMixerBase();

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /// Copy constructor (not implemented for this class)
   MpAudioMixerBase(const MpAudioMixerBase& rMpAudioMixer);

   /// Assignment operator (not implemented for this class)
   MpAudioMixerBase& operator=(const MpAudioMixerBase& rhs);
};

#endif // MpAudioMixerBase_h__
