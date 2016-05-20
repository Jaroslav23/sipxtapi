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
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winreg.h>
#include <stdio.h>
#include <assert.h>
#include <ipexport.h>
#include <stdlib.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include <os/wnt/OsNetworkWnt.h>
#include <os/HostAdapterAddress.h>
#include <os/OsSysLog.h>
#include <os/OsNetworkAdapterInfo.h>
#include <utl/UtlString.h>

// DEFINES
#define MAXNUM_DNS_ENTRIES 40

//used by getWindowsVersion
#define WINDOWS_VERSION_ERROR 0
#define WINDOWS_VERSION_98    1
#define WINDOWS_VERSION_NT4   2
#define WINDOWS_VERSION_2000  3

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

static DWORD (WINAPI *GetNetworkParams)(PFIXED_INFO, PULONG);


static DWORD (WINAPI *GetAdaptersInfo)(PIP_ADAPTER_INFO pAdapterInfo,
                                       PULONG pOutBufLen);

static DWORD (WINAPI *GetPerAdapterInfo)(
   ULONG IfIndex,
   PIP_PER_ADAPTER_INFO pPerAdapterInfo,
   PULONG pOutBufLen
   );

static DWORD (WINAPI *GetAdapterIndex)(
                                       LPWSTR AdapterName,
                                       PULONG IfIndex
                                       );

static DWORD (WINAPI *GetInterfaceInfo)(PIP_INTERFACE_INFO pIfTable,
                                        PULONG dwOutBufLen);

static DWORD (WINAPI *GetBestInterface)(IPAddr dwDestAddr,
                                        PDWORD pdwBestIfIndex);

static HMODULE hIpHelperModule = NULL;

/**
 * Loads the iphlpapi.dll and sets any func pointers we may need.
 *
 * @return Handle of iphlpapi.dll if it could be loaded. NULL if not.
 */
static HMODULE loadIPHelperAPI()
{
   // give it extra size to prevent buffer overflow
   char caFullDLLPath[_MAX_PATH + 20]; 

   if (hIpHelperModule == NULL)
   {
      //first try loading it using the systems path
      hIpHelperModule = LoadLibrary("iphlpapi.dll");

      if (!hIpHelperModule)
      {
         //if that fails, (it shouldn't), try using the GetSystemPath func 
         GetSystemDirectory(caFullDLLPath, _MAX_PATH);
         strcat(caFullDLLPath, "\\iphlpapi.dll");

         //try again
         hIpHelperModule = LoadLibrary(caFullDLLPath);
      }

      //ok, I give up...where the heck did they put the iphlpapi.dll???????
      if (!hIpHelperModule)
      {
         OsSysLog::add(FAC_KERNEL, PRI_ERR, "Cannot find iphlpapi.dll!\n");
      }
      else
      {
         //now find IPHelper functions
         *(FARPROC*)&GetNetworkParams = GetProcAddress(hIpHelperModule, "GetNetworkParams");
         if (GetNetworkParams == NULL)
         {
            OsSysLog::add(FAC_KERNEL, PRI_ERR, "Could not get the proc address to GetNetworkParams!\n");
            FreeLibrary(hIpHelperModule);
            hIpHelperModule = NULL;
            return hIpHelperModule;
         }   

         *(FARPROC*)&GetPerAdapterInfo = GetProcAddress(hIpHelperModule,"GetPerAdapterInfo");
         if (GetPerAdapterInfo == NULL)
         {
            OsSysLog::add(FAC_KERNEL, PRI_ERR, "Could not get the proc address to GetPerAdapterInfo!\n");
            FreeLibrary(hIpHelperModule);
            hIpHelperModule = NULL;
            return hIpHelperModule;
         }   


         *(FARPROC*)&GetInterfaceInfo = GetProcAddress(hIpHelperModule,"GetInterfaceInfo");
         if (GetPerAdapterInfo == NULL)
         {
            OsSysLog::add(FAC_KERNEL, PRI_ERR, "Could not get the proc address to GetInterfaceInfo!\n");
            FreeLibrary(hIpHelperModule);
            hIpHelperModule = NULL;
            return hIpHelperModule;
         }   

         *(FARPROC*)&GetAdapterIndex = GetProcAddress(hIpHelperModule,"GetAdapterIndex");
         if (GetAdapterIndex == NULL)
         {
            OsSysLog::add(FAC_KERNEL, PRI_ERR, "Could not get the proc address to GetAdapterIndex!\n");
            FreeLibrary(hIpHelperModule);
            hIpHelperModule = NULL;
            return hIpHelperModule;
         }   

         *(FARPROC*)&GetAdaptersInfo = GetProcAddress(hIpHelperModule,"GetAdaptersInfo");
         if (GetAdaptersInfo == NULL)
         {
            OsSysLog::add(FAC_KERNEL, PRI_ERR, "Could not get the proc address to GetAdaptersInfo!\n");
            FreeLibrary(hIpHelperModule);
            hIpHelperModule = NULL;
            return hIpHelperModule;
         }

         *(FARPROC*)&GetBestInterface = GetProcAddress(hIpHelperModule,"GetBestInterface");
         if (GetBestInterface == NULL)
         {
            OsSysLog::add(FAC_KERNEL, PRI_ERR, "Could not get the proc address to GetBestInterface!\n");
            FreeLibrary(hIpHelperModule);
            hIpHelperModule = NULL;
            return hIpHelperModule;
         }
      }
   }

   return hIpHelperModule;
}

