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

#ifndef _MprToSpkr_h_
#define _MprToSpkr_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsStatus.h"
#include "os/OsNotification.h"
#include "os/OsMsgQ.h"
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

/**
*  @brief The "To Speaker" media processing resource.
*
*/
class MprToSpkr : public MpAudioResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprToSpkr(const UtlString& rName, int samplesPerFrame, int samplesPerSec, OsMsgQ *pEchoQ);

     /// Destructor
   virtual ~MprToSpkr();

   typedef enum {
      ATTEN_LOUDEST = 0,    ///< 0 dB, no attenuation
      ATTEN_QUIETEST = -6   ///< Please do not make this lower than -48
   } AttenValues;

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
   OsMsgQ* mpEchoQ;       ///< Audio data will be sent to this queue too.
                          ///< This queue should be connected to Echo
                          ///< Cancelation resource.

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame=80,
                                     int samplesPerSecond=8000);

     /// Copy constructor (not implemented for this class)
   MprToSpkr(const MprToSpkr& rMprToSpkr);

     /// Assignment operator (not implemented for this class)
   MprToSpkr& operator=(const MprToSpkr& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprToSpkr_h_
