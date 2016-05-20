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

#ifndef CmCommandMsg_h__
#define CmCommandMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <cp/CpMessageTypes.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
* Call manager command message. Instructs call manager to carry out some action.
*/
class CmCommandMsg : public OsMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   typedef enum
   {
      CM_GAIN_FOCUS = 0,
      CM_YIELD_FOCUS,
      CM_DESTROY_ABSTRACT_CALL
   } SubTypesEnum;

   /* ============================ CREATORS ================================== */

   CmCommandMsg(SubTypesEnum subType);

   virtual ~CmCommandMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private copy constructor */
   CmCommandMsg(const CmCommandMsg& rMsg);

   /** Private assignment operator */
   CmCommandMsg& operator=(const CmCommandMsg& rhs);

};

#endif // CmCommandMsg_h__
