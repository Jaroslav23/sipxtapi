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

#ifndef ScCommandMsg_h__
#define ScCommandMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <net/SipDialog.h>
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
* Sip connection command message. Instructs sip connection to carry out some action.
*
* This message is meant for communication between different XSipConnections through
* XCpCallManager. XCpCallManager knows how to route these messages correctly.
*/
class ScCommandMsg : public OsMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   typedef enum
   {
      SCC_FIRST = 0,
      SCC_START_RTP_REDIRECT, ///< sent from master call to slave call to order it to start RTP redirect
      SCC_STOP_RTP_REDIRECT, ///< sent from master/slave call to slave/master call to stop RTP redirect
      SCC_REESTABLISH_RTP_REDIRECT, ///< sent from master/slave call to slave/master call to reestablish RTP redirect
   } SubTypesEnum;

   /* ============================ CREATORS ================================== */

   ScCommandMsg(SubTypesEnum subType, const SipDialog& sipDialog);

   /** Copy constructor */
   ScCommandMsg(const ScCommandMsg& rMsg);

   virtual ~ScCommandMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /** Assignment operator */
   ScCommandMsg& operator=(const ScCommandMsg& rhs);

   /* ============================ ACCESSORS ================================= */

   void getSipDialog(SipDialog& val) const { val = m_sipDialog; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   SipDialog m_sipDialog; ///< sip dialog where this message should be routed
};

#endif // ScCommandMsg_h__
