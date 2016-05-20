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

#ifndef OsNetworkWnt_h__
#define OsNetworkWnt_h__

// SYSTEM INCLUDES
#include <winsock2.h>
#include <iptypes.h>
#include <time.h>

// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <utl/UtlSList.h>

// DEFINES
#define MAXIPLEN 40
#define MAX_ADAPTERS 30

#ifndef MAX_ADAPTER_NAME_LENGTH
#define MAX_ADAPTER_NAME_LENGTH 256
#endif

typedef struct _AdapterInfo
{
   char AdapterName[MAX_ADAPTER_NAME_LENGTH + 4];          //long adapter name
   char IpAddress[MAXIPLEN];                               //ip address of adapter
   BYTE MacAddress[MAX_ADAPTER_NAME_LENGTH];               //mac address of adapter
} AdapterInfoRec, *pAdapterInfoRec;

// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * Class providing information about network in a system.
 */
class OsNetworkWnt
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /* ============================ MANIPULATORS ============================== */

   /**
    * Gets host IP addresses
    *
    * @param localHostAddresses Array of pointers to const HostAdapterAddress with IP
    *        addresses
    * @param numAddresses Max number of IP addresses passed in, number of actual IP
    *        addresses passed out
    *
    * @return True if successful
    */
   static bool getAllLocalHostIps(const class HostAdapterAddress* localHostAddresses[], int &numAddresses);

   /**
    * Finds adapter name for given IP address.
    *
    * @param adapterName Returns adapter name that belongs to IP address.
    * @param ipAddress IP address that we want to look up adapter name for.
    *
    * @return true if successful
    */
   static bool getAdapterName(UtlString &adapterName, const UtlString &ipAddress);

   /**
    * Returns list of all network adapters present in system. Caller
    * is responsible for deleting adapterList entries after it is done.
    *
    * @param adapterList list with OsNetworkAdapterInfo entries
    */
   static bool getAdapterList(UtlSList& adapterList);

   /**
    * Returns name of the best local interface for sending packets to given targetIpAddress.
    *
    * @return true if the best interface name could be determined, false otherwise
    */
   static bool getBestInterfaceName(const UtlString& targetIpAddress, UtlString& interfaceName);

   /**
    * Gets IP addresses of DNS servers for the given local IP.
    *
    * Only implemented for Windows.
    *
    * @param DNSServers Array of DNS server IPs
    * @param max Maximum number of DNS server IPs to retrieve
    * @param szLocalIp Local IP address to find DNS servers for
    *
    * @return Number of DNS servers found
    */
   static int getWindowsDNSServers(char DNSServers[][MAXIPLEN], int max, const char* szLocalIp);

   /**
    * Gets the domain name.
    *
    * Only implemented for Windows.
    */
   static int getWindowsDomainName(char *domain_name);

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // OsNetworkWnt_h__
