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

#ifndef AcConnectMsg_h__
#define AcConnectMsg_h__

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
* Abstract call command message. Instructs call to start dialing.
*/
class AcConnectMsg : public AcCommandMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   AcConnectMsg(const UtlString& sSipCallId,
                const UtlString& sToAddress,
                const UtlString& sLocalTag,
                const UtlString& sFromAddress,
                const UtlString& sLocationHeader,
                CP_CONTACT_ID contactId,
                SIP_TRANSPORT_TYPE transport,
                const UtlString& replacesField,
                CP_CALLSTATE_CAUSE callstateCause,
                const SipDialog* pCallbackSipDialog);

   virtual ~AcConnectMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   UtlString getSipCallId() const { return m_sSipCallId; }
   UtlString getToAddress() const { return m_sToAddress; }
   UtlString getLocalTag() const { return m_sLocalTag; }
   UtlString getFromAddress() const { return m_sFromAddress; }
   UtlString getLocationHeader() const { return m_sLocationHeader; }
   CP_CONTACT_ID getContactId() const { return m_contactId; }
   SIP_TRANSPORT_TYPE getTransport() const { return m_transport; }
   UtlString getReplacesField() const { return m_replacesField; }
   CP_CALLSTATE_CAUSE getCallstateCause() const { return m_callstateCause; }
   const SipDialog* getCallbackSipDialog() const { return m_pCallbackSipDialog; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private copy constructor */
   AcConnectMsg(const AcConnectMsg& rMsg);

   /** Private assignment operator */
   AcConnectMsg& operator=(const AcConnectMsg& rhs);

   UtlString m_sSipCallId;
   UtlString m_sToAddress;
   UtlString m_sLocalTag;
   UtlString m_sFromAddress;
   UtlString m_sLocationHeader;
   CP_CONTACT_ID m_contactId;
   SIP_TRANSPORT_TYPE m_transport;
   UtlString m_replacesField;
   CP_CALLSTATE_CAUSE m_callstateCause;
   SipDialog* m_pCallbackSipDialog;
};

#endif // AcConnectMsg_h__
