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

#ifndef AcTransferBlindMsg_h__
#define AcTransferBlindMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <utl/UtlString.h>
#include <net/SipDialog.h>
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
* Abstract call command message. Instructs call to initiate blind transfer on given connection.
*/
class AcTransferBlindMsg : public AcCommandMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   AcTransferBlindMsg(const SipDialog& sipDialog,
                      const UtlString& sTransferSipUrl);

   virtual ~AcTransferBlindMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   void getSipDialog(SipDialog& val) const { val = m_sipDialog; }
   UtlString getTransferSipUrl() const { return m_sTransferSipUrl; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private copy constructor */
   AcTransferBlindMsg(const AcTransferBlindMsg& rMsg);

   /** Private assignment operator */
   AcTransferBlindMsg& operator=(const AcTransferBlindMsg& rhs);

   SipDialog m_sipDialog;
   UtlString m_sTransferSipUrl;
};

#endif // AcTransferBlindMsg_h__
