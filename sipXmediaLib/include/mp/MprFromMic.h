//  
// Copyright (C) 2006-2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _MprFromMic_h_
#define _MprFromMic_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsMsgQ.h"
#include <mp/MpDefs.h>
#include "mp/MpAudioResource.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*  @brief The "From Microphone" media processing resource
*/
class MprFromMic : public MpAudioResource
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   /// Constructor
   MprFromMic(const UtlString& rName, int samplesPerFrame, int samplesPerSec);

   /// Destructor
   virtual ~MprFromMic();

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

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
   OsMsgQ *mpMicQ;                ///< We will read audio data from this queue.
   int m_framesProcessed;

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
      MpBufPtr outBufs[],
      int inBufsSize,
      int outBufsSize,
      UtlBoolean isEnabled,
      int samplesPerFrame=80,
      int samplesPerSecond=8000);

   /// Copy constructor (not implemented for this class)
   MprFromMic(const MprFromMic& rMprFromMic);

   /// Assignment operator (not implemented for this class)
   MprFromMic& operator=(const MprFromMic& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprFromMic_h_
