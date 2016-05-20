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

#ifndef AcSendInfoMsg_h__
#define AcSendInfoMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <utl/UtlString.h>
#include <net/SipDialog.h>
#include <cp/CpMessageTypes.h>
#include <cp/msg/AcCommandMsg.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
* Abstract call command message. Instructs call to send SIP INFO on given connection.
*/
class AcSendInfoMsg : public AcCommandMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   AcSendInfoMsg(const SipDialog& sipDialog,
                 const UtlString& sContentType,
                 const char* pContent,
                 const size_t nContentLength,
                 void* pCookie);

   virtual ~AcSendInfoMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   void getSipDialog(SipDialog& val) const { val = m_sipDialog; }
   UtlString getContentType() const { return m_sContentType; }
   const char* getContent() const { return m_pContent; }
   size_t getContentLength() const { return m_nContentLength; }
   void* getCookie() const { return m_pCookie; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private copy constructor */
   AcSendInfoMsg(const AcSendInfoMsg& rMsg);

   /** Private assignment operator */
   AcSendInfoMsg& operator=(const AcSendInfoMsg& rhs);

   SipDialog m_sipDialog;
   UtlString m_sContentType;
   char* m_pContent; ///< payload bytes
   size_t m_nContentLength; ///< length of content
   void* m_pCookie;
};

#endif // AcSendInfoMsg_h__
