//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MprDejitter_h_
#define _MprDejitter_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsStatus.h"
#include "os/OsBSem.h"
#include "mp/MpRtpBuf.h"
#include <mp/MpJitterBufferBase.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SdpCodec;
class MpJitterBufferBase;

/// The "Dejitter" media processing resource
class MprDejitter
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum
   {
      MAX_PAYLOADS = 128, // dynamic payload id ends with 127
   };

/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MprDejitter();

     /// Destructor
   virtual
   ~MprDejitter();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

   OsStatus initJitterBuffers(const UtlSList& codecList);

     /// Add a buffer containing an incoming RTP packet to the dejitter pool
   OsStatus pushPacket(const MpRtpBufPtr &pRtp);
     /**<
     *  This method places the packet to the pool depending the modulo division
     *  value.
     *
     *  @return OS_SUCCESS on success
     *  @return OS_LIMIT_REACHED if too many codecs used in incoming RTP packets
     */

     /// Get next RTP packet, or NULL if none is available.
   MpRtpBufPtr pullPacket(int rtpPayloadType, JitterBufferResult& jbResult);
     /**<
     *  @note Significant change is that the downstream puller may NOT pull all
     *        the available packets at once. Instead it is paced according to
     *        the needs of the RTP payload type.
     *
     *  This buffer is the primary dejitter/reorder buffer for the internal
     *  codecs. Some codecs may do their own dejitter stuff too. But we can't
     *  eliminate this buffer because then out-of-order packets would just be
     *  dumped on the ground.
     *
     *  This buffer does NOT substitute silence packets. That is done in
     *  MpJitterBuffer called from MprDecode.
     *
     *  If packets arrive out of order, and the newer packet has already been
     *  pulled due to the size of the jitter buffer set by the codec, this
     *  buffer will NOT discard the out-of-order packet, but send it along
     *  anyway it is up to the codec to discard the packets it cannot use. This
     *  allows this JB to be a no-op buffer for when the commercial library is
     *  used.
     */

   /**
    * Notify all jitter buffers about new frame tick.
    */   
   void frameIncrement();
//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

     // Return number of packets in buffer for given payload type.
   int getBufferLength(int rtpPayloadType);

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   void freeJitterBuffers();

                  /// Mutual exclusion lock for internal data
   OsBSem        m_rtpLock;

   MpJitterBufferBase* m_jitterBufferArray[MAX_PAYLOADS]; ///< stores jitter buffers by payload type
   MpJitterBufferBase* m_jitterBufferList[MAX_PAYLOADS]; ///< stores jitter buffers incrementally

     /// Copy constructor (not implemented for this class)
   MprDejitter(const MprDejitter& rMprDejitter);

     /// Assignment operator (not implemented for this class)
   MprDejitter& operator=(const MprDejitter& rhs);
 
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprDejitter_h_
