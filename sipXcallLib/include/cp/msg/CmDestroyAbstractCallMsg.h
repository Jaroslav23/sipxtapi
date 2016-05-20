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

#ifndef CmDestroyAbstractCallMsg_h__
#define CmDestroyAbstractCallMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <utl/UtlString.h>
#include <cp/CpMessageTypes.h>
#include <cp/msg/CmCommandMsg.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
* Call manager command message. Instructs call manager to destroy given call.
*/
class CmDestroyAbstractCallMsg : public CmCommandMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   CmDestroyAbstractCallMsg(const UtlString& sAbstractCallId);

   virtual ~CmDestroyAbstractCallMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   UtlString getAbstractCallId() const { return m_sAbstractCallId; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private copy constructor */
   CmDestroyAbstractCallMsg(const CmDestroyAbstractCallMsg& rMsg);

   /** Private assignment operator */
   CmDestroyAbstractCallMsg& operator=(const CmDestroyAbstractCallMsg& rhs);

   UtlString m_sAbstractCallId;
};

#endif // CmDestroyAbstractCallMsg_h__
