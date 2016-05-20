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

#ifndef AcNotificationMsg_h__
#define AcNotificationMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <net/SipDialog.h>
#include <cp/CpMessageTypes.h>

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
class AcNotificationMsg : public OsMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   typedef enum
   {
      ACN_FIRST = 0,
      ACN_TUNNELED, ///< tunneled abstract call notification message. Payload will be another message.
      ACN_STARTED, ///< dispatched to abstract call after thread starts
   } SubTypesEnum;

   /* ============================ CREATORS ================================== */

   AcNotificationMsg(SubTypesEnum subType, const SipDialog& sipDialog);

   /** Copy constructor */
   AcNotificationMsg(const AcNotificationMsg& rhs);

   virtual ~AcNotificationMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   AcNotificationMsg& operator=(const AcNotificationMsg& rhs);

   /* ============================ ACCESSORS ================================= */

   void getSipDialog(SipDialog& val) { val = m_sipDialog; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   SipDialog m_sipDialog; ///< sip dialog of call this message should be sent to
};

#endif // AcNotificationMsg_h__
