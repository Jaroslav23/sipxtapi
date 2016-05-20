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

#if defined(_VXWORKS)
#   include <taskLib.h>
#   include <netinet/in.h>
#endif

#include <net/SipMessage.h>
#include <utl/UtlHashBagIterator.h>
#include <net/SipLine.h>
#include <net/Url.h>
#include <net/SipLineCredential.h>
#include <os/OsConfigDb.h>
#include <os/OsRWMutex.h>
#include <os/OsReadLock.h>
#include <os/OsWriteLock.h>
#include <os/OsDateTime.h>
#include <net/NetMd5Codec.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

const UtlContainableType SipLine::TYPE = "SipLine";

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SipLine::SipLine(const Url& fullLineUrl,
                 LineStateEnum state)
: m_bAllowContactOverride(TRUE)
, m_preferredTransport(SIP_TRANSPORT_AUTO)
{
   m_fullLineUrl = SipLine::buildFullLineUrl(fullLineUrl);
   //then get uri from user entered url ...uri is complete in it
   m_lineUri = SipLine::getLineUri(fullLineUrl);
   m_currentState = state;

   // build default contact
   m_preferredContactUri = buildLineContact();
}

SipLine::~SipLine()
{
   m_credentials.destroyAll();
}

// Copy constructor
SipLine::SipLine(const SipLine& rSipLine)
{
   *this = rSipLine;
}

SipLine& SipLine::operator=(const SipLine& rSipLine)
{
   if (this == &rSipLine)            // handle the assignment to self case
      return *this;
   else
   {
      m_lineUri = rSipLine.m_lineUri;
      m_fullLineUrl = rSipLine.m_fullLineUrl;
      m_currentState = rSipLine.m_currentState;
      m_preferredContactUri = rSipLine.m_preferredContactUri;
      m_bAllowContactOverride = rSipLine.m_bAllowContactOverride;
      m_preferredTransport = rSipLine.m_preferredTransport;
      copyCredentials(rSipLine);
   }
   return *this;
}

//deep copy of credentials
void SipLine::copyCredentials(const SipLine &rSipLine)
{
   if(!m_credentials.isEmpty())
   {
      m_credentials.destroyAll();
   }
   if (!rSipLine.m_credentials.isEmpty())
   {
      UtlHashBagIterator observerIterator(const_cast<UtlHashBag&> (rSipLine.m_credentials));
      SipLineCredential* credential = NULL;

      while((credential = dynamic_cast<SipLineCredential*>(observerIterator())) != NULL)
      {
         addCredential(credential->getRealm(),
                       credential->getUserId(),
                       credential->getPasswordToken(),
                       credential->getType());
      }
   }
}

SipLine::LineStateEnum SipLine::getState() const
{
    return m_currentState;
}

void SipLine::setState(SipLine::LineStateEnum state)
{
    m_currentState = state;
}

UtlString SipLine::getUserId() const
{
    return m_fullLineUrl.getUserId();
}

Url SipLine::getFullLineUrl() const
{
    return m_fullLineUrl;
}
Url SipLine::getLineUri() const
{
    return m_lineUri;
}

UtlBoolean SipLine::addCredential(const UtlString& strRealm,
                                  const UtlString& strUserID,
                                  const UtlString& strPasswd,
                                  const UtlString& strType)
{
   UtlBoolean isAdded = FALSE;

   if(!realmExists(strRealm))
   {
      SipLineCredential* credential = new SipLineCredential(strRealm , strUserID, strPasswd, strType);
      m_credentials.insert(credential);
      isAdded = TRUE;
   }

   return isAdded;
}

UtlBoolean SipLine::addCredential(const SipLineCredential& lineCredential)
{
   UtlBoolean isAdded = FALSE;

   if(!realmExists(lineCredential.getRealm()))
   {
      SipLineCredential* credential = new SipLineCredential(lineCredential);
      m_credentials.insert(credential);
      isAdded = TRUE;
   }

   return isAdded;
}

