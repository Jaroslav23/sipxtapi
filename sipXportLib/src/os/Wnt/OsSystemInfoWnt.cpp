//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <Windows.h>

// APPLICATION INCLUDES
#include "os/wnt/OsSystemInfoWnt.h"

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

OsSystemInfoWnt::~OsSystemInfoWnt(void)
{

}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

OsSystemInfoWnt::OsWindowsVersion OsSystemInfoWnt::getOsVersion()
{
   OSVERSIONINFO versionInfo;
   OsWindowsVersion version = VERSION_ERROR;

   versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

   if (GetVersionEx(&versionInfo))
   {
      version = UNKNOWN_VERSION;

      if (versionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
      {
         // The operating system is Windows Me, Windows 98, or Windows 95
         if (versionInfo.dwMajorVersion == 4)
         {
            switch (versionInfo.dwMinorVersion)
            {
            case 0:
               version = WINDOWS_95;
               break;
            case 10:
               version = WINDOWS_98;
               break;
            case 90:
               version = WINDOWS_ME;
               break;
            default: ;
            }
         }
      }
      else if (versionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
      {
         // The operating system is Windows Vista, Windows Server "Longhorn", Windows Server 2003, Windows XP, Windows 2000, or Windows NT.
         if (versionInfo.dwMajorVersion == 4)
         {
            switch (versionInfo.dwMinorVersion)
            {
            case 0:
               version = WINDOWS_NT4;
               break;
            default: ;
            }
         }
         else if (versionInfo.dwMajorVersion == 5)
         {
            switch (versionInfo.dwMinorVersion)
            {
            case 0:
               version = WINDOWS_2000;
               break;
            case 1:
               version = WINDOWS_XP;
               break;
            case 2:
               version = WINDOWS_SERVER_2003;
               break;
            default: ;
            }
         }
         else if (versionInfo.dwMajorVersion == 6)
         {
            switch (versionInfo.dwMinorVersion)
            {
            case 0:
               version = WINDOWS_VISTA;
               break;
            default:
               version = FUTURE_VERSION;
            }
         }
         else if (versionInfo.dwMajorVersion > 6)
         {
            version = FUTURE_VERSION;
         }
      }
   }

   return version;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

OsSystemInfoWnt::OsSystemInfoWnt(void)
{
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

