//
// Copyright (C) 2006-2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef getWindowsDNSServers_h_
#define getWindowsDNSServers_h_

// SYSTEM INCLUDES
#include <winsock2.h>
#include <iptypes.h>

// APPLICATION INCLUDES
// DEFINES
#define MAXIPLEN 40

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
// STRUCTS
// TYPEDEFS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

#ifdef __cplusplus

extern "C" int getWindowsDNSServers(char DNSServers[][MAXIPLEN], int max, const char* szLocalIp);

#else

int getWindowsDNSServers(char DNSServers[][MAXIPLEN], int max, const char* szLocalIp);

#endif

#endif // getWindowsDNSServers_h_
