//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2007 Jaroslav Libak. All rights reserved.
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <os/OsMutexC.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


OsMutexC createMutex()
{
   return CreateMutex(NULL, FALSE, NULL);
}

enum OsStatus acquireMutex(OsMutexC mutex)
{
   DWORD    ntRes;
   DWORD    msecsTimer = INFINITE;
   enum OsStatus res = OS_FAILED;

   ntRes = WaitForSingleObject(mutex, msecsTimer);
   switch (ntRes)
   {
   case 0:
      res = OS_SUCCESS;
      break;
   case WAIT_TIMEOUT:
      res = OS_WAIT_TIMEOUT;
      break;
   case WAIT_ABANDONED:
      res = OS_WAIT_ABANDONED;
      break;
   default:
      res = OS_UNSPECIFIED;
      break;
   }

   return res;
}

enum OsStatus tryAcquireMutex(OsMutexC mutex)
{
   DWORD    ntRes;
   DWORD    msecsTimer = 0;
   enum OsStatus res = OS_FAILED;

   ntRes = WaitForSingleObject(mutex, msecsTimer);
   switch (ntRes)
   {
   case 0:
      res = OS_SUCCESS;
      break;
   case WAIT_TIMEOUT:
      res = OS_BUSY;
      break;
   case WAIT_ABANDONED:
      res = OS_WAIT_ABANDONED;
      break;
   default:
      res = OS_UNSPECIFIED;
      break;
   }

   return res;
}

enum OsStatus releaseMutex(OsMutexC mutex)
{
   if (ReleaseMutex(mutex))
      return OS_SUCCESS;
   else
      return OS_UNSPECIFIED;
}

void destroyMutex(OsMutexC mutex)
{
   CloseHandle(mutex);
}