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

#ifndef ScReestablishRtpRedirectCommandMsg_h__
#define ScReestablishRtpRedirectCommandMsg_h__

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
* Sip connection command message. Instructs sip connection to reestablish RTP redirect.
* That means re-INVITE will be sent with updated SdpBody.
*/
class ScReestablishRtpRedirectCommandMsg : public ScCommandMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   ScReestablishRtpRedirectCommandMsg(const SipDialog& targetSipDialog,
                                      const SipDialog& sourceSipDialog,
                                      const SdpBody& sourceRemoteSdpBody);

   /** Copy constructor */
   ScReestablishRtpRedirectCommandMsg(const ScReestablishRtpRedirectCommandMsg& rMsg);

   virtual ~ScReestablishRtpRedirectCommandMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   ScReestablishRtpRedirectCommandMsg& operator=(const ScReestablishRtpRedirectCommandMsg& rhs);

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

#endif // ScReestablishRtpRedirectCommandMsg_h__
