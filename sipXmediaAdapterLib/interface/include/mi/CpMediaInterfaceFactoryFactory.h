//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _CpMediaInterfaceFactoryFactory_h_
#define _CpMediaInterfaceFactoryFactory_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "CpMediaInterfaceFactory.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS

class OsConfigDb; // forward declaration

/**
 * Well known function that is called to create a CpMediaInterfaceFactory object.
 * This object is used to create media interfaces.
 * 
 * This function must be implemented for all "plug-in" static sipx media processing
 * libraries. 
 */
extern "C" CpMediaInterfaceFactory* sipXmediaFactoryFactory(OsConfigDb* pConfigDb);

/**
 * Destroy the singleton media factory
 */
extern "C" void sipxDestroyMediaFactoryFactory();

// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

#endif