// gets DNS IPs from iphlpapi.dll
// for Win98, it returns global DNS IPs
static int getIPHelperDNSEntries(char DNSServers[][MAXIPLEN], int max, const char* szLocalIp)
{
   int ipHelperDNSServerCount = 0;

   if (loadIPHelperAPI())
   {
      // Get list of adapters and find the index of the one associated with szLocalIp
      long index = -1;
      unsigned long outBufLen = 0;
      DWORD dwResult = GetAdaptersInfo(NULL, &outBufLen);

      if (outBufLen)
      {
         PIP_ADAPTER_INFO pIpAdapterInfo = (PIP_ADAPTER_INFO)malloc(outBufLen);
         dwResult = GetAdaptersInfo(pIpAdapterInfo, &outBufLen);

         if (dwResult == ERROR_SUCCESS)
         {
            PIP_ADAPTER_INFO pNextInfoRecord = pIpAdapterInfo;

            // loop through all adapters
            while (pNextInfoRecord && index < 0)
            {
               PIP_ADDR_STRING pIpAddrString = &(pNextInfoRecord->IpAddressList);

               // loop through all IPs of the adapter
               while (pIpAddrString)
               {
                  if (strcmp(szLocalIp, pIpAddrString->IpAddress.String) == 0)
                  {
                     // we found the index
                     index = static_cast<long>(pNextInfoRecord->Index);
                     break;
                  }

                  pIpAddrString = pIpAddrString->Next;
               }

               pNextInfoRecord = pNextInfoRecord->Next;
            }
         }

         free(pIpAdapterInfo);
      }

      if (index >= 0)
      {
         // now that we have the index, we
         // can call GetPerAdapterInfo
         outBufLen = 0;
         GetPerAdapterInfo(index, NULL, &outBufLen);

         if (outBufLen)
         {
            IP_PER_ADAPTER_INFO* pPerAdapterInfo = (IP_PER_ADAPTER_INFO*) malloc(outBufLen);
            dwResult = GetPerAdapterInfo(index, pPerAdapterInfo, &outBufLen);

            if (dwResult == ERROR_SUCCESS)
            {
               IP_ADDR_STRING* pDns = &pPerAdapterInfo->DnsServerList;

               // loop through all DNS IPs
               while (pDns && ipHelperDNSServerCount < max)
               {
                  SAFE_STRNCPY(DNSServers[ipHelperDNSServerCount], pDns->IpAddress.String,
                     sizeof(DNSServers[ipHelperDNSServerCount]));
                  ipHelperDNSServerCount++;

                  pDns = pDns->Next;
               }
            }

            free(pPerAdapterInfo);
         }
      }
   }

   return ipHelperDNSServerCount;
}

