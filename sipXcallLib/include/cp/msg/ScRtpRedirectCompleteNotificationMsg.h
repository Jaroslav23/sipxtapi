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

#ifndef ScRtpRedirectCompleteNotificationMsg_h__
#define ScRtpRedirectCompleteNotificationMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <net/SipDialog.h>
#include <net/SdpBody.h>
#include <cp/CpMessageTypes.h>
#include <cp/msg/ScNotificationMsg.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
* Sip connection notification message. Informs sip connection that rtp redirect was completed.
* Carries the latest remote SDP body.
*/
class ScRtpRedirectCompleteNotificationMsg : public ScNotificationMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   ScRtpRedirectCompleteNotificationMsg(const SipDialog& targetSipDialog,
                                        const SipDialog& sourceSipDialog,
                                        const SdpBody& sourceRemoteSdpBody);

   /** Copy constructor */
   ScRtpRedirectCompleteNotificationMsg(const ScRtpRedirectCompleteNotificationMsg& rMsg);

   virtual ~ScRtpRedirectCompleteNotificationMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   ScRtpRedirectCompleteNotificationMsg& operator=(const ScRtpRedirectCompleteNotificationMsg& rhs);

   /* ============================ ACCESSORS ================================= */

   void getSourceSipDialog(SipDialog& param) const { param = m_sourceSipDialog; }
   void getSourceRemoteSdpBody(SdpBody& param) const { param = m_sourceRemoteSdpBody; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

  SipDialog m_sourceSipDialog; ///< sip dialog of event sender
  SdpBody m_sourceRemoteSdpBody; ///< remote SDP body of the source (master) call
};

#endif // ScRtpRedirectCompleteNotificationMsg_h__
