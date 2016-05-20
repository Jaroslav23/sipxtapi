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

#ifndef OsStunResultSuccessMsg_h__
#define OsStunResultSuccessMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsStunResultMsg.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
* Sent when STUN request succeeds on a socket. STUN only affects UDP sockets.
*/
class OsStunResultSuccessMsg : public OsStunResultMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   OsStunResultSuccessMsg(const UtlString& sAdapterName,
                          const UtlString& sLocalIp,
                          int localPort,
                          const UtlString& sMappedIp,
                          int mappedPort);

   virtual ~OsStunResultSuccessMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   void getMappedIp(UtlString& sMappedIp) const { sMappedIp = m_sMappedIp; }
   int getMappedPort() const { return m_mappedPort; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   UtlString  m_sMappedIp;  ///< Mapped STUN address
   int        m_mappedPort; ///< Mapped STUN port

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private copy constructor */
   OsStunResultSuccessMsg(const OsStunResultSuccessMsg& rMsg);

   /** Private assignment operator */
   OsStunResultSuccessMsg& operator=(const OsStunResultSuccessMsg& rhs);
};

#endif // OsStunResultSuccessMsg_h__
