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

#ifndef ScStopRtpRedirectCommandMsg_h__
#define ScStopRtpRedirectCommandMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <cp/msg/ScCommandMsg.h>
#include <net/SipDialog.h>
#include <net/SdpBody.h>
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
* Sip connection command message. Instructs sip connection to stop RTP redirect operation.
*/
class ScStopRtpRedirectCommandMsg : public ScCommandMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   ScStopRtpRedirectCommandMsg(const SipDialog& targetSipDialog,
                               const SipDialog& sourceSipDialog);

   /** Copy constructor */
   ScStopRtpRedirectCommandMsg(const ScStopRtpRedirectCommandMsg& rMsg);

   virtual ~ScStopRtpRedirectCommandMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   ScStopRtpRedirectCommandMsg& operator=(const ScStopRtpRedirectCommandMsg& rhs);

   /* ============================ ACCESSORS ================================= */

   void getSourceSipDialog(SipDialog& param) const { param = m_sourceSipDialog; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   SipDialog m_sourceSipDialog; ///< sip dialog of call sending the command
};

#endif // ScStopRtpRedirectCommandMsg_h__
