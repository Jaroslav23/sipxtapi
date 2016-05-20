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

#ifndef AcTransferConsultativeMsg_h__
#define AcTransferConsultativeMsg_h__

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
* Abstract call command message. Instructs call to initiate consultative transfer on given connection.
*/
class AcTransferConsultativeMsg : public AcCommandMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   AcTransferConsultativeMsg(const SipDialog& sourceSipDialog,
                             const SipDialog& targetSipDialog);

   virtual ~AcTransferConsultativeMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   void getSourceSipDialog(SipDialog& val) const { val = m_sourceSipDialog; }
   void getTargetSipDialog(SipDialog& val) const { val = m_targetSipDialog; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private copy constructor */
   AcTransferConsultativeMsg(const AcTransferConsultativeMsg& rMsg);

   /** Private assignment operator */
   AcTransferConsultativeMsg& operator=(const AcTransferConsultativeMsg& rhs);

   SipDialog m_sourceSipDialog;
   SipDialog m_targetSipDialog;
};

#endif // AcTransferConsultativeMsg_h__
