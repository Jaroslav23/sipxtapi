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

#ifndef AcMuteInputConnectionMsg_h__
#define AcMuteInputConnectionMsg_h__

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
* Abstract call command message. Instructs call to mute inbound RTP connection on bridge.
*/
class AcMuteInputConnectionMsg : public AcCommandMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   AcMuteInputConnectionMsg(const SipDialog& sipDialog);

   virtual ~AcMuteInputConnectionMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   void getSipDialog(SipDialog& val) const { val = m_sipDialog; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private copy constructor */
   AcMuteInputConnectionMsg(const AcMuteInputConnectionMsg& rMsg);

   /** Private assignment operator */
   AcMuteInputConnectionMsg& operator=(const AcMuteInputConnectionMsg& rhs);

   SipDialog m_sipDialog;
};

#endif // AcMuteInputConnectionMsg_h__
