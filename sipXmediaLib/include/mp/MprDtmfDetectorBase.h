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

#ifndef _MprDtmfDetectorBase_h_
#define _MprDtmfDetectorBase_h_

// APPLICATION INCLUDES
#include "mp/MpFlowGraphMsg.h"
#include "mp/MpAudioResource.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class MpRtpInputAudioConnection;

/**
 *  Base class for all DTMF detectors. Implement doProcessFrame method from MpAudioResource. Subclass must also have
 *  MpRtpInputAudioConnection as friend.
 */
class MprDtmfDetectorBase : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   friend class MpRtpInputAudioConnection;

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     //:Constructor
   MprDtmfDetectorBase(const UtlString& rName, 
                       int samplesPerFrame,
                       int samplesPerSec)
   : MpAudioResource(rName, 0, 1, 1, 1, samplesPerFrame, samplesPerSec)
   {

   }

     //:Destructor
   virtual ~MprDtmfDetectorBase()
   {

   }

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

     /// Copy constructor (not implemented for this class)
   MprDtmfDetectorBase(const MprDtmfDetectorBase& rhs);

     /// Assignment operator (not implemented for this class)
   MprDtmfDetectorBase& operator=(const MprDtmfDetectorBase& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprDtmfDetectorBase_h_
