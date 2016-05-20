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

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlRandom.h>
#include <net/SipMessage.h>
#include <cp/Cp100RelTracker.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/**
 * Private context class for 100rel messages. Can be used for both
 * inbound and outbound 100rels. Either m_bPrackSent or m_bPrackReceived
 * is valid.
 */
class Cp100RelContext : public UtlString
{
public:
   Cp100RelContext(int cSeqNumber, const UtlString& cSeqMethod, int rSeqNumber)
      : UtlString(Cp100RelTracker::get100RelId(cSeqNumber, cSeqMethod, rSeqNumber))
      , m_bPrackSent(FALSE)
      , m_bPrackReceived(FALSE)
      , m_bSdpIn100rel(FALSE)
   {

   }

   Cp100RelContext(const SipMessage& sipMessage)
      : UtlString(Cp100RelTracker::get100RelId(sipMessage))
      , m_bPrackSent(FALSE)
      , m_bPrackReceived(FALSE)
      , m_bSdpIn100rel(FALSE)
   {

   }

   UtlBoolean m_bPrackSent; ///< TRUE if PRACK was sent for 100rel identified by this Id
   UtlBoolean m_bPrackReceived; ///< TRUE if PRACK was received for 100rel identified by this Id
   UtlBoolean m_bSdpIn100rel; ///< TRUE if SDP was in 100rel response
};

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

Cp100RelTracker::Cp100RelTracker()
: m_bCanSend100rel(TRUE)
, m_rseqNumber(getRandomStartRSeq())
{

}

Cp100RelTracker::~Cp100RelTracker()
{
   m_100relBag.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

void Cp100RelTracker::reset()
{
   m_bCanSend100rel = TRUE;
   m_rseqNumber = getRandomStartRSeq();
   m_100relBag.destroyAll();
}

UtlBoolean Cp100RelTracker::on100RelReceived(const SipMessage& sipMessage)
{
   if (sipMessage.is100RelResponse())
   {
      // this response is reliable
      UtlString s100RelId(get100RelId(sipMessage));

      Cp100RelContext* pContext = dynamic_cast<Cp100RelContext*>(m_100relBag.find(&s100RelId));
      if (!pContext)
      {
         // if this 100rel was received 1st time, also create context for it
         pContext = new Cp100RelContext(sipMessage);
         m_100relBag.insert(pContext);
      }

      if (pContext && !pContext->m_bPrackSent)
      {
         pContext->m_bPrackSent = TRUE;
         pContext->m_bSdpIn100rel = sipMessage.getSdpBody() != NULL;
         return TRUE;
      }
   }

   return FALSE;
}

void Cp100RelTracker::on100RelSent(const SipMessage& sipMessage, UtlString& s100RelId)
{
   if (sipMessage.is100RelResponse())
   {
      m_bCanSend100rel = FALSE;
      // this response is reliable
      UtlString key100RelId(get100RelId(sipMessage));

      Cp100RelContext* pContext = dynamic_cast<Cp100RelContext*>(m_100relBag.find(&key100RelId));
      if (!pContext)
      {
         // add context to bag, so that we can identify valid PRACK later
         pContext = new Cp100RelContext(sipMessage);
         pContext->m_bSdpIn100rel = sipMessage.getSdpBody() != NULL;
         s100RelId = pContext->data(); // copy string
         m_100relBag.insert(pContext);
      }
   }
}

UtlBoolean Cp100RelTracker::onPrackReceived(const SipMessage& sipMessage)
{
   if (sipMessage.isPrackRequest())
   {
      UtlString s100RelId(get100RelId(sipMessage));
      Cp100RelContext* pContext = dynamic_cast<Cp100RelContext*>(m_100relBag.find(&s100RelId));
      if (pContext)
      {
         m_bCanSend100rel = TRUE;
         pContext->m_bPrackReceived = TRUE;
         return TRUE;
      }
   }

   return FALSE;
}

int Cp100RelTracker::getNextRSeq()
{
   return m_rseqNumber++;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

UtlBoolean Cp100RelTracker::canSend1xxRel() const
{
   return m_bCanSend100rel;
}

UtlBoolean Cp100RelTracker::are1xxRelsAcknowledged() const
{
   return m_bCanSend100rel;
}

UtlBoolean Cp100RelTracker::is100RelIdValid(const UtlString& s100RelId) const
{
   Cp100RelContext* pContext = dynamic_cast<Cp100RelContext*>(m_100relBag.find(&s100RelId));
   if (pContext)
   {
      return TRUE;
   }

   return FALSE;
}

UtlBoolean Cp100RelTracker::wasPrackReceived(const UtlString& s100RelId) const
{
   Cp100RelContext* pContext = dynamic_cast<Cp100RelContext*>(m_100relBag.find(&s100RelId));
   if (pContext && pContext->m_bPrackReceived)
   {
      return TRUE;
   }

   return FALSE;
}

UtlBoolean Cp100RelTracker::wasSdpBodyIn100rel(const UtlString& s100RelId) const
{
   Cp100RelContext* pContext = dynamic_cast<Cp100RelContext*>(m_100relBag.find(&s100RelId));
   if (pContext && pContext->m_bSdpIn100rel)
   {
      return TRUE;
   }

   return FALSE;
}

UtlString Cp100RelTracker::get100RelId(int cSeqNumber, const UtlString& cSeqMethod, int rSeqNumber)
{
   UtlString result;
   result.appendFormat("%d;%s;%d", cSeqNumber, cSeqMethod.data(), rSeqNumber);
   return result;
}

UtlString Cp100RelTracker::get100RelId(const SipMessage& sipMessage)
{
   UtlString s100RelId;

   if (sipMessage.is100RelResponse())
   {
      // this response is reliable
      int cSeqNumber;
      UtlString cSeqMethod;
      sipMessage.getCSeqField(cSeqNumber, cSeqMethod); // CSeq identifies transaction
      int rSeqNumber;
      sipMessage.getRSeqField(rSeqNumber); // RSeq identifies 100rel response. They all will have the same CSeq

      s100RelId = get100RelId(cSeqNumber, cSeqMethod, rSeqNumber);
   }
   else if (sipMessage.isPrackRequest())
   {
      // prack request
      int rSeqNumber;
      int cSeqNumber;
      UtlString cSeqMethod;
      sipMessage.getRAckField(rSeqNumber, cSeqNumber, cSeqMethod);

      s100RelId = get100RelId(cSeqNumber, cSeqMethod, rSeqNumber);
   }

   return s100RelId;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

void Cp100RelTracker::onTransactionStart(const UtlString& sipMethod, int cseqNum)
{
   if (sipMethod.compareTo(SIP_INVITE_METHOD) == 0)
   {
      // new INVITE transaction, reset tracker
      reset();
   }
}

void Cp100RelTracker::onTransactionEnd(const UtlString& sipMethod, int cseqNum)
{
   // do nothing
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

int Cp100RelTracker::getRandomStartRSeq()
{
   UtlRandom randomGenerator;
   return (abs(randomGenerator.rand()) % 65535);
}

/* ============================ FUNCTIONS ================================= */

