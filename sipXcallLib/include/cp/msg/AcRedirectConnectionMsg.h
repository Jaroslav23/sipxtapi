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

#ifndef AcRedirectConnectionMsg_h__
#define AcRedirectConnectionMsg_h__

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
* Abstract call command message. Instructs call to redirect call connection.
*/
class AcRedirectConnectionMsg : public AcCommandMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   AcRedirectConnectionMsg(const SipDialog& sSipDialog,
                           const UtlString& sRedirectSipUrl);

   virtual ~AcRedirectConnectionMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   void getSipDialog(SipDialog& sSipDialog) const { sSipDialog = m_sSipDialog; }
   UtlString getRedirectSipUrl() const { return m_sRedirectSipUrl; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private copy constructor */
   AcRedirectConnectionMsg(const AcRedirectConnectionMsg& rMsg);

   /** Private assignment operator */
   AcRedirectConnectionMsg& operator=(const AcRedirectConnectionMsg& rhs);

   SipDialog m_sSipDialog;
   UtlString m_sRedirectSipUrl;
};

#endif // AcRedirectConnectionMsg_h__
