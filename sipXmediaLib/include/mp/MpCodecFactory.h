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


#ifndef _MpCodecFactory_h_
#define _MpCodecFactory_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsStatus.h"
#include "os/OsBSem.h"
#include "sdp/SdpCodec.h"
#include "mp/MpEncoderBase.h"
#include "mp/MpDecoderBase.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Singleton class used to generate encoder and decoder objects of a
//:an indicated type.
class MpCodecFactory
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
///@name Creators
//@{
   static MpCodecFactory* getMpCodecFactory(void);
     //:Return a pointer to the MpCodecFactory singleton object, creating 
     //:it if necessary

   virtual
   ~MpCodecFactory();
     //:Destructor

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
   OsStatus createDecoder(const SdpCodec& pSdpCodec, MpDecoderBase*& rpDecoder);
     //:Returns a new instance of a decoder of the indicated type
     //!param: pSdpCodec - (in) instance of SDP codec
     //!param: rpDecoder - (out) Reference to a pointer to the new decoder object

   OsStatus createEncoder(const SdpCodec& pSdpCodec, MpEncoderBase*& rpEncoder);
     //:Returns a new instance of an encoder of the indicated type
     //!param: pSdpCodec - (in) instance of SDP codec
     //!param: rpEncoder - (out) Reference to a pointer to the new encoder object

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

   MpCodecFactory();
     //:Constructor (called only indirectly via getMpCodecFactory())
     // We identify this as a protected (rather than a private) method so
     // that gcc doesn't complain that the class only defines a private
     // constructor and has no friends.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   // Static data members used to enforce Singleton behavior
   static MpCodecFactory sInstance;

   MpCodecFactory(const MpCodecFactory& rMpCodecFactory);
     //:Copy constructor (not supported)

   MpCodecFactory& operator=(const MpCodecFactory& rhs);
     //:Assignment operator (not supported)
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpCodecFactory_h_
