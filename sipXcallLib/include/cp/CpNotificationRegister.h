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

#ifndef CpNotificationRegister_h__
#define CpNotificationRegister_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlDefs.h>
#include <utl/UtlHashMap.h>
#include <cp/CpDefs.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS
class UtlSList;
class SipDialog;

/**
 * Register for inter connection notifications of different types.
 *
 * This class is not thread safe.
 */
class CpNotificationRegister
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /** Constructor */
   CpNotificationRegister();

   /** Destructor */
   ~CpNotificationRegister();

   /* ============================ MANIPULATORS ============================== */

   /**
   * Subscribe for given notification type with given target sip call.
   * ScNotificationMsg messages will be sent to callbackSipDialog.
   */
   OsStatus subscribe(CP_NOTIFICATION_TYPE notificationType,
                      const SipDialog& callbackSipDialog);

   /**
   * Unsubscribes from given notification type with given target sip call.
   * Unsubscription is done by SipDialog equality.
   */
   OsStatus unsubscribe(CP_NOTIFICATION_TYPE notificationType,
                        const SipDialog& callbackSipDialog);

   /**
    * Gets list of dialogs interested in given notification type. Returns
    * pointer to internal list, that should not be deleted. Returned list
    * should not be modified.
    * Returned list contains SipDialog objects.
    */
   const UtlSList* getSubscribedDialogs(CP_NOTIFICATION_TYPE notificationType) const;

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   mutable UtlHashMap m_register; ///< register for subscriptions. Key - UtlInt, value - UtlSList (of SipDialog)
};

#endif // CpNotificationRegister_h__
