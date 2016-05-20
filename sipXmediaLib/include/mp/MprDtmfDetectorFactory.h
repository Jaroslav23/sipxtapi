//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef MprDtmfDetectorFactory_h__
#define MprDtmfDetectorFactory_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <mp/MprDtmfDetectorBase.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * Factory for creating instances of MprDtmfDetectors.
 */
class MprDtmfDetectorFactory
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /* ============================ MANIPULATORS ============================== */

   /**
    * Creates instance of DTMF detector.
    */
   static MprDtmfDetectorBase* createDtmfDetector(const UtlString& rName, 
                                                  int samplesPerFrame,
                                                  int samplesPerSec);

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Constructor */
   MprDtmfDetectorFactory();

   /** Destructor */
   ~MprDtmfDetectorFactory();
};

#endif // MprDtmfDetectorFactory_h__
