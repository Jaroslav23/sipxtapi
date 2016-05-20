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

#ifndef CpSipTransactionManager_h__
#define CpSipTransactionManager_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlDefs.h>
#include <utl/UtlRandom.h>
#include <utl/UtlHashMap.h>
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
 * CpSipTransactionManager is sipXcallLib layer transaction manager. This one uses Cseq number
 * and method name to differentiate between transactions, and supports transaction
 * tracking for multiple transactions with the same method at time. Separate transaction
 * manager is needed for inbound and outbound transactions. Transactions are stored as indexed by
 * cseq number.
 * 
 * This transaction manager is needed because RFC3261 also requires transaction
 * tracking on UAC/UAS core layer, not only transaction layer. We cannot reuse
 * SipTransaction from transaction layer here. This class is more powerful than
 * old CSeqManager. It tracks correctly individual transactions, not only the last one.
 * 
 * This class is NOT meant to resend INVITE, CANCEL, BYE etc. when packet loss occurs.
 * That is done by transaction layer.
 *
 * ACK resend in INVITE transaction after 2xx is done when another 2xx is received.
 * (remote side didn't get ACK, and resent 2xx). ACK is part of INVITE transaction only
 * if final response was non 2xx.
 * 
 * To track transaction state correctly for 401 and 407 authentication retry, for INVITE
 * updateInviteTransaction must be called with cseqNum+1. For non-INVITE, endTransaction with cseqNum
 * must be called, and startTransaction with cseqNum+1 when authentication retry message is detected.
 * SipUserAgent automatically just increments cseqNum for authentication retry.
 *
 * Terminated transactions are automatically cleaned up after Timer B time. Non terminated transactions
 * are cleaned up after much longer time.
 *
 * INVITE transactions are tracked differently. Only 1 INVITE transaction can be occurring
 * at time. This class therefore supports tracking only 1 INVITE transaction.
 *
 * This class is NOT thread safe.  
 */
class CpSipTransactionManager
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   typedef enum
   {
      TRANSACTION_NOT_FOUND = 0,
      TRANSACTION_ACTIVE,
      TRANSACTION_TERMINATED
   } TransactionState;

   typedef enum
   {
      INVITE_INACTIVE = 0, ///< no INVITE transaction is active
      INITIAL_INVITE_ACTIVE, ///< initial INVITE transaction is active
      REINVITE_ACTIVE, ///< a re-INVITE transaction is active
   } InviteTransactionState;

   /* ============================ CREATORS ================================== */

   /** Constructor. */
   CpSipTransactionManager();

   /** Destructor. */
   ~CpSipTransactionManager();

   /* ============================ MANIPULATORS ============================== */

   /**
    * Starts new transaction for given method and cseqNum.
    * For INVITE, this method starts normal INVITE transaction. For starting
    * re-INVITE transaction, use separate method.
    *
    * Only 1 INVITE/re-INVITE transaction may be active at time.
    */
   UtlBoolean startTransaction(const UtlString& sipMethod, int cseqNum);

   /**
    * Starts initial INVITE transaction. We allow only 1 INVITE transaction at time.
    */
   UtlBoolean startInitialInviteTransaction(int cseqNum);

   /**
   * Starts re-INVITE transaction.
   */
   UtlBoolean startReInviteTransaction(int cseqNum);

   /**
    * Updates an active INVITE/re-INVITE transaction with new cseqNum. We need to supply
    * new cseqNum after 401/407 authentication retry, when cseqNum is normally incremented.
    * Old transaction will be terminated automatically.
    */
   UtlBoolean updateInviteTransaction(int cseqNum);

   /**
    * Marks given transaction as terminated.
    *
    * This method can be used for INVITE transaction.
    */
   UtlBoolean endTransaction(const UtlString& sipMethod, int cseqNum);

   /**
    * Marks given transaction as terminated.
    *
    * This method can be used for INVITE transaction.
    */
   UtlBoolean endTransaction(int cseqNum);

   /**
    * Stops INVITE transaction regardless of cseqNum.
    */
   void endInviteTransaction();

   /** Sets listener that should be notified about transaction start/end .*/
   void setSipTransactionListener(CpSipTransactionListener* pTransactionListener);

   /* ============================ ACCESSORS ================================= */

   /** Assigns custom data to transaction, which can be retrieved later. */
   UtlBoolean setTransactionData(const UtlString& sipMethod, int cseqNum, void* pCookie);

   /** Gets data stored with transaction */
   void* getTransactionData(const UtlString& sipMethod, int cseqNum) const;

   /** Gets Invite transaction cseq number */
   int getInviteCSeqNum() const;

   /* ============================ INQUIRY =================================== */

   /**
   * Finds transaction state for given sip method.
   *
   * @param szSipMethod Can be INVITE, INFO, NOTIFY, REFER, OPTIONS.
   * @param cseqNum CSeq number from Sip message.
   */
   CpSipTransactionManager::TransactionState getTransactionState(const UtlString& sipMethod, int cseqNum) const;

   /**
    * Finds transaction state for given cseq number.
    */
   CpSipTransactionManager::TransactionState getTransactionState(int cseqNum) const;

   /**
    * Finds transaction state for given sip message.
    */
   CpSipTransactionManager::TransactionState getTransactionState(const SipMessage& rSipMessage) const;

   /**
    * Gets state of invite transaction. This is more accurate than getTransactionState.
    */
   CpSipTransactionManager::InviteTransactionState getInviteTransactionState() const;

   /** Returns TRUE if an INVITE transaction is active */
   UtlBoolean isInviteTransactionActive() const;

   /** Returns TRUE if given transaction is INVITE */
   UtlBoolean isInviteTransaction(int cseqNum) const;

   /** Gets number of transactions in given state for given method */
   int getTransactionCount(const UtlString& method, TransactionState state = TRANSACTION_ACTIVE);

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Disabled copy constructor */ 
   CpSipTransactionManager(const CpSipTransactionManager& rhs);     

   /** Disabled assignment operator */
   CpSipTransactionManager& operator=(const CpSipTransactionManager& rhs);

   /** Cleans terminated transactions older than 60s */
   void cleanOldTransactions();

   /** Called to notify listeners that a transaction has been started */
   void notifyTransactionStart(const UtlString& sipMethod, int cseqNum);

   /** Called to notify listeners that a transaction has been stopped */
   void notifyTransactionEnd(const UtlString& sipMethod, int cseqNum);

   // members for generic transactions
   UtlHashMap m_transactionMap; ///< hashmap for storing transactions by cseq number
   long m_lastCleanUpTime;
   CpSipTransactionListener* m_pTransactionListener; ///< listener that gets notifications about transaction start/end

   // INVITE transaction specific
   int m_iInviteCSeq; ///< CSeq of INVITE transaction
   InviteTransactionState m_inviteTransactionState; ///< keeps invite transaction state
   void* m_pInviteCookie; ///< custom data stored with INVITE transaction
};

#endif // CpSipTransactionManager_h__
