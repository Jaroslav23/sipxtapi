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

#ifndef OsNetworkLinux_h__
#define OsNetworkLinux_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <utl/UtlSList.h>

// DEFINES
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
class OsNetworkLinux
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /* ============================ MANIPULATORS ============================== */

   /**
    * Return this host's IP addresses.
    */
   static bool getAllLocalHostIps(const class HostAdapterAddress* localHostAddresses[],
                                  int &numAddresses);

   /**
    * Return a generated adapter name associated with the IP address.
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
   
   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // OsNetworkLinux_h__