int SipLine::getNumOfCredentials() const
{
   return m_credentials.entries();
}

UtlBoolean SipLine::getCredential(const UtlString& type,
                                  const UtlString& realm,
                                  UtlString& userID,
                                  UtlString& passwordMD5Digest) const
{
   SipLineCredential lineCredential;

   UtlBoolean result = getCredential(type, realm, lineCredential);
   if (result)
   {
      userID = lineCredential.getUserId();
      passwordMD5Digest = lineCredential.getPasswordMD5Digest(realm);
   }

   return result;
}

UtlBoolean SipLine::getCredential(const UtlString& type,
                                  const UtlString& realm,
                                  SipLineCredential& lineCredential) const
{
   if (type.isNull())
   {
      return FALSE;
   }

   UtlString emptyRealm(NULL);

   SipLineCredential* credential = dynamic_cast<SipLineCredential*>(m_credentials.find(&realm));
   if (credential && !type.compareTo(credential->getType(), UtlString::matchCase))
   {
      // also type must match
      lineCredential = *credential;
   }
   else // if realm was not found, then return credentials for default = empty realm
   {
      credential = dynamic_cast<SipLineCredential*>(m_credentials.find(&emptyRealm));
      if (credential && !type.compareTo(credential->getType(), UtlString::matchCase))
      {
         // also type must match
         lineCredential = *credential;
      }
   }

   return credential != NULL;
}

UtlBoolean SipLine::removeCredential(const UtlString& type,
                                     const UtlString& realm)
{
   return m_credentials.destroy(&realm);
}

void SipLine::removeAllCredentials()
{
    m_credentials.destroyAll();
}

void SipLine::setPreferredContact(const UtlString& contactAddress, int contactPort)
{
   m_preferredContactUri = buildLineContact(contactAddress, contactPort);
}

Url SipLine::getPreferredContactUri() const
{
   return m_preferredContactUri;
}

SIP_TRANSPORT_TYPE SipLine::getPreferredTransport() const
{
   return m_preferredTransport;
}

void SipLine::setPreferredTransport(SIP_TRANSPORT_TYPE transport)
{
   m_preferredTransport = transport;
   // also update transport parameter in contact
   if (m_preferredTransport == SIP_TRANSPORT_TCP)
   {
      m_preferredContactUri.setUrlParameter(SIP_TRANSPORT, SIP_TRANSPORT_TCP_STR);
   }
   else
   {
      m_preferredContactUri.removeUrlParameter(SIP_TRANSPORT);
   }
}

UtlBoolean SipLine::realmExists(const UtlString& realm) const
{
   SipLineCredential* credential = dynamic_cast<SipLineCredential*>(m_credentials.find(&realm));
   return credential != NULL;
}

Url SipLine::buildLineContact(const UtlString& address, int port)
{
   Url contactUrl(m_fullLineUrl);

   if (!address.isNull())
   {
      contactUrl.setHostAddress(address);
      contactUrl.setHostPort(port);
   }
   contactUrl.setPassword(NULL);
   contactUrl.setPath(NULL);
   contactUrl.includeAngleBrackets();

   if (m_preferredTransport == SIP_TRANSPORT_TCP)
   {
      contactUrl.setUrlParameter(SIP_TRANSPORT, SIP_TRANSPORT_TCP_STR);
   }
   else
   {
      contactUrl.removeUrlParameter(SIP_TRANSPORT);
   }

   return contactUrl;
}

UtlContainableType SipLine::getContainableType() const
{
   return SipLine::TYPE;
}

unsigned SipLine::hash() const
{
   UtlString strIdentityUri = m_lineUri.toString();
   strIdentityUri.toLower();
   return strIdentityUri.hash();
}

