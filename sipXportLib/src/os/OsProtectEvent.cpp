//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif

#include "os/OsProtectEvent.h"
#include "os/OsUtil.h"
#include "os/OsSysLog.h"
#include <assert.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/* ============================ CREATORS ================================== */

OsProtectedEvent::OsProtectedEvent(const intptr_t userData)
: OsEvent(userData)
, mRefSem(OsBSem::Q_PRIORITY, OsBSem::FULL)
, mIntData(0)
, mIntData2(0)
, mRef(0)
{
}

OsProtectedEvent::~OsProtectedEvent()
{
//   OsSysLog::add(FAC_KERNEL, PRI_DEBUG, "Warning OsProtectedEvent deleted");
   mStringData = OsUtil::NULL_OS_STRING;
}

/* ============================ MANIPULATORS ============================== */

OsStatus OsProtectedEvent::signal(const intptr_t eventData)
{
   assert(mRef > 0);
   return OsEvent::signal(eventData);
}

// Reset the event so that it may be signaled again.
// Return OS_NOT_SIGNALED if the event has not been signaled (or has
// already been cleared), otherwise return OS_SUCCESS.
OsStatus OsProtectedEvent::reset(void)
{
   OsStatus res;
   res = OsEvent::reset();
   mStringData = OsUtil::NULL_OS_STRING;
   mIntData = 0;
   mIntData2 = 0;
   return res;
}

// Wait for the event to be signaled.
// Return OS_BUSY if the timeout expired, otherwise return OS_SUCCESS.
OsStatus OsProtectedEvent::wait(int msgId, const OsTime& rTimeout)
{
   OsStatus res;

   mIntData2 = msgId;
   res = OsEvent::wait(rTimeout);
   assert(mRef > 0);
   return res;
}

void OsProtectedEvent::setStringData(UtlString& rStringData)
{
   mStringData = OsUtil::NULL_OS_STRING;
   if (!rStringData.isNull())
   {
      mStringData = rStringData;
   }
}

void OsProtectedEvent::setIntData(int data)
{
   mIntData = data;
}

void OsProtectedEvent::setIntData2(int data)
{
   mIntData2 = data;
}

void OsProtectedEvent::setInUse(UtlBoolean inUse)
{
   mRefSem.acquire();
   mRef = inUse;
   if (!inUse)
      reset();
   mRefSem.release();
}

/* ============================ ACCESSORS ================================= */

OsStatus OsProtectedEvent::getStringData(UtlString& data)
{
   if (!mStringData.isNull())
   {
      data.remove(0);
      data.append(mStringData.data());
   }

   return OS_SUCCESS;
}

OsStatus OsProtectedEvent::getIntData(int& data)
{
   data = mIntData;
   return OS_SUCCESS;
}

OsStatus OsProtectedEvent::getIntData2(int& data)
{
   data = mIntData2;
   return OS_SUCCESS;
}

/* ============================ INQUIRY =================================== */

UtlBoolean OsProtectedEvent::isInUse()
{
   return (mRef > 0);
}