// gets global DNS IPs from registry
static int getDNSEntriesFromRegistry(char regDNSServers[][MAXIPLEN], int max)
{
   int retRegDNSServerCount = 0;
   const char *strParametersKey    = "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters";
   const char *strDhcpNameServerValue       = "DhcpNameServer";
   const char *strNameServerValue           = "NameServer";
   HKEY hKey;
   BYTE data[255];
   DWORD cbData;
   DWORD dataType;
   DWORD err;

   err = RegOpenKeyEx(
      HKEY_LOCAL_MACHINE,  // handle to open key
      strParametersKey,  // subkey name
      0,   // reserved
      KEY_READ, // security access mask
      &hKey    // handle to open key
      );

   if (err == ERROR_SUCCESS)
   {
      cbData = sizeof(data);

      // first try global DHCP DNS IPs
      err = RegQueryValueEx(
         hKey,                      // handle to key
         strDhcpNameServerValue,    // value name
         0,                         // reserved
         &dataType,                 // type buffer
         data,                      // data buffer
         &cbData);                  // size of data buffer

      // if error or string is empty
      if (err != ERROR_SUCCESS || !data[0])
      {
         //try a different value
         err = RegQueryValueEx(
            hKey,                // handle to key
            strNameServerValue,  // value name
            0,                   // reserved
            &dataType,           // type buffer
            data,                // data buffer
            &cbData);            // size of data buffer
      }

      if (err == ERROR_SUCCESS)
      {
         // separator might be a space or ,
         char *seps = " ,";  // used for parsing the ip addresses
         char *token = NULL;   // pointer to ip addresses when parsing
         char *nexttoken; // context for _s function

         // we need to break it up on NT.  It puts all the IP's on one line.
         // find the first token
         token = strtok_s((char *)data, seps, &nexttoken);

         while (token && retRegDNSServerCount < max)
         {
            SAFE_STRNCPY(regDNSServers[retRegDNSServerCount], token, sizeof(regDNSServers[retRegDNSServerCount]));
            retRegDNSServerCount++;

            // sarch for the next one
            token = strtok_s(NULL, seps, &nexttoken);
         }
      }
      else
      {
         OsSysLog::add(FAC_KERNEL, PRI_ERR, "Error reading values from registry in func: getDNSEntriesFromRegistry\n"); 
      }

      RegCloseKey(hKey);
   }
   else
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "Error opening registry in func: getDNSEntriesFromRegistry\n");
   }   

   return retRegDNSServerCount;
}

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

bool OsNetworkWnt::getAllLocalHostIps(const class HostAdapterAddress* localHostAddresses[], int &numAddresses)
{
   bool rc = false;

   if (loadIPHelperAPI())
   {
      unsigned long outBufLen = 0;
      // first find out how big buffer we need
      DWORD dwResult = GetAdaptersInfo(NULL, &outBufLen);

      if (dwResult == ERROR_BUFFER_OVERFLOW)
      {
         PIP_ADAPTER_INFO pIpAdapterInfo = (PIP_ADAPTER_INFO)malloc(outBufLen);
         dwResult = GetAdaptersInfo(pIpAdapterInfo, &outBufLen);

         if (dwResult == ERROR_SUCCESS)
         {
            int maxAddresses = numAddresses; // remember how many we want
            rc = true;
            numAddresses = 0; // we have 0 so far
            PIP_ADAPTER_INFO pNextInfoRecord = pIpAdapterInfo;

            while (pNextInfoRecord && (numAddresses < maxAddresses))
            {
               PIP_ADDR_STRING pNextAddress = &pNextInfoRecord->IpAddressList;

               // loop through all IPs
               while (pNextAddress && (numAddresses < maxAddresses))
               {
                  const char *szAddr = pNextAddress->IpAddress.String;

                  // ignore the loopback address
                  if (strncmp(szAddr, "127.0.0.1", 9) != 0 &&
                     strncmp(szAddr, "0.0.0.0", 7) != 0)
                  {
                     // IP address is ok, remember it
                     localHostAddresses[numAddresses] = new HostAdapterAddress(pNextInfoRecord->AdapterName, szAddr);
                     numAddresses++;
                  }

                  pNextAddress = pNextAddress->Next;
               }

               pNextInfoRecord = pNextInfoRecord->Next;
            }
         }

         free((void*)pIpAdapterInfo);
      }
      else
      {
         OsSysLog::add(FAC_KERNEL, PRI_ERR, "Cannot get host IPs, unexpected error code: %i\n", dwResult);
      }
   }
   else
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "Cannot get host IPs, IP helper API couldn't be loaded\n");
   }

   return rc;
}

