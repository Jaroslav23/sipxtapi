//
// Copyright (C) 2005-2007 SIPez LLC.
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

// SYSTEM INCLUDES
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif /* _WIN32 */

// APPLICATION INCLUDES
// DEFINES
#if defined _WIN32 && !defined WINCE
#define strcasecmp stricmp
#endif

// GLOBAL VARIABLES
// EXTERNAL VARIABLES
// EXTERNAL FUNCTIONS
// STRUCTURES
// GLOBALS

/* ============================ FUNCTIONS ================================= */

#if defined _WIN32 && defined SIPXTAPI_EXPORTS

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
                      )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

#endif /* defined(_WIN32) */

