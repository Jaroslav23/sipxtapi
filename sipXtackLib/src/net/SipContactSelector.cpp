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
#include <os/OsNatAgentTask.h>
#include <os/OsUtil.h>
#include <net/Url.h>
#include <net/SipUserAgent.h>
#include <net/SipContactSelector.h>
#include <net/SipContactDb.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

SipContactSelector::SipContactSelector(SipUserAgent& sipUserAgent)
: m_sipUserAgent(sipUserAgent)
{

}

SipContactSelector::~SipContactSelector()
{

}

/* ============================ MANIPULATORS ============================== */

void SipContactSelector::getBestContactAddress(UtlString& contactIp,
                                               int& contactPort,
                                               SIP_TRANSPORT_TYPE transport,
                                               const UtlString& messageLocalIp,//optional
                                               const UtlString& targetIpAddress,//optional
                                               int targetPort) const//must be present if targetIpAddress is not empty
{
   if (!messageLocalIp.isNull())
   {
      contactIp = messageLocalIp;
   }
   else
   {
      contactIp = m_sipUserAgent.getDefaultIpAddress();
   }

   if(transport == SIP_TRANSPORT_TCP)
   {
      contactPort = m_sipUserAgent.getTcpPort();

      if (!targetIpAddress.isNull() && messageLocalIp.isNull())
      {
         findBestLocalContactAddress(contactIp, contactPort, targetIpAddress, transport);
      }
   }
#ifdef HAVE_SSL
   else if(transport == SIP_TRANSPORT_TLS)
   {
      contactPort = m_sipUserAgent.getTlsPort();

      if (!targetIpAddress.isNull() && messageLocalIp.isNull())
      {
         findBestLocalContactAddress(contactIp, contactPort, targetIpAddress, transport);
      }
   }
#endif
   else // UDP transport
   {
      contactPort = m_sipUserAgent.getUdpPort();

      if (!targetIpAddress.isNull())
      {
         // target ip address is know, we can use it to find a better contact
         if (!isPrivateIp(targetIpAddress))
         {
            findBestContactAddress(contactIp, contactPort, targetIpAddress, targetPort, transport);
         }
         else if (messageLocalIp.isNull()) // don't override localIp from sipMessage if target ip is private
         {
            findBestLocalContactAddress(contactIp, contactPort, targetIpAddress, transport);
         }
      } // target ip address is not know, we can't do more
   }
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void SipContactSelector::findBestLocalContactAddress(UtlString& contactIp,
                                                     int& contactPort,
                                                     const UtlString& targetIpAddress,
                                                     SIP_TRANSPORT_TYPE transport) const
{
   // message is sent to private ip, select the best local contact for that target
   UtlString interfaceName;
   if (OsNetwork::getBestInterfaceName(targetIpAddress, interfaceName))
   {
      UtlSList contacts;
      m_sipUserAgent.getContactDb().getAllForAdapterName(contacts, interfaceName, SIP_CONTACT_LOCAL, transport);
      UtlSListIterator itor(contacts);
      SipContact* pBestContact = NULL;
      while (itor())
      {
         SipContact* pContact = dynamic_cast<SipContact*>(itor.item());
         if (pContact)
         {
            if (pBestContact == NULL)
            {
               // note that if there are multiple IPs on single adapter, we are unable to select the correct local IP
               pBestContact = pContact;
               break;
            }
         }
      }
      if (pBestContact)
      {
         contactIp = pBestContact->getIpAddress();
         contactPort = pBestContact->getPort();
      }
      contacts.destroyAll();
   }
}

void SipContactSelector::findBestContactAddress(UtlString& contactIp,
                                                int& contactPort,
                                                const UtlString& targetIpAddress,
                                                int targetPort,
                                                SIP_TRANSPORT_TYPE transport) const
{
   // message is not going into private network, we may find NAT binding
   UtlBoolean bFound = OsNatAgentTask::getInstance()->findContactAddress(targetIpAddress, targetPort,
      &contactIp, &contactPort);
   // we consider NAT binding the most accurate, if it's not found, we try searching contact db
   if (!bFound)
   {
      // no NAT binding is known, select the best contact
      UtlString interfaceName;
      if (OsNetwork::getBestInterfaceName(targetIpAddress, interfaceName))
      {
         UtlSList contacts;
         m_sipUserAgent.getContactDb().getAllForAdapterName(contacts, interfaceName, SIP_CONTACT_AUTO, transport);
         UtlSListIterator itor(contacts);
         SipContact* pBestContact = NULL;
         while (itor())
         {
            SipContact* pContact = dynamic_cast<SipContact*>(itor.item());
            if (pContact)
            {
               if (pBestContact == NULL)
               {
                  pBestContact = pContact;
               }
               else if (pBestContact->getContactType() < pContact->getContactType())
               {
                  pBestContact = pContact;
               }
            }
         }
         if (pBestContact)
         {
            contactIp = pBestContact->getIpAddress();
            contactPort = pBestContact->getPort();
         }
         contacts.destroyAll();
      }
   }
}

UtlBoolean SipContactSelector::isPrivateIp(const UtlString& ipAddress) const
{
   if (ipAddress.compareTo("127.0.0.1") == 0)
   {
      return TRUE;
   }
   else if (OsUtil::isSameNetwork(ipAddress, "10.0.0.0", "255.0.0.0"))
   {
      return TRUE;
   }
   else if (OsUtil::isSameNetwork(ipAddress, "172.16.0.0", "255.240.0.0"))
   {
      return TRUE;
   }
   else if (OsUtil::isSameNetwork(ipAddress, "192.168.0.0", "255.255.0.0"))
   {
      return TRUE;
   }
   else if (OsUtil::isSameNetwork(ipAddress, "169.254.0.0", "255.255.0.0"))
   {
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}

void SipContactSelector::getBestContactUri(Url& contactUri,
                                           const UtlString& userId,
                                           SIP_TRANSPORT_TYPE transport,
                                           const UtlString& messageLocalIp,
                                           const UtlString& targetIpAddress,
                                           int targetPort) const
{

   UtlString contactIp;
   int contactPort;
   getBestContactAddress(contactIp, contactPort, transport, messageLocalIp, targetIpAddress, targetPort);

   SipContact::buildContactUri(contactUri, NULL, userId, contactIp, contactPort, transport);
}
/* ============================ FUNCTIONS ================================= */