bool OsNetworkWnt::getAdapterName(UtlString &adapterName, const UtlString &ipAddress)
{
   bool rc = false;

   if (ipAddress == "127.0.0.1")
   {
      // this must be loopback
      rc = true;
      adapterName = "loopback";
      return rc;
   }
   else if (loadIPHelperAPI())
   {
      unsigned long outBufLen = 0;
      // first find out how big buffer we need
      DWORD dwResult = GetAdaptersInfo(NULL, &outBufLen);

      if (dwResult == ERROR_BUFFER_OVERFLOW)
      {
         PIP_ADAPTER_INFO pIpAdapterInfo = (PIP_ADAPTER_INFO)malloc(outBufLen);
         dwResult = GetAdaptersInfo(pIpAdapterInfo, &outBufLen);

         if (dwResult == ERROR_SUCCESS)
         {
            PIP_ADAPTER_INFO pNextInfoRecord = pIpAdapterInfo;
            unsigned int adapterId = 0;

            while (pNextInfoRecord && !rc)
            {
               PIP_ADDR_STRING pNextAddress = &(pNextInfoRecord->IpAddressList);

               while (pNextAddress)
               {
                  const char *szAddr = pNextAddress->IpAddress.String;
                  // if the target matches this address or if the target is any
                  if (ipAddress == szAddr || ipAddress == "0.0.0.0")
                  {
                     adapterName = pNextInfoRecord->AdapterName;
                     rc = true;
                     break;
                  }

                  pNextAddress = pNextAddress->Next;
               }

               adapterId++;
               pNextInfoRecord = pNextInfoRecord->Next;
            }
         }

         free((void*)pIpAdapterInfo);
      }
      else
      {
         OsSysLog::add(FAC_KERNEL, PRI_ERR, "Cannot convert IP address to adapter name, unexpected error code: %i\n", dwResult);
      }
   }
   else
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "Cannot convert IP address to adapter name, IP helper API couldn't be loaded\n");
   }

   return rc;
}

bool OsNetworkWnt::getAdapterList(UtlSList& adapterList)
{
   bool rc = false;
   adapterList.destroyAll();

   if (loadIPHelperAPI())
   {
      unsigned long outBufLen = 0;
      // first find out how big buffer we need
      DWORD dwResult = GetAdaptersInfo(NULL, &outBufLen);

      if (dwResult == ERROR_BUFFER_OVERFLOW)
      {
         PIP_ADAPTER_INFO pIpAdapterInfo = (PIP_ADAPTER_INFO)malloc(outBufLen);
         dwResult = GetAdaptersInfo(pIpAdapterInfo, &outBufLen);

         if (dwResult == ERROR_SUCCESS)
         {
            PIP_ADAPTER_INFO pNextInfoRecord = pIpAdapterInfo;

            while (pNextInfoRecord && !rc)
            {
               UtlSList* pIpAddresses = new UtlSList();
               PIP_ADDR_STRING pNextAddress = &(pNextInfoRecord->IpAddressList);

               while (pNextAddress)
               {
                  // remember all ip addresses of adapter
                  pIpAddresses->append(new UtlString(pNextAddress->IpAddress.String));
                  pNextAddress = pNextAddress->Next;
               }

               OsNetworkAdapterInfo *AdapterInfo = new OsNetworkAdapterInfo(pNextInfoRecord->AdapterName,
                  pNextInfoRecord->Description, pIpAddresses);
               adapterList.append(AdapterInfo);

               pNextInfoRecord = pNextInfoRecord->Next;
            }

            // always append loopback at the end
            UtlSList* ploopBackAddresses = new UtlSList();
            ploopBackAddresses->append(new UtlString("127.0.0.1"));
            adapterList.append(new OsNetworkAdapterInfo("loopback", "loopback", ploopBackAddresses));
            rc = true;
         }

         free((void*)pIpAdapterInfo);
      }
      else
      {
         OsSysLog::add(FAC_KERNEL, PRI_ERR, "Cannot get adapter list, unexpected error code: %i\n", dwResult);
      }
   }
   else
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "Cannot get adapter list, IP helper API couldn't be loaded\n");
   }

   return rc;
}

