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

#ifndef AcConferenceSplitMsg_h__
#define AcConferenceSplitMsg_h__

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
* Conference command message. Instructs conference to split sip connection with given sip dialog
* into a new call.
*/
class AcConferenceSplitMsg : public AcCommandMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   AcConferenceSplitMsg(const SipDialog& sipDialog, const UtlString& newCallId);

   virtual ~AcConferenceSplitMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   void getSipDialog(SipDialog& sipDialog) const { sipDialog = m_sipDialog; }

   void getNewCallId(UtlString& callId) const { callId = m_newCallId; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private copy constructor */
   AcConferenceSplitMsg(const AcConferenceSplitMsg& rMsg);

   /** Private assignment operator */
   AcConferenceSplitMsg& operator=(const AcConferenceSplitMsg& rhs);

   SipDialog m_sipDialog;
   UtlString m_newCallId;
};

#endif // AcConferenceSplitMsg_h__
