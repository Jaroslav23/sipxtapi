//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2006 ProfitFuel Inc.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// $$
///////////////////////////////////////////////////////////////////////////////
#ifndef _MprSpeexPreprocess_h_
#define _MprSpeexPreprocess_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpAudioResource.h"
#include "mp/MpFlowGraphMsg.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
struct SpeexPreprocessState_;
typedef struct SpeexPreprocessState_ SpeexPreprocessState;

struct SpeexEchoState_;
typedef struct SpeexEchoState_ SpeexEchoState;

/// The "Speex Audio Preprocessor" media processing resource
/**
*  This resource is a wrapper over Speex's audio preprocessor. It is used
*  to do Automatic Gain Control, denoising and echo residue removal.
*  
*  MprSpeexPreprocess expects audio data on the first input and echo residue
*  power spectrum on the second input and produces processed audio on its
*  first output. Echo residue power spectrum could be get from second output
*  of MprSpeexEchoCancel resource.
*/
class MprSpeexPreprocess : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

     /// Constructor
   MprSpeexPreprocess(const UtlString& rName,
                      int samplesPerFrame, int samplesPerSec);

     /// Destructor
   virtual
   ~MprSpeexPreprocess();


/* ============================ MANIPULATORS ============================== */

     /// Enable or disable Automatic Gain Control
   UtlBoolean setAGC(UtlBoolean enable);

     /// Enable or disable noise reduction
   UtlBoolean setNoiseReduction(UtlBoolean enable);

   /// Enable or disable voice activity detection
   UtlBoolean setVAD(UtlBoolean enable);

   /// Attach echo canceller to preprocessor. Should only be called
   /// once, before flowgraph is started. Not thread safe.
   UtlBoolean attachEchoCanceller(SpeexEchoState* pEchoState);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   typedef enum
   {
      SET_AGC  = MpFlowGraphMsg::RESOURCE_SPECIFIC_START,
      SET_NOISE_REDUCTION,
      SET_VAD
   } AddlMsgTypes;

   SpeexPreprocessState *mpPreprocessState; ///< Structure containing internal
                                            ///<  state of Speex preprocessor.
   UtlBoolean m_bVadEnabled; ///< TRUE if voice activity detection is enabled

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame=80,
                                     int samplesPerSecond=8000);

     /// Handle messages for this resource.
   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);

     /// Handle the @link MprSpeexPreprocess::SET_AGC SET_AGC @endlink message.
   UtlBoolean handleSetAGC(UtlBoolean enable);

     /// Handle the @link MprSpeexPreprocess::SET_NOISE_REDUCTION SET_NOISE_REDUCTION @endlink message.
   UtlBoolean handleSetNoiseReduction(UtlBoolean enable);

   UtlBoolean handleSetVAD(UtlBoolean enable);

     /// Copy constructor (not implemented for this class)
   MprSpeexPreprocess(const MprSpeexPreprocess& rMprSpeexPreprocess);

     /// Assignment operator (not implemented for this class)
   MprSpeexPreprocess& operator=(const MprSpeexPreprocess& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprSpeexPreprocess_h_