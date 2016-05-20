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

#ifndef AcRenegotiateCodecsMsg_h__
#define AcRenegotiateCodecsMsg_h__

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
* Abstract call command message. Instructs call connection to renegotiate codecs.
* If sipDialog has NULL callid, then command is issued for all connections.
*/
class AcRenegotiateCodecsMsg : public AcCommandMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   AcRenegotiateCodecsMsg(const SipDialog& sipDialog,
                          const UtlString& sAudioCodecs,
                          const UtlString& sVideoCodecs);

   virtual ~AcRenegotiateCodecsMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   void getSipDialog(SipDialog& val) const { val = m_sipDialog; }
   UtlString getAudioCodecs() const { return m_sAudioCodecs; }
   UtlString getVideoCodecs() const { return m_sVideoCodecs; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private copy constructor */
   AcRenegotiateCodecsMsg(const AcRenegotiateCodecsMsg& rMsg);

   /** Private assignment operator */
   AcRenegotiateCodecsMsg& operator=(const AcRenegotiateCodecsMsg& rhs);

   SipDialog m_sipDialog;
   UtlString m_sAudioCodecs;
   UtlString m_sVideoCodecs;
};

#endif // AcRenegotiateCodecsMsg_h__
