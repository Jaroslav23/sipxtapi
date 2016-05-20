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

#ifndef OsMutexC_h__
#define OsMutexC_h__

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <os/OsStatus.h>

typedef HANDLE OsMutexC;

#else

#include <os/OsStatus.h>
#include "os/linux/OsLinuxDefs.h"
#include "os/linux/pt_mutex.h"

typedef pt_mutex_t OsMutexC;

#endif

OsMutexC createMutex();
//: Creates new mutex

enum OsStatus acquireMutex(OsMutexC mutex);
//:Block the task until the mutex is acquired

enum OsStatus tryAcquireMutex(OsMutexC mutex);
//:Conditionally acquire the mutex (i.e., don't block)

enum OsStatus releaseMutex(OsMutexC mutex);
//:Release the mutex

void destroyMutex(OsMutexC mutex);
//:Destroy the mutex

#endif // OsMutexC_h__
