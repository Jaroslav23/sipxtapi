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

#ifndef ScTimerMsg_h__
#define ScTimerMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <cp/msg/CpTimerMsg.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS
class SipDialog;

/**
 * ScTimerMsg represents message which gets sent when a timer fires in sipxcalllib.
 * The timer message is meant to be processed by sip connection state machine.
 *
 * This message carries sip call id, local tag and remote tag to be able to identify
 * the sip connection it is destined for when timer fires.
 *
 * Subclass for custom timer message.
 */
class ScTimerMsg : public CpTimerMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */
   typedef enum
   {
      PAYLOAD_TYPE_FIRST = 0, ///< Add your own payload ids here
      PAYLOAD_TYPE_100REL, ///< 100rel retransmit
      PAYLOAD_TYPE_2XX, ///< 2xx retransmit
      PAYLOAD_TYPE_DISCONNECT, ///< force disconnect request
      PAYLOAD_TYPE_REINVITE, ///< delayed re-INVITE requested
      PAYLOAD_TYPE_BYE_RETRY, ///< retrying BYE for inbound call (check if ACK was received, and then BYE)
      PAYLOAD_TYPE_SESSION_TIMEOUT_CHECK, ///< check for session timeout (session timer support)
      PAYLOAD_TYPE_INVITE_EXPIRATION, ///< check for invite expiration (some final response must arrive before timeout)
      PAYLOAD_TYPE_DELAYED_ANSWER, ///< delayed call answer action
   } PayloadTypeEnum;

   /**
    * Constructor.
    */
   ScTimerMsg(PayloadTypeEnum payloadType,
              const UtlString& sCallId,
              const UtlString& sLocalTag,
              const UtlString& sRemoteTag,
              UtlBoolean isFromLocal = TRUE);

   /** Copy constructor */
   ScTimerMsg(const ScTimerMsg& rhs);

   /** Create a copy of this msg object (which may be of a derived type) */
   virtual OsMsg* createCopy(void) const;

   /** Destructor. */
   virtual ~ScTimerMsg();

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   ScTimerMsg& operator=(const ScTimerMsg& rhs);

   /* ============================ ACCESSORS ================================= */

   ScTimerMsg::PayloadTypeEnum getPayloadType() const { return m_payloadType; }

   /** Gets SipDialog this message is destined for */
   void getSipDialog(SipDialog& sipDialog) const;

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   PayloadTypeEnum m_payloadType; ///< type of payload
   UtlString m_sCallId; ///< sip call id (not abstract call id)
   UtlString m_sLocalTag; ///< local tag from sip message
   UtlString m_sRemoteTag; ///< remote tag from sip message
   UtlBoolean m_isFromLocal; ///< TRUE if this dialog was initiated locally

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

#endif // ScTimerMsg_h__
