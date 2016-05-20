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
#include "os/linux/pt_mutex.h"
#include "os/OsMutexC.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

OsMutexC createMutex()
{
  OsMutexC mutex;
  
  int res;

  res = pt_mutex_init(&mutex);
  assert(res == POSIX_OK);
  
  return mutex;
}

enum OsStatus acquireMutex(OsMutexC mutex)
{
  enum OsStatus status;
  
  status = (pt_mutex_lock(&mutex) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;
  
  return status;
}

enum OsStatus tryAcquireMutex(OsMutexC mutex)
{
  enum OsStatus status;
  
  status = (pt_mutex_trylock(&mutex) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;
  
  return status;
}

enum OsStatus releaseMutex(OsMutexC mutex)
{
  enum OsStatus status;
  
  status = (pt_mutex_unlock(&mutex) == POSIX_OK) ? OS_SUCCESS : OS_BUSY;
  
  return status;
}

void destroyMutex(OsMutexC mutex)
{
  int res;
  res = pt_mutex_destroy(&mutex);
  
  assert(res == POSIX_OK);
}

