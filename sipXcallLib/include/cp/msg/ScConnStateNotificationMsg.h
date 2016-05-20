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

#ifndef ScConnStateNotificationMsg_h__
#define ScConnStateNotificationMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <net/SipDialog.h>
#include <cp/CpMessageTypes.h>
#include <cp/msg/ScNotificationMsg.h>
#include <cp/state/ISipConnectionState.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
* Sip connection notification message. Informs sip connection about other connection
* state change.
*/
class ScConnStateNotificationMsg : public ScNotificationMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   ScConnStateNotificationMsg(ISipConnectionState::StateEnum state,
                              const SipDialog& targetSipDialog,
                              const SipDialog& sourceSipDialog);

   /** Copy constructor */
   ScConnStateNotificationMsg(const ScConnStateNotificationMsg& rMsg);

   virtual ~ScConnStateNotificationMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   ScConnStateNotificationMsg& operator=(const ScConnStateNotificationMsg& rhs);

   /* ============================ ACCESSORS ================================= */

   ISipConnectionState::StateEnum getState() const { return m_state; }
   void getSourceSipDialog(SipDialog& val) const { val = m_sourceSipDialog; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

  ISipConnectionState::StateEnum m_state; ///< new connection state
  SipDialog m_sourceSipDialog; ///< sip dialog of event sender
};

#endif // ScConnStateNotificationMsg_h__
