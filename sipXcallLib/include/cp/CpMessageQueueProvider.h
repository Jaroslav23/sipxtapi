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

#ifndef CpMessageQueueProvider_h__
#define CpMessageQueueProvider_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsMsgQ.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * CpMessageQueueProvider is an interface that needs to be implemented by class
 * that is capable of providing local call queue and global call manager queue
 * for message posting. 
 */
class CpMessageQueueProvider
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   /**
    * Gets local call queue for sending messages.
    */       
   virtual OsMsgQ& getLocalQueue() = 0;

   /**
    * Gets global queue for inter call communication.
    */       
   virtual OsMsgQ& getGlobalQueue() = 0;

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // CpMessageQueueProvider_h__
