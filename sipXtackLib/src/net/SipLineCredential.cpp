//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SipLineCredentials.cpp: implementation of the SipLineCredentials class.
//
//////////////////////////////////////////////////////////////////////

#include <net/SipLineCredential.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SipLineCredential::SipLineCredential()
{

}

SipLineCredential::SipLineCredential(const UtlString& realm,
                                     const UtlString& userId,
                                     const UtlString& passwordToken,
                                     const UtlString& type)
: UtlString(realm) // initialize superclass string to realm, so that we can use this class in hashbag
{
   m_type = type;
   m_passwordToken = passwordToken;
   m_userId = userId;
   m_realm = realm;
}

SipLineCredential::~SipLineCredential()
{

}

SipLineCredential::SipLineCredential(const SipLineCredential& rSipLineCredentials)
{
   *this = rSipLineCredentials;
}

SipLineCredential& SipLineCredential::operator=(const SipLineCredential& rSipLineCredentials)
{
   UtlString::operator=(rSipLineCredentials); // copy UtlString superclass

   // copy this class members
   m_userId = rSipLineCredentials.m_userId;
   m_passwordToken = rSipLineCredentials.m_passwordToken;
   m_realm = rSipLineCredentials.m_realm;
   m_type = rSipLineCredentials.m_type;

   return *this;
}

UtlString SipLineCredential::getPasswordMD5Digest(const UtlString& realm) const
{
   UtlString digest;

   if (!realm.isNull())
   {
      HttpMessage::buildMd5UserPasswordDigest(m_userId, realm, m_passwordToken, digest);
   }
   else
   {
      HttpMessage::buildMd5UserPasswordDigest(m_userId, m_realm, m_passwordToken, digest);
   }

   return digest;
}

UtlString SipLineCredential::getRealm() const
{
   return m_realm;
}

UtlString SipLineCredential::getUserId() const
{
   return m_userId;
}

UtlString SipLineCredential::getPasswordToken() const
{
   return m_passwordToken;
}

UtlString SipLineCredential::getType() const
{
   return m_type;
}
