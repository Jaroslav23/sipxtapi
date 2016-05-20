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

#ifndef CpLoopDetector_h__
#define CpLoopDetector_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlHashMap.h>
#include <utl/UtlString.h>

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
 * This class is responsible for recognizing sip message loops. A loop occurs when
 * we send a request and receive request with the same cseq number, method and
 * Via branch parameter from original request also exists in one of Via fields of
 * the received request.
 * 
 * We track a few last sent requests, and then check inbound requests against them.  
 *
 * This class is not thread safe.
 */
class CpLoopDetector
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /** Constructor */
   CpLoopDetector();

   /** Destructor */
   ~CpLoopDetector();

   /* ============================ MANIPULATORS ============================== */

   /** 
    * Must be called after some sip message is sent so that Via field is present
    * in message.
    */
   void onMessageSent(const SipMessage& sipMessage);

   /**
    * Checks if given inbound message is in a loop. Returns TRUE, if message
    * should not be accepted.
    */
   UtlBoolean isInboundMessageLoop(const SipMessage& sipMessage) const;

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Constructs hash key for hash map */
   static UtlString getHashKey(const SipMessage& sipMessage);

   /** Constructs hash key for hash map */
   static UtlString getHashKey(int seqNum, const UtlString& seqMethod, UtlBoolean bIsRequest);

   /** Goes through stored messages and cleans those that are too old */
   void cleanOldMessages();

   /** Returns TRUE if message has a branch tag */
   static UtlBoolean hasBranchTag(const SipMessage& sipMessage);

   UtlHashMap m_messageMap; ///< map for storing information about sent messages
   int m_idCounter; ///< counter for generating value ids
};

#endif // CpLoopDetector_h__
