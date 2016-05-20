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

#ifndef ScStartRtpRedirectCommandMsg_h__
#define ScStartRtpRedirectCommandMsg_h__

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
* Sip connection command message. Instructs sip connection to start RTP redirect operation.
* Sent from master call to slave call. Carries remote SDP from master call to slave call.
*/
class ScStartRtpRedirectCommandMsg : public ScCommandMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   ScStartRtpRedirectCommandMsg(const SipDialog& targetSipDialog,
                                const SipDialog& sourceSipDialog,
                                const SdpBody& sourceRemoteSdpBody);

   /** Copy constructor */
   ScStartRtpRedirectCommandMsg(const ScStartRtpRedirectCommandMsg& rMsg);

   virtual ~ScStartRtpRedirectCommandMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   ScStartRtpRedirectCommandMsg& operator=(const ScStartRtpRedirectCommandMsg& rhs);

   /* ============================ ACCESSORS ================================= */

   void getSourceSipDialog(SipDialog& param) const { param = m_sourceSipDialog; }
   void getSourceRemoteSdpBody(SdpBody& param) const { param = m_sourceRemoteSdpBody; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   SipDialog m_sourceSipDialog; ///< sip dialog of call sending the command
   SdpBody m_sourceRemoteSdpBody; ///< remote SDP body of the source (master) call
};

#endif // ScStartRtpRedirectCommandMsg_h__
