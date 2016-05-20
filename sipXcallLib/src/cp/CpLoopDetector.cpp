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
#include <utl/UtlHashMapIterator.h>
#include <net/SipMessage.h>
#include <cp/CpLoopDetector.h>

// DEFINES
#define MAX_STORED_MESSAGES 10

#ifndef ABS
#define ABS(x) ((x) < 0 ? (-(x)) : (x))
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/**
 * This class holds information about sent messages needed for loop detection.
 */
class CpMessageInfo : public UtlInt
{
public:
   CpMessageInfo(int id, const SipMessage& sipMessage)
      : UtlInt(id)
      , m_bIsRequest(FALSE)
      , m_seqNum(0)
   {
      m_bIsRequest = sipMessage.isRequest();
      sipMessage.getCSeqField(&m_seqNum, &m_seqMethod);

      UtlString viaField;
      sipMessage.getViaField(&viaField, 0);
      SipMessage::getViaTag(viaField, "branch", m_branchTag);
   }

   UtlBoolean m_bIsRequest; ///< if this message is request
   int m_seqNum; ///< sequence number
   UtlString m_seqMethod; ///< sip method
   UtlString m_branchTag; ///< branch tag of Via field
};


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

CpLoopDetector::CpLoopDetector()
: m_idCounter(1)
{

}

CpLoopDetector::~CpLoopDetector()
{

}

/* ============================ MANIPULATORS ============================== */

void CpLoopDetector::onMessageSent(const SipMessage& sipMessage)
{
   if (m_idCounter % (MAX_STORED_MESSAGES*2) == 0)
   {
      // from time to time clean old messages
      cleanOldMessages();
   }

   if (sipMessage.isRequest() && hasBranchTag(sipMessage))
   {
      int seqNum;
      UtlString seqMethod;
      sipMessage.getCSeqField(&seqNum, &seqMethod);
      UtlString sHashKey = getHashKey(seqNum, seqMethod, sipMessage.isRequest());

      CpMessageInfo* pMessageInfo = dynamic_cast<CpMessageInfo*>(m_messageMap.findValue(&sHashKey));
      if (!pMessageInfo) // if not found, then insert
      {
         // add message
         pMessageInfo = new CpMessageInfo(m_idCounter++, sipMessage);
         m_messageMap.insertKeyAndValue(sHashKey.clone(), pMessageInfo);
      }
   } // we only discover loops for requests
}

UtlBoolean CpLoopDetector::isInboundMessageLoop(const SipMessage& sipMessage) const
{
   if (sipMessage.isRequest())
   {
      UtlString sHashKey = getHashKey(sipMessage);
      CpMessageInfo* pMessageInfo = dynamic_cast<CpMessageInfo*>(m_messageMap.findValue(&sHashKey));

      if (pMessageInfo)
      {
         // we found sent request with the same method, cseq number, check branch tag
         UtlString localBranchTag = pMessageInfo->m_branchTag;
         UtlString msgBranchTag;
         UtlString viaField;
         int counter = 0;

         while (sipMessage.getViaField(&viaField, counter++))
         {
            SipMessage::getViaTag(viaField, "branch", msgBranchTag);

            if (localBranchTag.compareTo(msgBranchTag) == 0)
            {
               // we found our local branch in received message Via -> is a loop
               return TRUE;
            }

            msgBranchTag.remove(0);
            viaField.remove(0);
         }
      } // most of the time pMessageInfo will not be found
   }

   return FALSE;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlString CpLoopDetector::getHashKey(const SipMessage& sipMessage)
{
   int seqNum;
   UtlString seqMethod;
   sipMessage.getCSeqField(&seqNum, &seqMethod);

   return getHashKey(seqNum, seqMethod, sipMessage.isRequest());
}

UtlString CpLoopDetector::getHashKey(int seqNum, const UtlString& seqMethod, UtlBoolean bIsRequest)
{
   UtlString sHashKey;
   sHashKey.appendFormat("%d;%s;%s", seqNum, seqMethod.data(), bIsRequest ? "request" : "response");
   return sHashKey;
}

void CpLoopDetector::cleanOldMessages()
{
   UtlHashMapIterator itor(m_messageMap);
   CpMessageInfo* pMessageInfo;

   while (itor()) // go through all key-value pairs
   {
      pMessageInfo = dynamic_cast<CpMessageInfo*>(itor.value());
      if (pMessageInfo)
      {
         if (ABS(m_idCounter - pMessageInfo->getValue()) > MAX_STORED_MESSAGES)
         {
            // this value should be destroyed
            m_messageMap.destroy(itor.key());
         }
      }
   }
}

UtlBoolean CpLoopDetector::hasBranchTag(const SipMessage& sipMessage)
{
   UtlString viaField;
   if (sipMessage.getViaField(&viaField, 0))
   {
      UtlString branchTag;
      SipMessage::getViaTag(viaField, "branch", branchTag);
      return !branchTag.isNull();
   }

   return FALSE;
}

/* ============================ FUNCTIONS ================================= */
