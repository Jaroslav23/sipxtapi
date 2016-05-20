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

#ifndef AcAcceptConnectionMsg_h__
#define AcAcceptConnectionMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <utl/UtlString.h>
#include <net/SipDialog.h>
#include <net/SipTransport.h>
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
* Abstract call command message. Instructs call to accept call connection.
*/
class AcAcceptConnectionMsg : public AcCommandMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   AcAcceptConnectionMsg(const SipDialog& sSipDialog,
                         UtlBoolean bSendSDP,
                         const UtlString& sLocationHeader,
                         CP_CONTACT_ID contactId,
                         SIP_TRANSPORT_TYPE transport);

   virtual ~AcAcceptConnectionMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   void getSipDialog(SipDialog& sSipDialog) const { sSipDialog = m_sSipDialog; }
   UtlBoolean getSendSDP() const { return m_bSendSDP; }
   UtlString getLocationHeader() const { return m_sLocationHeader; }
   CP_CONTACT_ID getContactId() const { return m_contactId; }
   SIP_TRANSPORT_TYPE getTransport() const { return m_transport; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private copy constructor */
   AcAcceptConnectionMsg(const AcAcceptConnectionMsg& rMsg);

   /** Private assignment operator */
   AcAcceptConnectionMsg& operator=(const AcAcceptConnectionMsg& rhs);

   SipDialog m_sSipDialog;
   UtlBoolean m_bSendSDP;
   UtlString m_sLocationHeader;
   CP_CONTACT_ID m_contactId;
   SIP_TRANSPORT_TYPE m_transport;
};

#endif // AcAcceptConnectionMsg_h__
