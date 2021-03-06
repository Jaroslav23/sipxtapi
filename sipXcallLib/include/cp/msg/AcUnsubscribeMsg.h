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

#ifndef AcUnsubscribeMsg_h__
#define AcUnsubscribeMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <utl/UtlString.h>
#include <net/SipDialog.h>
#include <cp/CpDefs.h>
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
* Abstract call command message. Unsubscribes connection from receiving notifications.
*/
class AcUnsubscribeMsg : public AcCommandMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   AcUnsubscribeMsg(CP_NOTIFICATION_TYPE notificationType,
                    const SipDialog& targetSipDialog,
                    const SipDialog& callbackSipDialog);

   virtual ~AcUnsubscribeMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   CP_NOTIFICATION_TYPE getNotificationType() const { return m_notificationType; }
   void getTargetSipDialog(SipDialog& val) const { val = m_targetSipDialog; }
   void getCallbackSipDialog(SipDialog& val) const { val = m_callbackSipDialog; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private copy constructor */
   AcUnsubscribeMsg(const AcUnsubscribeMsg& rMsg);

   /** Private assignment operator */
   AcUnsubscribeMsg& operator=(const AcUnsubscribeMsg& rhs);

   CP_NOTIFICATION_TYPE m_notificationType; ///< type of notification to subscribe
   SipDialog m_targetSipDialog; ///< dialog this message is destined for
   SipDialog m_callbackSipDialog; ///< dialog for sending notifications
};

#endif // AcUnsubscribeMsg_h__