bool OsNetworkWnt::getBestInterfaceName(const UtlString& targetIpAddress, UtlString& interfaceName)
{
   bool rc = false;
   interfaceName.clear();

   if (loadIPHelperAPI())
   {
      IPAddr dwDestAddr = inet_addr(targetIpAddress.data());
      DWORD dwBestIfIndex = -1;
      DWORD dwRes = GetBestInterface(dwDestAddr, &dwBestIfIndex);
      if (dwRes == NO_ERROR)
      {
         // get list of all interfaces and find the correct one
         unsigned long outBufLen = 0;
         if ((dwRes = GetInterfaceInfo(NULL, &outBufLen)) == ERROR_INSUFFICIENT_BUFFER)
         {
            PIP_INTERFACE_INFO pIpInterfaceInfo = (PIP_INTERFACE_INFO)malloc(outBufLen);
            dwRes = GetInterfaceInfo(pIpInterfaceInfo, &outBufLen);
            if (dwRes == NO_ERROR)
            {
               // loop through all interfaces
               for (long i = 0; i < pIpInterfaceInfo->NumAdapters; i++)
               {
                  if (pIpInterfaceInfo->Adapter[i].Index == dwBestIfIndex)
                  {
                     size_t convertedChars = 0;
                     char adapterName[MAX_ADAPTER_NAME_LENGTH + 4];
                     memset(adapterName, 0, sizeof(adapterName));
                     wcstombs_s(&convertedChars, adapterName, sizeof(adapterName), pIpInterfaceInfo->Adapter[i].Name, _TRUNCATE);
                     // on windows vista we get guid, on older we get this garbage
                     UtlString sAdapterName(adapterName);
                     if (sAdapterName.first("\\DEVICE\\TCPIP_") == 0)
                     {
                        sAdapterName.remove(0, strlen("\\DEVICE\\TCPIP_"));
                     }
                     interfaceName = sAdapterName;
                     rc = true;
                     break;
                  }
               }
            }
            else
            {
               OsSysLog::add(FAC_KERNEL, PRI_ERR, "Cannot determine the best interface name, GetInterfaceInfo failed with error %d\n", dwRes);
            }
            // Free the PIP_INTERFACE_INFO structure
            free(pIpInterfaceInfo);
         }
         else
         {
            OsSysLog::add(FAC_KERNEL, PRI_ERR, "Cannot determine the best interface name, GetInterfaceInfo failed with error %d\n", dwRes);
         }
      }
      else
      {
         OsSysLog::add(FAC_KERNEL, PRI_ERR, "Cannot determine the best interface name, GetBestInterface failed with error %d\n", dwRes);
      }
   }
   else
   {
      OsSysLog::add(FAC_KERNEL, PRI_ERR, "Cannot determine the best interface name, IP helper API couldn't be loaded\n");
   }

   return rc;
}

int OsNetworkWnt::getWindowsDNSServers(char DNSServers[][MAXIPLEN], int max, const char* szLocalIp)
{
   int finalDNSServerCount = 0; //number of dns entries returned to user

   // retrieve the DNS entries from a MS provided DLL
   // This func will also load the dll if on win98 or NT 2000
   finalDNSServerCount = getIPHelperDNSEntries(DNSServers, max, szLocalIp);

   if (finalDNSServerCount > 0)
   {
      return finalDNSServerCount;
   }
   else
   {
      // this one returns global DNS IPs from registry
      finalDNSServerCount = getDNSEntriesFromRegistry(DNSServers, max);
   }

   //return the number of DNS entries found
   return finalDNSServerCount;
}

///////////////////////////////////////////
//
// gets the domain name
//
// returns:  0 on failure  1 on success
//
//
//////////////////////////////////////////
int OsNetworkWnt::getWindowsDomainName(char *domain_name)
{
   DWORD Err;
   DWORD NetworkInfoSize;
   PFIXED_INFO pNetworkInfo;

   int retval = 0;

   *domain_name = '\0';

   if (loadIPHelperAPI())  //inits iphlpapi and returns true if dll loaded
   {
      if (GetNetworkParams != NULL)
      {
         //force size to 0 so the GetNetworkParams gets the correct size
         NetworkInfoSize = 0;
         if( ( Err = GetNetworkParams( NULL, &NetworkInfoSize ) ) != 0 )
         {
            if( Err != ERROR_BUFFER_OVERFLOW )
            {
               printf( "GetNetworkParams sizing failed with error %d\n", Err );
               return 0;
            }
         }

         // Allocate memory from sizing information
         if( ( pNetworkInfo = (PFIXED_INFO)GlobalAlloc( GPTR, NetworkInfoSize ) ) == NULL )
         {
            printf( "Memory allocation error\n" );
            return 0;
         }

         // Get actual network params
         if( ( Err = GetNetworkParams( pNetworkInfo, &NetworkInfoSize ) ) != 0 )
         {
            printf( "GetNetworkParams failed with error %d\n", Err );
            return 0;
         }

         strcpy(domain_name, pNetworkInfo->DomainName);

         //free the memory
         GlobalFree(pNetworkInfo);   // handle to global memory object

      }

   }

   return retval;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
