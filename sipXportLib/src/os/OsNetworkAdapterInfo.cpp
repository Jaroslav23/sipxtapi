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
#include <os/OsNetworkAdapterInfo.h>
#include <utl/UtlSListIterator.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType OsNetworkAdapterInfo::TYPE = "OsNetworkAdapterInfo";

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

OsNetworkAdapterInfo::OsNetworkAdapterInfo(const UtlString& name,
                                           const UtlString& description,
                                           UtlSList* pIpAddresses)
: m_name(name)
, m_description(description)
, m_pIpAddresses(pIpAddresses)
{

}

OsNetworkAdapterInfo::~OsNetworkAdapterInfo()
{
   if (m_pIpAddresses)
   {
      m_pIpAddresses->destroyAll();
      delete m_pIpAddresses;
      m_pIpAddresses = NULL;
   }
}

OsNetworkAdapterInfo::OsNetworkAdapterInfo(const OsNetworkAdapterInfo& rhs)
: m_name(rhs.m_name)
, m_description(rhs.m_description)
, m_pIpAddresses(NULL)
{
   if (rhs.m_pIpAddresses)
   {
      // clone list of ip addresses
      m_pIpAddresses = new UtlSList();
      UtlSListIterator itor(*rhs.m_pIpAddresses);
      UtlCopyableContainable *pItem = NULL;
      while (itor())
      {
         pItem = dynamic_cast<UtlCopyableContainable*>(itor.item());
         if (pItem)
         {
            m_pIpAddresses->append(pItem->clone());
         }
      }
   }
}

/* ============================ MANIPULATORS ============================== */

UtlContainableType OsNetworkAdapterInfo::getContainableType() const
{
   return OsNetworkAdapterInfo::TYPE;
}

unsigned OsNetworkAdapterInfo::hash() const
{
   return m_name.hash();
}

int OsNetworkAdapterInfo::compareTo(UtlContainable const *compareContainable) const
{
   int compareFlag = -1;

   if (compareContainable)
   {
      if (compareContainable->isInstanceOf(OsNetworkAdapterInfo::TYPE) == TRUE)
      {
         OsNetworkAdapterInfo const *pNetworkAdapterInfo = dynamic_cast<OsNetworkAdapterInfo const *>(compareContainable);
         if (pNetworkAdapterInfo)
         {
            UtlString otherName;
            pNetworkAdapterInfo->getName(otherName);
            compareFlag = m_name.compareTo(otherName, UtlString::matchCase);
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

UtlCopyableContainable* OsNetworkAdapterInfo::clone() const
{
   return new OsNetworkAdapterInfo(*this);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

