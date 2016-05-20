//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
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
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsMsgPool.h"
#include "utl/UtlString.h"
#include "os/OsSysLog.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

   // Default constructor.  model is a message of the single type that
   // will be contained in the pool, and its createCopy virtual method
   // will be used to populate the pool.  The caller disposes of model
OsMsgPool::OsMsgPool(const char* name,
   const OsMsg& model,
   int initialCount,
   int increment,
   OsMsgPoolSharing sharing)
: mIncrement(increment)
, mNext(0)
{
   int i;
   OsMsg* pMsg;
   mpMutex = NULL;
   mCurrentCount = 0;
   mpModel = model.createCopy();
   mpModel->setReusable(TRUE);
   mpModel->setInUse(FALSE);

   mpName = new UtlString((NULL == name) ? "Unknown" : name);

   mInitialCount = (initialCount > 1) ? initialCount : 10;
   mIncrement = (mIncrement > 0) ? mIncrement : 1;

   mpElts = (OsMsg**)malloc(sizeof(OsMsg*)*mInitialCount);

   for (i = 0; i < mInitialCount; i++)
   {
      pMsg = mpModel->createCopy();

      if (pMsg)
      {
         pMsg->setReusable(TRUE);
         pMsg->setInUse(FALSE);
         mpElts[i] = pMsg;
         mCurrentCount++;
      }
   }

   if (MULTIPLE_CLIENTS == sharing) {
      mpMutex = new OsMutex(OsMutex::Q_PRIORITY |
                            OsMutex::DELETE_SAFE |
                            OsMutex::INVERSION_SAFE);
      assert(mpMutex);
   }
}

// Destructor
OsMsgPool::~OsMsgPool()
{
   // Hmmm...
   int i;
   OsMsg* pMsg;

   if (mpMutex) mpMutex->acquire();

   for (i = 0; i < mCurrentCount; i++)
   {
      pMsg = mpElts[i];

      if (pMsg)
      {
         pMsg->setReusable(FALSE);
         if (!pMsg->isMsgInUse())
         {
            mpElts[i] = NULL;
            delete pMsg;
         }
      }
   }

   free(mpElts);

   mpModel->setReusable(FALSE);
   delete mpModel;
   delete mpName;
   if (mpMutex) mpMutex->release();
   delete mpMutex;
}

/* ============================ MANIPULATORS ============================== */

   // Find and return an available element of the pool, creating more if
   // necessary and permitted.  Return NULL if failure.

OsMsg* OsMsgPool::findFreeMsg()
{
   int i;
   OsMsg* pMsg = NULL;
   OsMsg* ret = NULL;

   // If there is a mutex for this pool, acquire it before doing any work.
   if (mpMutex)
   {
      mpMutex->acquire();
   }

   // Scan mNext through the table looking for a message that is
   // allocated and not in use.
   for (i = 0; ((i < mCurrentCount) && !ret); i++)
   {
      // Examine the element.
      pMsg = mpElts[mNext];
      if (pMsg && !pMsg->isMsgInUse())
      {
         pMsg->setInUse(TRUE);
         ret = pMsg;
      }
      // Advance mNext, wrapping around if it reaches mCurrentCount.
      mNext++;
      if (mNext >= mCurrentCount)
      {
         mNext = 0;
      }
   }

   // If no free message was found, extend pool
   if (!ret)
   {
      int limit;

      mNext = mCurrentCount;
      limit = mCurrentCount + mIncrement;

      mpElts = (OsMsg**)realloc(mpElts, sizeof(OsMsg*)*limit);

      if (mpElts)
      {
         // Create the new elements.
         for (i = mCurrentCount; i < limit; i++)
         {
            pMsg = mpModel->createCopy();
            if (pMsg)
            {
               pMsg->setReusable(TRUE);
               pMsg->setInUse(FALSE);
               mpElts[i] = pMsg;
               mCurrentCount++;
            }
         }

         // take the 1st next message
         ret = mpElts[mNext];
         assert(ret);

         if (ret && !ret->isMsgInUse())
         {
            ret->setInUse(TRUE);
         }
         mNext++;

         if (mNext >= mCurrentCount)
         {
            mNext = 0;
         }
      }
   }

   // If there is a mutex for this pool, release it.
   if (mpMutex)
   {
      mpMutex->release();
   }
   return ret;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

// Return the number of items in use.
int OsMsgPool::getNoInUse(void)
{
   int i, count;

   if (mpMutex)
   {
      mpMutex->acquire();
   }

   count = 0;
   for (i = 0; i < mCurrentCount; i++)
   {
      if (mpElts[i] && mpElts[i]->isMsgInUse())
      {
         count++;
      }
   }

   if (mpMutex)
   {
      mpMutex->release();
   }

   return count;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
