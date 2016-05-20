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

#ifndef CmGainFocusMsg_h__
#define CmGainFocusMsg_h__

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
* Call manager command message. Instructs call manager to gain focus for given call.
*/
class CmGainFocusMsg : public CmCommandMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   CmGainFocusMsg(const UtlString& sAbstractCallId, UtlBoolean bGainOnlyIfNoFocusedCall = FALSE);

   virtual ~CmGainFocusMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   UtlString getAbstractCallId() const { return m_sAbstractCallId; }

   UtlBoolean getGainOnlyIfNoFocusedCall() const { return m_bGainOnlyIfNoFocusedCall; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private copy constructor */
   CmGainFocusMsg(const CmGainFocusMsg& rMsg);

   /** Private assignment operator */
   CmGainFocusMsg& operator=(const CmGainFocusMsg& rhs);

   UtlString m_sAbstractCallId;
   UtlBoolean m_bGainOnlyIfNoFocusedCall;
};

#endif // CmGainFocusMsg_h__
