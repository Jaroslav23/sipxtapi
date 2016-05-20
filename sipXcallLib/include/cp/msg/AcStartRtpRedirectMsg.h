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

#ifndef AcStartRtpRedirectMsg_h__
#define AcStartRtpRedirectMsg_h__

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
* Abstract call command message. Instructs call to start RTP redirect operation.
*/
class AcStartRtpRedirectMsg : public AcCommandMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   AcStartRtpRedirectMsg(const UtlString& slaveAbstractCallId, const SipDialog& slaveSipDialog);

   virtual ~AcStartRtpRedirectMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   void getSlaveAbstractCallId(UtlString& param) const { param = m_slaveAbstractCallId; }
   void getSlaveSipDialog(SipDialog& param) const { param = m_slaveSipDialog; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private copy constructor */
   AcStartRtpRedirectMsg(const AcStartRtpRedirectMsg& rMsg);

   /** Private assignment operator */
   AcStartRtpRedirectMsg& operator=(const AcStartRtpRedirectMsg& rhs);

   UtlString m_slaveAbstractCallId;
   SipDialog m_slaveSipDialog;
};

#endif // AcStartRtpRedirectMsg_h__
