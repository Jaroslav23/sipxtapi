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
#include <utl/UtlInt.h>
#include <utl/UtlHashBag.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/UtlHashBagIterator.h>
#include <net/SipMessage.h>
#include <cp/CpSipTransactionManager.h>

// DEFINES
#define CLEAN_PERIOD 50 // in seconds
const int maxTransactionStorageTime = T1_PERIOD_MSEC * 64; // invite/non-invite transaction timeout

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

class CpTransactionState : public UtlInt
{
public:
   CpTransactionState(const UtlString& sipMethod, int cseqNum)
      : UtlInt(cseqNum)
      , m_sipMethod(sipMethod)
      , m_transactionState(CpSipTransactionManager::TRANSACTION_ACTIVE)
      , m_lastUsageTime(0)
      , m_pCookie(NULL)
   {
      m_lastUsageTime = OsDateTime::getSecsSinceEpoch();
   }

   CpSipTransactionManager::TransactionState m_transactionState;
   UtlString m_sipMethod; ///< sip method
   long m_lastUsageTime; ///< start time in seconds since 1/1/1970
   void* m_pCookie; ///< custom data stored with transaction
};

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

CpSipTransactionManager::CpSipTransactionManager()
: m_lastCleanUpTime(0)
, m_iInviteCSeq(-1)
, m_inviteTransactionState(CpSipTransactionManager::INVITE_INACTIVE)
, m_pTransactionListener(NULL)
, m_pInviteCookie(NULL)
{   
}

