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

#ifndef AcTunneledNotificationMsg_h__
#define AcTunneledNotificationMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <net/SipDialog.h>
#include <cp/CpMessageTypes.h>
#include <cp/msg/AcNotificationMsg.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
* Abstract call notification message. Informs call about some event. 
*/
class AcTunneledNotificationMsg : public AcNotificationMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   AcTunneledNotificationMsg(const OsMsg& msg, const SipDialog& sipDialog);

   /** Copy constructor */
   AcTunneledNotificationMsg(const AcTunneledNotificationMsg& rhs);

   virtual ~AcTunneledNotificationMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   AcTunneledNotificationMsg& operator=(const AcTunneledNotificationMsg& rhs);

   /* ============================ ACCESSORS ================================= */

   const OsMsg* getMsg() const { return m_pMsg; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsMsg* m_pMsg; ///< inner message, that should be processed in target call
};

#endif // AcTunneledNotificationMsg_h__
