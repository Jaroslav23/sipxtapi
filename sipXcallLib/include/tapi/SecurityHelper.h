//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
//
// Copyright (C) 2005-2007 SIPez LLC.
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

#ifndef SecurityHelper_h__
#define SecurityHelper_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
class SIPX_SECURITY_ATTRIBUTES;

// STRUCTS
// TYPEDEFS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

class SecurityHelper
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   /* ============================ CREATORS ================================== */

   /* ============================ MANIPULATORS ============================== */
   void generateSrtpKey(SIPX_SECURITY_ATTRIBUTES& securityAttrib);

   /* ============================ ACCESSORS ================================= */

   void setDbLocation(SIPX_SECURITY_ATTRIBUTES& securityAttrib, const char* dbLocation);
   void setMyCertNickname(SIPX_SECURITY_ATTRIBUTES& securityAttrib, const char* myCertNickname);
   void setDbPassword(SIPX_SECURITY_ATTRIBUTES& securityAttrib, const char* dbPassword);

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

#endif // SecurityHelper_h__
