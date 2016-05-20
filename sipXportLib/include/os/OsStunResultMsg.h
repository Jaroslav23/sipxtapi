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

#ifndef OsStunResultMsg_h__
#define OsStunResultMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <utl/UtlString.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
* Base class for STUN result messages (notifications).
*/
class OsStunResultMsg : public OsMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   typedef enum
   {
      STUN_RESULT_FAILURE = 0,///< STUN request failed
      STUN_RESULT_SUCCESS, ///< when STUN request succeeded
   } SubTypeEnum;

   /* ============================ CREATORS ================================== */

   OsStunResultMsg(SubTypeEnum subType,
                   const UtlString& sAdapterName,
                   const UtlString& sLocalIp,
                   int localPort);

   virtual ~OsStunResultMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   void getAdapterName(UtlString& sAdapterName) const { sAdapterName = m_sAdapterName; }
   void getLocalIp(UtlString& sLocalIp) const { sLocalIp = m_sLocalIp; }
   int getLocalPort() const { return m_localPort; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   UtlString m_sAdapterName; ///< name of adapter for which STUN result is available
   UtlString m_sLocalIp; ///< local IP address
   int m_localPort; ///< local port

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private copy constructor */
   OsStunResultMsg(const OsStunResultMsg& rMsg);

   /** Private assignment operator */
   OsStunResultMsg& operator=(const OsStunResultMsg& rhs);
};

#endif // OsStunResultMsg_h__
