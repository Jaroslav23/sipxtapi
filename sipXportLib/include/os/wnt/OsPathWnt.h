//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsPathWnt_h_
#define _OsPathWnt_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsPathBase.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsPathBase;
class UtlString;

//:OS generic path class.  Will massage any input string so separators are correct.
//:Also provided functions to 
class OsPathWnt : public OsPathBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:


/* ============================ CREATORS ================================== */

   OsPathWnt();
     //:Default constructor

   OsPathWnt(const OsPathWnt& rOsPathWnt);
     //:Copy constructor

   virtual
   ~OsPathWnt();
     //:Destructor

   OsPathWnt(const UtlString& rPath);
     //: Copy contructor

   OsPathWnt(const char* pPath);
     //: Construct OsPathWnt from char* 
    
   OsPathWnt(const UtlString& rVolume, const UtlString& rDirName, const UtlString& rFileName, 
           const UtlString& rExtension);
     //: Forms a OsPathWnt from discrete parts 

/* ============================ MANIPULATORS ============================== */

    OsPathWnt& operator=(const OsPathWnt& rhs);
      //:Assignment operator

/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsPathWnt_h_


