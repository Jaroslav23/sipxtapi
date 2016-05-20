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

#ifndef AcDropAllConnectionsMsg_h__
#define AcDropAllConnectionsMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <utl/UtlString.h>
#include <cp/CpMessageTypes.h>
#include <cp/msg/AcCommandMsg.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
* Abstract call command message. Instructs call to hang up all active call connection.
*/
class AcDropAllConnectionsMsg : public AcCommandMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   AcDropAllConnectionsMsg(UtlBoolean bDestroyAbstractCall = FALSE);

   virtual ~AcDropAllConnectionsMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   UtlBoolean getDestroyAbstractCall() const { return m_bDestroyAbstractCall; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private copy constructor */
   AcDropAllConnectionsMsg(const AcDropAllConnectionsMsg& rMsg);

   /** Private assignment operator */
   AcDropAllConnectionsMsg& operator=(const AcDropAllConnectionsMsg& rhs);

   UtlBoolean m_bDestroyAbstractCall;
};

#endif // AcDropAllConnectionsMsg_h__