int SipLine::compareTo(UtlContainable const *compareContainable) const
{
   int compareFlag = -1;

   if (compareContainable)
   {
      if (compareContainable->isInstanceOf(SipLine::TYPE) == TRUE)
      {
         // for same type compare by identity uri
         SipLine const *pLine = dynamic_cast<SipLine const *>(compareContainable);
         if (pLine)
         {
            compareFlag = m_lineUri.toString().compareTo(pLine->getLineUri().toString(), UtlString::ignoreCase);
         }
      }
      else
      {
         // for different type compare by direct hash
         compareFlag = directHash() - compareContainable->directHash();
      }
   }

   return compareFlag;
}

UtlCopyableContainable* SipLine::clone() const
{
   return new SipLine(*this);
}

Url SipLine::getLineUri(const Url& url)
{
   Url lineUri(url.toString());
   lineUri.setDisplayName(NULL);
   if (lineUri.getScheme() == Url::SipsUrlScheme)
   {
      // override scheme, we want sip and sips lines to be equivalent
      // this is only used for line lookup, and doesn't violate security
      lineUri.setScheme(Url::SipUrlScheme);
   }
   lineUri.removeAngleBrackets();
   lineUri.removeParameters();
   lineUri.setPassword(NULL); // line uri cannot have password
   lineUri.setHostPort(PORT_NONE); // line uri doesn't have port associated with it
   return lineUri;
}

Url SipLine::getLineUri(const UtlString& sUrl)
{
   Url lineUri(sUrl);
   lineUri.setDisplayName(NULL);
   if (lineUri.getScheme() == Url::SipsUrlScheme)
   {
      // override scheme, we want sip and sips lines to be equivalent
      // this is only used for line lookup, and doesn't violate security
      lineUri.setScheme(Url::SipUrlScheme);
   }
   lineUri.removeAngleBrackets();
   lineUri.removeParameters();
   lineUri.setPassword(NULL); // line uri cannot have password
   lineUri.setHostPort(PORT_NONE); // line uri doesn't have port associated with it
   return lineUri;
}

UtlBoolean SipLine::areLineUrisEqual(const Url& leftUri, const Url& rightUri)
{
   return leftUri.isUserHostEqual(rightUri);
}

Url SipLine::buildFullLineUrl(const Url& url)
{
   Url fullLineUrl(url.toString());
   fullLineUrl.includeAngleBrackets();
   fullLineUrl.removeFieldParameters();
   fullLineUrl.removeHeaderParameter("transport");
   return fullLineUrl;
}

Url SipLine::buildFullLineUrl(const UtlString& sUrl)
{
   Url fullLineUrl(sUrl);
   fullLineUrl.includeAngleBrackets();
   fullLineUrl.removeFieldParameters();
   fullLineUrl.removeHeaderParameter("transport");
   return fullLineUrl;
}

UtlString SipLine::getProxyServers() const
{
   return m_proxyServers;
}

void SipLine::setProxyServers(const UtlString& proxyServers)
{
   m_proxyServers = proxyServers;
}

const char* SipLine::convertLineStateToString(LineStateEnum lineState)
{
   const char* str = "Unknown";

   switch (lineState)
   {
   case LINE_STATE_UNKNOWN:
      str = MAKESTR(LINE_STATE_UNKNOWN);
      break;
   case LINE_STATE_REGISTERED:
      str = MAKESTR(LINE_STATE_REGISTERED);
      break;
   case LINE_STATE_DISABLED:
      str = MAKESTR(LINE_STATE_DISABLED);
      break;
   case LINE_STATE_FAILED:
      str = MAKESTR(LINE_STATE_FAILED);
      break;
   case LINE_STATE_PROVISIONED:
      str = MAKESTR(LINE_STATE_PROVISIONED);
      break;
   case LINE_STATE_TRYING:
      str = MAKESTR(LINE_STATE_TRYING);
      break;
   case LINE_STATE_EXPIRED:
      str = MAKESTR(LINE_STATE_EXPIRED);
      break;
   default:
      break;
   }

   return str;
}

UtlString SipLine::getStateAsString() const
{
   return SipLine::convertLineStateToString(m_currentState);
}
