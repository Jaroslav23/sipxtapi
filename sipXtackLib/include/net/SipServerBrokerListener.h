//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef SipServerBrokerListener_h__
#define SipServerBrokerListener_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <net/SipProtocolServerBase.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

enum EventSubTypes
{
   SIP_SERVER_BROKER_NOTIFY = 1
};

class SipServerBrokerListener : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   SipServerBrokerListener(SipProtocolServerBase* pOwner) :
      OsServerTask("SipTcpServerBrokerListener-%d", (void*)pOwner)
      , mpOwner(pOwner)
   {
      start();
   }

   virtual ~SipServerBrokerListener()
   {
      waitUntilShutDown();
   }

/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean handleMessage(OsMsg& rMsg);
   
/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   SipProtocolServerBase* mpOwner;
};

#endif // SipServerBrokerListener_h__
