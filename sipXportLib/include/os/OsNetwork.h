//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _OsNetwork_h_
#define _OsNetwork_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * OsNetwork class contains static methods for querying various
 * network settings in the OS. It is implemented separately
 * for each OS. 
 */ 

#if defined(_WIN32)
#  include "os/Wnt/OsNetworkWnt.h"
   typedef class OsNetworkWnt OsNetwork;
#elif defined(__pingtel_on_posix__)
#  include "os/linux/OsNetworkLinux.h"
   typedef class OsNetworkLinux OsNetwork;
#else
#  error Unsupported target platform.
#endif

#endif  // _OsNetwork_h_
