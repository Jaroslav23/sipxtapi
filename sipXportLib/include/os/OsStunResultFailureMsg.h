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

#ifndef OsStunResultFailureMsg_h__
#define OsStunResultFailureMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsStunResultMsg.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
* Sent when STUN request fails on a socket.
*/
class OsStunResultFailureMsg : public OsStunResultMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   OsStunResultFailureMsg(const UtlString& sAdapterName,
                          const UtlString& sLocalIp,
                          int localPort);

   virtual ~OsStunResultFailureMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private copy constructor */
   OsStunResultFailureMsg(const OsStunResultFailureMsg& rMsg);

   /** Private assignment operator */
   OsStunResultFailureMsg& operator=(const OsStunResultFailureMsg& rhs);
};

#endif // OsStunResultFailureMsg_h__
