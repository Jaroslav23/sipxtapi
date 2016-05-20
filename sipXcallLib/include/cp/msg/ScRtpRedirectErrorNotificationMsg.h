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

#ifndef ScRtpRedirectErrorNotificationMsg_h__
#define ScRtpRedirectErrorNotificationMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <net/SipDialog.h>
#include <cp/CpDefs.h>
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
* Sip connection notification message. Informs sip connection that start rtp redirect
* command was received, and carries slave connection remote SDP body.
*/
class ScRtpRedirectErrorNotificationMsg : public ScNotificationMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   ScRtpRedirectErrorNotificationMsg(const SipDialog& targetSipDialog,
                                     const SipDialog& sourceSipDialog,
                                     CP_RTP_REDIRECT_CAUSE cause,
                                     int responseCode,
                                     const UtlString& responseText);

   /** Copy constructor */
   ScRtpRedirectErrorNotificationMsg(const ScRtpRedirectErrorNotificationMsg& rMsg);

   virtual ~ScRtpRedirectErrorNotificationMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   ScRtpRedirectErrorNotificationMsg& operator=(const ScRtpRedirectErrorNotificationMsg& rhs);

   /* ============================ ACCESSORS ================================= */

   void getSourceSipDialog(SipDialog& param) const { param = m_sourceSipDialog; }
   CP_RTP_REDIRECT_CAUSE getCause() const { return m_cause; }
   int getResponseCode() const { return m_responseCode; }
   void getResponseText(UtlString& param) const { param = m_responseText; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

  SipDialog m_sourceSipDialog; ///< sip dialog of event sender
  CP_RTP_REDIRECT_CAUSE m_cause; ///< cause of error
  int m_responseCode; ///< SIP response code if available
  UtlString m_responseText; ///< textual description of SIP response code, if available
};

#endif // ScRtpRedirectErrorNotificationMsg_h__
