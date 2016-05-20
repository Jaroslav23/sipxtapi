//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include "utl/UtlVoidPtr.h"
#include "utl/UtlInt.h"
#include "tapi/SipXHandleMap.h"
#include "utl/UtlHashMapIterator.h"

#define MAX_ALLOC_TRIES 100

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor

SipXHandleMap::SipXHandleMap(int startingHandle, int avoidHandle)
   : mLock(OsMutex::Q_FIFO)
   , mNextHandle(startingHandle)
   , mAvoidHandle(avoidHandle)
{
}

// Destructor

SipXHandleMap::~SipXHandleMap()
{
}

/* ============================ MANIPULATORS ============================== */


void SipXHandleMap::addHandleRef(SIPXHANDLE hHandle)
{
   lock();

   UtlInt handle(hHandle);

   UtlInt* count = dynamic_cast<UtlInt*>(mLockCountHash.findValue(&handle));

   if (!count)
   {
      mLockCountHash.insertKeyAndValue(new UtlInt(hHandle), new UtlInt(1));
   }
   else
   {
      (*count)++;
   }

   unlock();
}


void SipXHandleMap::releaseHandleRef(SIPXHANDLE hHandle)
{
   lock();

   UtlInt handle(hHandle);

   UtlInt* pCount = dynamic_cast<UtlInt*>(mLockCountHash.findValue(&handle));

   if (pCount)
   {
      (*pCount)--;
   }

   unlock();
}


void SipXHandleMap::lock() const
{
   mLock.acquire();
}


void SipXHandleMap::unlock() const
{
   mLock.release();
}


UtlBoolean SipXHandleMap::allocHandle(SIPXHANDLE& hHandle, const void* pData)
{
   UtlBoolean res = FALSE;
   UtlVoidPtr* pValue = NULL;
   UtlBoolean isKeyTaken = FALSE;
   UtlInt key;
   int counter = 0;

   lock();

   for (counter = 0; counter < MAX_ALLOC_TRIES; counter++)
   {
      hHandle = mNextHandle++;
      if (hHandle == mAvoidHandle)
      {
         continue;
      }
      key.setValue(hHandle);
      isKeyTaken = contains(&key);

      if (!isKeyTaken)
      {
         res = TRUE;
         break;
      }
   }

   if (counter > 5)
   {
      OsSysLog::add(FAC_SIPXTAPI, PRI_WARNING, 
         "Something is wrong, handle map is too dense");
   }

   if (res)
   {
      insertKeyAndValue(new UtlInt(hHandle), new UtlVoidPtr((void*)pData));
      addHandleRef(hHandle);
   }

   unlock();
   return res;
}


const void* SipXHandleMap::findHandle( SIPXHANDLE handle ) const
{
   lock();

   const void* pRC = NULL;
   UtlInt key(handle);
   UtlVoidPtr* pValue;

   pValue = dynamic_cast<UtlVoidPtr*>(findValue(&key));

   if (pValue)
   {
      pRC = pValue->getValue();
   }

   unlock();

   return pRC;
}


const void* SipXHandleMap::removeHandle(SIPXHANDLE handle) 
{
   lock();

   releaseHandleRef(handle);
   const void* pRC = NULL;
   UtlInt key(handle);

   UtlInt* pCount = dynamic_cast<UtlInt*>(mLockCountHash.findValue(&key));

   if (!pCount || pCount->getValue() < 1)
   {
      UtlVoidPtr* pValue;

      pValue = dynamic_cast<UtlVoidPtr*>(findValue(&key));
      if (pValue)
      {
         pRC = pValue->getValue();
         destroy(&key);
      }

      if (pCount)
      {
         mLockCountHash.destroy(&key);
      }
   }

   unlock();
   return pRC;
}


/* ============================ ACCESSORS ================================= */

void SipXHandleMap::dump() 
{
   lock();
   UtlHashMapIterator itor(*this);
   UtlInt* pKey;
   UtlVoidPtr* pValue;

   while ((pKey = dynamic_cast<UtlInt*>(itor())))
   {
      pValue = dynamic_cast<UtlVoidPtr*>(findValue(pKey));
      printf("\tkey=%08X, value=%08X\n", pKey->getValue(),
             pValue ? pValue->getValue() : 0);
   }

   unlock();
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

