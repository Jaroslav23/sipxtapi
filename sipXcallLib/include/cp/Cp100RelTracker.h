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

#ifndef CpReliable1xxTracker_h__
#define CpReliable1xxTracker_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlDefs.h>
#include <utl/UtlString.h>
#include <utl/UtlHashBag.h>
#include <cp/CpSipTransactionListener.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS
class SipMessage;

/**
 * Cp100RelTracker is Reliable Provisional Response tracker. This class keeps track
 * of sent provisional responses described in RFC3262. 1xx reliable response must be
 * retransmitted every T1 (500ms) up to 64 times.
 * 
 * This class doesn't send 1xx reliable responses or PRACKs. These need to be sent
 * by owner of this class. This class needs to be notified when 1xx is sent or
 * PRACK is received.
 *
 * Only 1 side can send PRACKs during INVITE transaction. One side normally sends 100rel
 * responses, and the other side PRACKs. This may reverse during re-INVITE.
 *
 * Tracking only one outbound unacknowledged 100rel response is supported. It is not
 * recommended by RFC3262 to send multiple unacknowledged 100rels.
 *
 * Validation of RSeq incrementation by 1 is not implemented for inbound 100rels.
 */
class Cp100RelTracker : public CpSipTransactionListener
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /** Constructor */
   Cp100RelTracker();

   /** Destructor */
   ~Cp100RelTracker();

   /* ============================ MANIPULATORS ============================== */

   /** Resets state of 1xx tracker. Call after INVITE transaction finishes */
   void reset();

   /**
    * Notifies tracker that 1xx reliable message was received. It returns
    * TRUE if a PRACK needs to be sent. We may get multiple retransmissions
    * of 100rel, but send PRACK only once.
    */
   UtlBoolean on100RelReceived(const SipMessage& sipMessage);

   /**
    * Notifies tracker than 1xx reliable response was sent by us. This makes
    * it possible to check when we receive PRACK, if this PRACK is valid.
    *
    * Returned s100RelId can be used to check if PRACK was already received.
    */
   void on100RelSent(const SipMessage& sipMessage, UtlString& s100RelId);

   /** Returns TRUE if given PRACK is valid - responds to our 100rel */
   UtlBoolean onPrackReceived(const SipMessage& sipMessage);

   /** Gets the next RSeq number */
   int getNextRSeq();

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /**
    * Returns TRUE if we may send 1xx reliable message. This method uses
    * very simple logic. To make it work, on100RelSent and onPrackReceived
    * must be called.
    */
   UtlBoolean canSend1xxRel() const;

   /** Returns TRUE if all sent reliable provisional responses have been acknowledged */
   UtlBoolean are1xxRelsAcknowledged() const;

   /**
    * Returns TRUE if 100RelId is valid - is being tracked.
    */
   UtlBoolean is100RelIdValid(const UtlString& s100RelId) const;

   /**
    * Returns TRUE if PRACK was received for given 100rel Id. That means retransmission
    * of 100rel can stop.
    */
   UtlBoolean wasPrackReceived(const UtlString& s100RelId) const;

   /** Returns TRUE if there was SDP body in 100rel response */
   UtlBoolean wasSdpBodyIn100rel(const UtlString& s100RelId) const;

   /** Constructs Id for 100rel tracking */
   static UtlString get100RelId(int cSeqNumber, const UtlString& cSeqMethod, int rSeqNumber);

   /** Constructs 100rel Id from 100rel response or PRACK request */
   static UtlString get100RelId(const SipMessage& sipMessage);

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /** Called when new transaction is started. */
   virtual void onTransactionStart(const UtlString& sipMethod, int cseqNum);

   /** Called when transaction is stopped. */
   virtual void onTransactionEnd(const UtlString& sipMethod, int cseqNum);

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Generates random starting RSeq */
   static int getRandomStartRSeq();

   UtlBoolean m_bCanSend100rel; ///< TRUE if we may send 100rel response
   UtlHashBag m_100relBag; ///< bag for tracking 100rel responses and PRACKs
   int m_rseqNumber; ///< current available RSeq number
};

#endif // CpReliable1xxTracker_h__
