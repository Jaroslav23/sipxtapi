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

#ifndef SipLineCredential_h__
#define SipLineCredential_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <net/Url.h>
#include <net/HttpMessage.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * Class for storing SIP line credential.
 */
class SipLineCredential : public UtlString
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   SipLineCredential();

   SipLineCredential(const UtlString& realm, ///< realm in which username and password are valid
                     const UtlString& userId, ///< username
                     const UtlString& passwordToken, ///< clear text password
                     const UtlString& type); ///< type of credential

   virtual ~SipLineCredential();
   SipLineCredential(const SipLineCredential& rSipLineCredentials);
   SipLineCredential& operator=(const SipLineCredential& rSipLineCredentials);

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   /** Gets realm assigned to credential */
   UtlString getRealm() const;

   /** Gets username assigned to credential */
   UtlString getUserId() const;

   /** Gets clear text password assigned to credential */
   UtlString getPasswordToken() const;

   /**
    *  Gets MD5 digest of userid, realm and password. Uses supplied
    *  realm if not NULL, otherwise credentials realm.
    */
   UtlString getPasswordMD5Digest(const UtlString& realm = NULL) const;

   /** Gets type of credential */
   UtlString getType() const;

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   UtlString m_userId; ///< username
   UtlString m_passwordToken; ///< clear text password
   UtlString m_realm; ///< realm in which username is valid
   UtlString m_type; ///< type of credential
};

#endif // SipLineCredential_h__