CpSipTransactionManager::~CpSipTransactionManager()
{
   m_pTransactionListener = NULL;
   m_transactionMap.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean CpSipTransactionManager::startTransaction(const UtlString& sipMethod, int cseqNum)
{
   if (sipMethod.compareTo(SIP_INVITE_METHOD) != 0)
   {
      // non INVITE transaction
      UtlInt utlCSeq(cseqNum);
      CpTransactionState* pTransactionState = dynamic_cast<CpTransactionState*>(m_transactionMap.findValue(&utlCSeq));
      if (!pTransactionState)
      {
         m_transactionMap.insertKeyAndValue(new UtlInt(cseqNum), new CpTransactionState(sipMethod, cseqNum)); // if its not already there then add it
         notifyTransactionStart(sipMethod, cseqNum);
         return TRUE;
      }
   }
   else
   {
      return startInitialInviteTransaction(cseqNum);
   }

   return FALSE;
}

UtlBoolean CpSipTransactionManager::startInitialInviteTransaction(int cseqNum)
{
   if (m_inviteTransactionState == CpSipTransactionManager::INVITE_INACTIVE)
   {
      m_inviteTransactionState = CpSipTransactionManager::INITIAL_INVITE_ACTIVE;
      m_iInviteCSeq = cseqNum;
      m_pInviteCookie = NULL;
      notifyTransactionStart(SIP_INVITE_METHOD, cseqNum);
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}

UtlBoolean CpSipTransactionManager::startReInviteTransaction(int cseqNum)
{
   if (m_inviteTransactionState == CpSipTransactionManager::INVITE_INACTIVE)
   {
      m_inviteTransactionState = CpSipTransactionManager::REINVITE_ACTIVE;
      m_iInviteCSeq = cseqNum;
      m_pInviteCookie = NULL;
      notifyTransactionStart(SIP_INVITE_METHOD, cseqNum);
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}

UtlBoolean CpSipTransactionManager::updateInviteTransaction(int cseqNum)
{
   if (m_inviteTransactionState != CpSipTransactionManager::INVITE_INACTIVE &&
      m_iInviteCSeq != cseqNum)
   {
      notifyTransactionEnd(SIP_INVITE_METHOD, cseqNum - 1); // end old transaction
      m_iInviteCSeq = cseqNum; // update cseqNum
      notifyTransactionStart(SIP_INVITE_METHOD, cseqNum); // start new transaction
      return TRUE;
   }

   return FALSE;
}

UtlBoolean CpSipTransactionManager::endTransaction(const UtlString& sipMethod, int cseqNum)
{
   cleanOldTransactions(); // maybe cleanup old terminated transactions

   if (sipMethod.compareTo(SIP_INVITE_METHOD) == 0)
   {
      // INVITE transaction
      if (m_iInviteCSeq == cseqNum)
      {
         endInviteTransaction();
         return TRUE;
      }
   }
   else
   {
      // non INVITE transaction
      UtlInt utlCSeq(cseqNum);
      CpTransactionState* pTransactionState = dynamic_cast<CpTransactionState*>(m_transactionMap.findValue(&utlCSeq));
      if (pTransactionState && pTransactionState->m_transactionState == CpSipTransactionManager::TRANSACTION_ACTIVE)
      {
         if (sipMethod.compareTo(pTransactionState->m_sipMethod) == 0)
         {
            notifyTransactionEnd(sipMethod, cseqNum); // end old transaction
            pTransactionState->m_transactionState = CpSipTransactionManager::TRANSACTION_TERMINATED;
            return TRUE;
         }
      }
   }

   return FALSE;
}

UtlBoolean CpSipTransactionManager::endTransaction(int cseqNum)
{
   cleanOldTransactions(); // maybe cleanup old terminated transactions

   // INVITE transaction
   if (m_iInviteCSeq == cseqNum)
   {
      endInviteTransaction();
      return TRUE;
   }
   else
   {
      // non INVITE transaction
      UtlInt utlCSeq(cseqNum);
      CpTransactionState* pTransactionState = dynamic_cast<CpTransactionState*>(m_transactionMap.findValue(&utlCSeq));
      if (pTransactionState && pTransactionState->m_transactionState == CpSipTransactionManager::TRANSACTION_ACTIVE)
      {
         notifyTransactionEnd(pTransactionState->m_sipMethod, cseqNum); // end old transaction
         pTransactionState->m_transactionState = CpSipTransactionManager::TRANSACTION_TERMINATED;
         return TRUE;
      }
   }

   return FALSE;
}

void CpSipTransactionManager::endInviteTransaction()
{
   if (m_inviteTransactionState != CpSipTransactionManager::INVITE_INACTIVE)
   {
      notifyTransactionEnd(SIP_INVITE_METHOD, m_iInviteCSeq); // end old transaction
      m_iInviteCSeq = -1;
      m_pInviteCookie = NULL;
      m_inviteTransactionState = CpSipTransactionManager::INVITE_INACTIVE;
   }
}

void CpSipTransactionManager::setSipTransactionListener(CpSipTransactionListener* pTransactionListener)
{
   m_pTransactionListener = pTransactionListener;
}

/* ============================ ACCESSORS ================================= */

UtlBoolean CpSipTransactionManager::setTransactionData(const UtlString& sipMethod, int cseqNum, void* pCookie)
{
   if (sipMethod.compareTo(SIP_INVITE_METHOD) == 0)
   {
      // INVITE transaction
      if (m_iInviteCSeq != -1 && m_iInviteCSeq == cseqNum)
      {
         m_pInviteCookie = pCookie;
         return TRUE;
      }
   }
   else
   {
      // non INVITE transaction
      UtlInt utlCSeq(cseqNum);
      CpTransactionState* pTransactionState = dynamic_cast<CpTransactionState*>(m_transactionMap.findValue(&utlCSeq));
      if (pTransactionState && pTransactionState->m_sipMethod.compareTo(sipMethod) == 0)
      {
         pTransactionState->m_pCookie = pCookie;
         return TRUE;
      }
   }
   return FALSE;
}

void* CpSipTransactionManager::getTransactionData(const UtlString& sipMethod, int cseqNum) const
{
   if (sipMethod.compareTo(SIP_INVITE_METHOD) == 0)
   {
      // INVITE transaction
      if (m_iInviteCSeq != -1 && m_iInviteCSeq == cseqNum)
      {
         return m_pInviteCookie;
      }
   }
   else
   {
      // non INVITE transaction
      UtlInt utlCSeq(cseqNum);
      CpTransactionState* pTransactionState = dynamic_cast<CpTransactionState*>(m_transactionMap.findValue(&utlCSeq));
      if (pTransactionState && pTransactionState->m_sipMethod.compareTo(sipMethod) == 0)
      {
         return pTransactionState->m_pCookie;
      }
   }

   return NULL;
}

int CpSipTransactionManager::getInviteCSeqNum() const
{
   return m_iInviteCSeq;
}

/* ============================ INQUIRY =================================== */

CpSipTransactionManager::TransactionState CpSipTransactionManager::getTransactionState(const UtlString& sipMethod,
                                                                                       int cseqNum) const
{
   if (sipMethod.compareTo(SIP_INVITE_METHOD) == 0)
   {
      // INVITE transaction
      if (m_iInviteCSeq != -1 && m_iInviteCSeq == cseqNum)
      {
         // we do not track terminated INVITE transactions
         return CpSipTransactionManager::TRANSACTION_ACTIVE;
      }
   }
   else
   {
      // non INVITE transaction
      UtlInt utlCSeq(cseqNum);
      CpTransactionState* pTransactionState = dynamic_cast<CpTransactionState*>(m_transactionMap.findValue(&utlCSeq));
      if (pTransactionState && pTransactionState->m_sipMethod.compareTo(sipMethod) == 0)
      {
         return pTransactionState->m_transactionState;
      }
   }

   return CpSipTransactionManager::TRANSACTION_NOT_FOUND;
}

CpSipTransactionManager::TransactionState CpSipTransactionManager::getTransactionState(int cseqNum) const
{
   // INVITE transaction
   if (m_iInviteCSeq != -1 && m_iInviteCSeq == cseqNum)
   {
      // we do not track terminated INVITE transactions
      return CpSipTransactionManager::TRANSACTION_ACTIVE;
   }
   else
   {
      // non INVITE transaction
      UtlInt utlCSeq(cseqNum);
      CpTransactionState* pTransactionState = dynamic_cast<CpTransactionState*>(m_transactionMap.findValue(&utlCSeq));
      if (pTransactionState)
      {
         return pTransactionState->m_transactionState;
      }
   }

   return CpSipTransactionManager::TRANSACTION_NOT_FOUND;
}

CpSipTransactionManager::TransactionState CpSipTransactionManager::getTransactionState(const SipMessage& rSipMessage) const
{
   UtlString method;
   int cseq;
   rSipMessage.getCSeqField(cseq, method);

   return getTransactionState(method, cseq);
}

CpSipTransactionManager::InviteTransactionState CpSipTransactionManager::getInviteTransactionState() const
{
   return m_inviteTransactionState;
}

UtlBoolean CpSipTransactionManager::isInviteTransactionActive() const
{
   return m_inviteTransactionState != CpSipTransactionManager::INVITE_INACTIVE;
}

UtlBoolean CpSipTransactionManager::isInviteTransaction(int cseqNum) const
{
   return isInviteTransactionActive() && m_iInviteCSeq == cseqNum;
}

int CpSipTransactionManager::getTransactionCount(const UtlString& method,
                                                 TransactionState state /*= TRANSACTION_ACTIVE*/)
{
   if (method.compareTo(SIP_INVITE_METHOD) == 0)
   {
      // invite
      if (m_inviteTransactionState == CpSipTransactionManager::INVITE_INACTIVE)
      {
         return 0;
      }
      else
      {
         return 1;
      }
   }
   else
   {
      // non invite transaction
      int count = 0;
      UtlHashMapIterator mapItor(m_transactionMap);
      while (mapItor())
      {
         CpTransactionState* pTransactionState = dynamic_cast<CpTransactionState*>(mapItor.value());
         if (pTransactionState &&
            pTransactionState->m_transactionState == state &&
            pTransactionState->m_sipMethod.compareTo(method) == 0)
         {
            count++;
         }
      }

      return count;
   }
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void CpSipTransactionManager::cleanOldTransactions()
{
   // get time in seconds since 1/1/1970
   long timeNow = OsDateTime::getSecsSinceEpoch();

   if (timeNow - m_lastCleanUpTime >= CLEAN_PERIOD)
   {
      m_lastCleanUpTime = timeNow;

      UtlHashMapIterator mapItor(m_transactionMap);
      while (mapItor())
      {
         CpTransactionState* pTransactionState = dynamic_cast<CpTransactionState*>(mapItor.value());
         if (pTransactionState &&
             ((pTransactionState->m_transactionState == CpSipTransactionManager::TRANSACTION_TERMINATED &&
             timeNow - pTransactionState->m_lastUsageTime >= maxTransactionStorageTime) ||
             (timeNow - pTransactionState->m_lastUsageTime >= maxTransactionStorageTime*10)))
         {
            // transaction is terminated and timeout elapsed, or has been stored for too long
            m_transactionMap.destroy(mapItor.key()); // destroy transaction
         }
      }
   }
}

void CpSipTransactionManager::notifyTransactionStart(const UtlString& sipMethod, int cseqNum)
{
   if (m_pTransactionListener)
   {
      m_pTransactionListener->onTransactionStart(sipMethod, cseqNum);
   }
}

void CpSipTransactionManager::notifyTransactionEnd(const UtlString& sipMethod, int cseqNum)
{
   if (m_pTransactionListener)
   {
      m_pTransactionListener->onTransactionEnd(sipMethod, cseqNum);
   }
}

/* ============================ FUNCTIONS ================================= */
