//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "net/SipContactDb.h"
#include "utl/UtlInt.h"
#include "utl/UtlVoidPtr.h"
#include "os/OsLock.h"
#include "utl/UtlHashMapIterator.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipContactDb::SipContactDb()
: m_nextContactId(1)
, m_mutex(OsMutex::Q_FIFO)
, m_bTurnEnabled(FALSE)
{
    
}

// Destructor
SipContactDb::~SipContactDb()
{
   m_contacts.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean SipContactDb::addContact(SipContact& sipContact)
{
   OsLock lock(m_mutex);
   UtlBoolean bRet = FALSE;

   assert(sipContact.getContactId() < 1);

   if (!contactExists(sipContact))
   {
      assignContactId(sipContact);

      SipContact* pContact = (SipContact*)sipContact.clone();
      m_contacts.insertKeyAndValue(new UtlInt(pContact->getContactId()),
                                   pContact);

      // If turn is enabled, duplicate the contact with a relay type
      if (m_bTurnEnabled &&
         pContact->getContactType() == SIP_CONTACT_LOCAL &&
         pContact->getTransportType() == SIP_TRANSPORT_UDP)
      {
         SipContact* pRelayContact = (SipContact*)sipContact.clone();
         pRelayContact->setContactType(SIP_CONTACT_RELAY);
         pRelayContact->setContactId(-1);
         addContact(*pRelayContact);
      }
      bRet = TRUE;
   }

   return bRet;
}

UtlBoolean SipContactDb::deleteContact(int contactId)
{
   OsLock lock(m_mutex);
   UtlInt idKey(contactId);
   return m_contacts.destroy(&idKey);
}

SipContact* SipContactDb::find(int contactId) const
{
   OsLock lock(m_mutex);
   UtlInt idKey(contactId);

   SipContact* pContact = dynamic_cast<SipContact*>(m_contacts.findValue(&idKey));
   if (pContact)
   {
      return (SipContact*)pContact->clone();
   }

   return NULL;
}

SipContact* SipContactDb::find(SIP_CONTACT_TYPE typeFilter,
                               SIP_TRANSPORT_TYPE transportFilter) const
{
   OsLock lock(m_mutex);
   UtlHashMapIterator itor(m_contacts);

   while (itor())
   {
      SipContact* pContact = dynamic_cast<SipContact*>(itor.value());
      if (pContact)
      {
         if (((typeFilter == SIP_CONTACT_AUTO) || (pContact->getContactType() == typeFilter)) &&
            (pContact->getTransportType() == transportFilter || transportFilter == SIP_TRANSPORT_AUTO))
         {
             return (SipContact*)pContact->clone();
         }
      }
   }

   return NULL;
}

SipContact* SipContactDb::find(const UtlString& adapterIp,
                               SIP_CONTACT_TYPE typeFilter,
                               SIP_TRANSPORT_TYPE transportFilter) const
{
   OsLock lock(m_mutex);
   UtlHashMapIterator itor(m_contacts);

   while (itor())
   {
      SipContact* pContact = dynamic_cast<SipContact*>(itor.value());
      if (pContact)
      {
         if (pContact->hasAdapterIp(adapterIp) &&
            ((typeFilter == SIP_CONTACT_AUTO) || (pContact->getContactType() == typeFilter)) &&
            (pContact->getTransportType() == transportFilter || transportFilter == SIP_TRANSPORT_AUTO))
         {
            return (SipContact*)pContact->clone();
         }
      }
   }

   return NULL;
}

void SipContactDb::getAll(UtlSList& contacts) const
{
   OsLock lock(m_mutex);
   contacts.destroyAll();
   UtlHashMapIterator itor(m_contacts);

   while (itor())
   {
      SipContact* pContact = dynamic_cast<SipContact*>(itor.value());
      if (pContact)
      {
         contacts.append(pContact->clone());
      }
   }
}
                                             
void SipContactDb::getAllForAdapterName(UtlSList& contacts,
                                        const UtlString& adapterName,
                                        SIP_CONTACT_TYPE typeFilter,
                                        SIP_TRANSPORT_TYPE transportFilter) const
{
   OsLock lock(m_mutex);
   contacts.destroyAll();
   UtlHashMapIterator itor(m_contacts);

   while (itor())
   {
      SipContact* pContact = dynamic_cast<SipContact*>(itor.value());
      if (pContact)
      {
         if (pContact->hasAdapterName(adapterName) &&
            ((typeFilter == SIP_CONTACT_AUTO) || (pContact->getContactType() == typeFilter)) &&
            (pContact->getTransportType() == transportFilter || transportFilter == SIP_TRANSPORT_AUTO))
         {
            contacts.append(pContact->clone());
         }
      }
   }
}

void SipContactDb::getAllForAdapterIp(UtlSList& contacts,
                                      const UtlString& adapterIp,
                                      SIP_CONTACT_TYPE typeFilter,
                                      SIP_TRANSPORT_TYPE transportFilter) const
{
   OsLock lock(m_mutex);
   contacts.destroyAll();
   UtlHashMapIterator itor(m_contacts);

   while (itor())
   {
      SipContact* pContact = dynamic_cast<SipContact*>(itor.value());
      if (pContact)
      {
         if (pContact->hasAdapterIp(adapterIp) &&
            ((typeFilter == SIP_CONTACT_AUTO) || (pContact->getContactType() == typeFilter)) &&
            (pContact->getTransportType() == transportFilter || transportFilter == SIP_TRANSPORT_AUTO))
         {
            contacts.append(pContact->clone());
         }
      }
   }
}

void SipContactDb::enableTurn(UtlBoolean bEnable) 
{
   OsLock lock(m_mutex);
   UtlHashMapIterator itor(m_contacts);
   m_bTurnEnabled = bEnable;    

   SipContact* pContact = NULL;
   while (itor())
   {
      pContact = dynamic_cast<SipContact*>(itor.value());
      if (pContact)
      {
         if (m_bTurnEnabled)
         {
            if (pContact->getContactType() == SIP_CONTACT_LOCAL &&
                pContact->getTransportType() == SIP_TRANSPORT_UDP)
            {
               SipContact* pRelayContact = (SipContact*)pContact->clone();
               pRelayContact->setContactType(SIP_CONTACT_RELAY);
               pRelayContact->setContactId(-1);
               addContact(*pRelayContact);
            }
         }
         else // turn is disabled
         {
            if (pContact->getContactType() == SIP_CONTACT_RELAY)
            {
               deleteContact(pContact->getContactId());
            }
         }
      }
   }
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

UtlBoolean SipContactDb::contactExists(int id) const
{
   OsLock lock(m_mutex);

   UtlInt idKey(id);
   return m_contacts.findValue(&idKey) != NULL;
}

UtlBoolean SipContactDb::contactExists(const UtlString& ipAddress,
                                       int port,
                                       SIP_CONTACT_TYPE type,
                                       SIP_TRANSPORT_TYPE transportType) const
{
   OsLock lock(m_mutex);

   SipContact* pSipContact = NULL;
   UtlHashMapIterator itor(m_contacts);
   while (itor())
   {
      pSipContact = dynamic_cast<SipContact*>(itor.value());
      if (pSipContact)
      {
         if (pSipContact->hasIpAddress(ipAddress) &&
            pSipContact->getPort() == port &&
            pSipContact->getContactType() == type &&
            pSipContact->getTransportType() == transportType)
         {
            return TRUE;
         }
      }
   }

   return FALSE;    
}

UtlBoolean SipContactDb::contactExists(const SipContact& sipContact) const
{
   UtlString ipAddress;
   sipContact.getIpAddress(ipAddress);
   return contactExists(ipAddress, sipContact.getPort(),
      sipContact.getContactType(), sipContact.getTransportType());
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean SipContactDb::assignContactId(SipContact& sipContact)
{
    OsLock lock(m_mutex);    
    sipContact.setContactId(m_nextContactId++);
    return TRUE;
}

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

