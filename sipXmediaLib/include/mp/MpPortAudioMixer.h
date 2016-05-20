//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef MpPortAudioMixer_h__
#define MpPortAudioMixer_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpAudioDriverDefs.h"
#include "mp/MpAudioMixerBase.h"
#include <portmixer.h>

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
* Mixer for PortAudio driver. Must be destroyed before its audio stream
* is closed.
*/
class MpPortAudioMixer : public MpAudioMixerBase
{
   friend class MpPortAudioDriver;

   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Destructor.
   virtual ~MpPortAudioMixer(void);

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

   /**
   * Returns number of mixers available for the same device this mixer
   * was created for.
   */
   virtual int getNumMixers() const;

   /**
   * Returns name of mixer name for given mixer id.
   */
   virtual void getMixerName(UtlString& name, int i) const;

   /**
   * Master (output) volume
   */
   virtual MpAudioVolume getMasterVolume() const;
   virtual void setMasterVolume(MpAudioVolume volume);

   /**
   * Main output volume
   */
   virtual MpAudioVolume getPCMOutputVolume() const;
   virtual void setPCMOutputVolume(MpAudioVolume volume);
   virtual UtlBoolean supportsPCMOutputVolume() const;

   /**
   * All output volumes
   */
   virtual int getNumOutputVolumes() const;
   virtual void getOutputVolumeName(UtlString& name, int i) const;
   virtual MpAudioVolume getOutputVolume(int i) const;
   virtual void setOutputVolume(int i, MpAudioVolume volume);

   /**
   * Input source
   */
   virtual int getNumInputSources() const;
   virtual void getInputSourceName(UtlString& name, int i) const;
   virtual int getCurrentInputSource() const; /* may return -1 == none */
   virtual void setCurrentInputSource(int i);

   /**
   * Input volume
   */
   virtual MpAudioVolume getInputVolume() const;
   virtual void setInputVolume(MpAudioVolume volume);

   /**
   * Balance
   */
   virtual UtlBoolean supportsOutputBalance() const;
   virtual MpAudioBalance getOutputBalance() const;
   virtual void setOutputBalance(MpAudioBalance balance);

   /**
   * Playthrough
   */
   virtual UtlBoolean supportsPlaythrough() const;
   virtual MpAudioVolume getPlaythrough() const;
   virtual void setPlaythrough(MpAudioVolume volume);

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

   /// Constructor.
   MpPortAudioMixer();

   /// Copy constructor (not implemented for this class)
   MpPortAudioMixer(const MpPortAudioMixer& rMpAudioMixer);

   /// Assignment operator (not implemented for this class)
   MpPortAudioMixer& operator=(const MpPortAudioMixer& rhs);

   static MpPortAudioMixer* createMixer(MpAudioStreamId stream,
                                        int mixerIndex);

   PxMixer* m_pxMixer;
};

#endif // MpPortAudioMixer_h__
