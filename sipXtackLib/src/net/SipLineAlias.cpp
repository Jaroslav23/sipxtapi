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
// APPLICATION INCLUDES
#include <net/SipLineAlias.h>
#include <net/SipLine.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType SipLineAlias::TYPE = "SipLineAlias";

// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

SipLineAlias::SipLineAlias(const Url& aliasUri,
                           const SipLine& sipLine)
: m_originalLineUri(sipLine.getLineUri())
, m_aliasUri(aliasUri)
{

}

SipLineAlias::SipLineAlias(const Url& aliasUri, const Url& sipLineUri)
: m_originalLineUri(sipLineUri)
, m_aliasUri(aliasUri)
{

}

SipLineAlias::~SipLineAlias()
{

}

SipLineAlias::SipLineAlias(const SipLineAlias& rSipLineAlias)
{
   *this = rSipLineAlias;
}

SipLineAlias& SipLineAlias::operator=(const SipLineAlias& rSipLineAlias)
{
   if (this == &rSipLineAlias)            // handle the assignment to self case
      return *this;
   else
   {
      m_aliasUri = rSipLineAlias.m_aliasUri;
      m_originalLineUri = rSipLineAlias.m_originalLineUri;
   }
   return *this;
}

/* ============================ MANIPULATORS ============================== */

UtlContainableType SipLineAlias::getContainableType() const
{
   return SipLineAlias::TYPE;
}

unsigned SipLineAlias::hash() const
{
   UtlString strIdentityUri = m_aliasUri.toString();
   strIdentityUri.toLower();
   return strIdentityUri.hash();
}

int SipLineAlias::compareTo(UtlContainable const *compareContainable) const
{
   int compareFlag = -1;

   if (compareContainable)
   {
      if (compareContainable->isInstanceOf(SipLineAlias::TYPE) == TRUE)
      {
         // for same type compare by identity uri
         SipLineAlias const *pLineAlias = dynamic_cast<SipLineAlias const *>(compareContainable);
         if (pLineAlias)
         {
            compareFlag = m_aliasUri.toString().compareTo(pLineAlias->getAliasUri().toString(), UtlString::ignoreCase);
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

UtlCopyableContainable* SipLineAlias::clone() const
{
   return new SipLineAlias(*this);
}

/* ============================ ACCESSORS ================================= */

Url SipLineAlias::getOriginalLineUri() const
{
   return m_originalLineUri;
}

Url SipLineAlias::getAliasUri() const
{
   return m_aliasUri;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
