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

#ifndef XCpCallControl_h__
#define XCpCallControl_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlDefs.h>
#include <net/SipTransport.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * XCpCallControl is meant to be used by calls/connections to control other calls.
 * This is needed in consultative call transfer.
 *
 * Methods of this interface must be executed from XCpAbstractCall thread, otherwise
 * deadlock might occur.
 */
class XCpCallControl
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /* ============================ MANIPULATORS ============================== */

   /**
   * Creates new call, and starts dialing. Allows to specify call state cause,
   * that will be used to fire CP_CALLSTATE_DIALTONE event.
   * @param sipDialog Output parameter, that will receive sip dialog details
   *        of created call.
   * @param pCallbackSipDialog Optional parameter. If present, then call state
   *        notifications will be sent to this call. To cancel notifications, use
   *        unsubscribe with sipDialog.
   */
   virtual OsStatus createConnectedCall(SipDialog& sipDialog,
                                        const UtlString& toAddress,
                                        const UtlString& fullLineUrl,// includes display name, SIP URI
                                        const UtlString& sSipCallId = NULL, // can be used to suggest sip call-id
                                        const UtlString& locationHeader = NULL,
                                        CP_CONTACT_ID contactId = AUTOMATIC_CONTACT_ID,
                                        SIP_TRANSPORT_TYPE transport = SIP_TRANSPORT_AUTO,
                                        CP_FOCUS_CONFIG focusConfig = CP_FOCUS_IF_AVAILABLE,
                                        const UtlString& replacesField = NULL, // value of Replaces INVITE field
                                        CP_CALLSTATE_CAUSE callstateCause = CP_CALLSTATE_CAUSE_NORMAL,
                                        const SipDialog* pCallbackSipDialog = NULL) = 0;

   /** Attempts to drop connection of some abstract call, for given sip dialog */
   virtual OsStatus dropAbstractCallConnection(const SipDialog& sSipDialog) = 0;

   /**
    * Sends message to specified call connection. If connection cannot be found,
    * OS_NOT_FOUND will be returned, and no more messages should be sent to the same
    * destination.
    */
   virtual OsStatus sendMessage(const OsMsg& msg, const SipDialog& sSipDialog) = 0;

   /**
    * Subscribe for given notification type with given target sip call.
    * ScNotificationMsg messages will be sent to callbackSipDialog.
    */
   virtual OsStatus subscribe(CP_NOTIFICATION_TYPE notificationType,
                              const SipDialog& targetSipDialog,
                              const SipDialog& callbackSipDialog) = 0;

   /**
    * Unsubscribes for given notification type with given target sip call.
    */
   virtual OsStatus unsubscribe(CP_NOTIFICATION_TYPE notificationType,
                                const SipDialog& targetSipDialog,
                                const SipDialog& callbackSipDialog) = 0;

   /* ============================ ACCESSORS ================================= */

   /** Checks if given call exists and is established. */
   virtual UtlBoolean isCallEstablished(const SipDialog& sipDialog) const = 0;

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // XCpCallControl_h__
