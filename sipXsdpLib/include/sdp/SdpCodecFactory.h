//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _SdpCodecFactory_h_
#define _SdpCodecFactory_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <sdp/SdpCodec.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
* Factory for SdpCodec instances.
*/
class SdpCodecFactory
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /* ============================ MANIPULATORS ============================== */

   /** Builds SdpCodec instance from given codec type */
   static SdpCodec* buildSdpCodec(SdpCodec::SdpCodecTypes codecType);

   /* ============================ ACCESSORS ================================= */

   /** Checks if audio/telephone-event is present, and if not adds it */
   static UtlString getFixedAudioCodecs(const UtlString& audioCodecs);

   /**
    * Enables announcement of VAD support for certain codecs like G.729.
    * There is currently no other way to enable/disable VAD in SDP for some
    * codecs using static payload format.
    */
   static void enableCodecVAD(UtlBoolean bEnable);

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   static UtlBoolean ms_bEnableCodecVAD; ///< when true then VAD support will be announced in SDP

   /** Default constructor */
   SdpCodecFactory();

   /** Copy constructor */
   SdpCodecFactory(const SdpCodecFactory& rSdpCodecFactory);

   /** Assignment operator */
   SdpCodecFactory& operator=(const SdpCodecFactory& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SdpCodecFactory_h_
