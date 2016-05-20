//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
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
#include <assert.h>

// APPLICATION INCLUDES
#include <utl/UtlInit.h>

#include <utl/UtlHashMapIterator.h>
#include "net/SipPublishContentMgr.h"
#include "net/SipSubscribeServer.h"
#include "tapi/SipXPublish.h"
#include "tapi/SipXHandleMap.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
SipXHandleMap gPubHandleMap(1, SIPX_PUB_NULL);  /**< Global Map of Published (subscription server) event data handles */


// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

/**
* Finds publisher and locks it.
*/

SIPX_PUBLISH_DATA* sipxPublishLookup(const SIPX_PUB hPub,
                                     SIPX_LOCK_TYPE type,
                                     const OsStackTraceLogger& oneBackInStack)
{
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxPublishLookup", oneBackInStack);
   SIPX_PUBLISH_DATA* pRC;    
   OsStatus status = OS_FAILED;

   gPubHandleMap.lock();
   pRC = (SIPX_PUBLISH_DATA*)gPubHandleMap.findHandle(hPub);

   if(pRC)
   {
      switch (type)
      {
      case SIPX_LOCK_READ:
         status = pRC->mutex.acquire();
         assert(status == OS_SUCCESS);
         break;
      case SIPX_LOCK_WRITE:
         status = pRC->mutex.acquire();
         assert(status == OS_SUCCESS);
         break;
      default:
         break;
      }
   }

   gPubHandleMap.unlock();

   return pRC;
}

/**
* Releases publisher lock.
*/

void sipxPublishReleaseLock(SIPX_PUBLISH_DATA* pData,
                            SIPX_LOCK_TYPE type,
                            const OsStackTraceLogger& oneBackInStack) 
{
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxPublishReleaseLock", oneBackInStack);
   OsStatus status;

   if (pData)
   {
      switch (type)
      {
      case SIPX_LOCK_READ:
         status = pData->mutex.release();
         assert(status == OS_SUCCESS);
         break;
      case SIPX_LOCK_WRITE:
         status = pData->mutex.release();
         assert(status == OS_SUCCESS);
         break;
      default:
         break;
      }
   }
}

/**
 *  Remove/Destroy all Publishers
 */

void sipxPublisherDestroyAll(const SIPX_INST hInst) 
{
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxPublisherDestroyAll");
   gPubHandleMap.lock() ;

   UtlHashMapIterator pubIter(gPubHandleMap);
   UtlInt* pKey;
   UtlVoidPtr* pValue;
   SIPX_PUB hPub;        

   while ((pKey = dynamic_cast<UtlInt*>(pubIter())))
   {
      pValue = dynamic_cast<UtlVoidPtr*>(gPubHandleMap.findValue(pKey));
      assert(pValue);

      hPub = (SIPX_PUB)pValue->getValue();

      if (hPub)
      {
         bool bRemove = false;

         SIPX_PUBLISH_DATA* pData = sipxPublishLookup(hPub, SIPX_LOCK_READ, logItem);
         if (pData)
         {
            if (pData->pInst == hInst)
            {
               bRemove = true;
            }
            sipxPublishReleaseLock(pData, SIPX_LOCK_READ, logItem);
         }

         if (bRemove)
         {
            sipxPublisherDestroy(hPub, NULL, NULL, 0);
         }
      }
   }

   gPubHandleMap.unlock();
}


void sipxPublisherFreeObject(const SIPX_PUB hPub)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxPublisherFreeObject");

   gPubHandleMap.lock();

   SIPX_PUBLISH_DATA* pData = sipxPublishLookup(hPub, SIPX_LOCK_WRITE, stackLogger);

   if (pData)
   {
      gPubHandleMap.removeHandle(hPub);
      delete pData;
   }   

   gPubHandleMap.unlock();
}

/***************************************************************************
* Public Publisher Functions
***************************************************************************/

SIPXTAPI_API SIPX_RESULT sipxPublisherDestroy(const SIPX_PUB hPub,
                                              const char* szContentType,
                                              const char* pFinalContent,
                                              const int nContentLength)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxPublisherDestroy");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxDestroyPublisher hPub=%d szContentType=\"%s\" pFinalContent=\"%s\" nContentLength=%d",
      hPub,
      szContentType ? szContentType : "<null>",
      pFinalContent ? pFinalContent : "<null>",
      nContentLength);

   SIPX_RESULT sipXresult = SIPX_RESULT_FAILURE;
   UtlBoolean unPublish = FALSE;

   if(szContentType && *szContentType &&
      pFinalContent && *pFinalContent &&
      (nContentLength > 0 || nContentLength == -1))
   {
      unPublish = TRUE;
      sipxPublisherUpdate(hPub, szContentType, pFinalContent, nContentLength);
   }

   SIPX_PUBLISH_DATA* pData = sipxPublishLookup(hPub, SIPX_LOCK_WRITE, stackLogger);

   if(pData)
   {
      SipSubscribeServer* pSubscribeServer = pData->pInst->pSubscribeServer;
      UtlString resourceId(pData->resourceId);
      UtlString eventType(pData->eventType);

      sipxPublishReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);

      if(nContentLength > 0 &&
         (!szContentType || *szContentType == '\000' ||
          !pFinalContent || *pFinalContent == '\000'))
      {
         unPublish = FALSE;
         sipXresult = SIPX_RESULT_INVALID_ARGS;
      }

      if(unPublish && pSubscribeServer)
      {
         SipPublishContentMgr* publishMgr = pSubscribeServer->getPublishMgr(eventType);

         if(publishMgr)
         {
            // Publish the state change
            publishMgr->unpublish(resourceId, eventType, eventType);
         }

         sipxPublisherFreeObject(hPub);

         sipXresult = SIPX_RESULT_SUCCESS;
      }
   }
   else
   {
      // Could not find the publish data for the given handle
      sipXresult = SIPX_RESULT_INVALID_ARGS;
   }

   return sipXresult;
}


SIPXTAPI_API SIPX_RESULT sipxPublisherCreate(const SIPX_INST hInst,
                                             SIPX_PUB* phPub,
                                             const char* szResourceId,
                                             const char* szEventType,
                                             const char* szContentType,
                                             const char* pContent,
                                             const int nContentLength)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxPublisherCreate");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCreatePublisher hInst=%p phPub=%d szResourceId=\"%s\" szEventType=\"%s\" szContentType=\"%s\" pContent=\"%s\" nContentLength=%d",
      hInst,
      *phPub,
      szResourceId ? szResourceId : "<null>",
      szEventType ? szEventType : "<null>",
      szContentType ? szContentType : "<null>",
      pContent ? pContent : "<null>",
      nContentLength);

   SIPX_RESULT sipXresult = SIPX_RESULT_FAILURE;
   // Get the instance data
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   // Verify that no content has been previously published for this
   // resourceID and eventType
   HttpBody* oldContentPtr = NULL;
   UtlBoolean isDefaultContent;
   SipPublishContentMgr* publishMgr = NULL;

   if(szEventType && *szEventType)
   {
      if(pInst->pSubscribeServer->isEventTypeEnabled(szEventType))
      {
         publishMgr = pInst->pSubscribeServer->getPublishMgr(szEventType);

         if(publishMgr)
         {
            publishMgr->getContent(szResourceId, 
               szEventType, 
               szEventType, 
               szContentType,
               oldContentPtr, 
               isDefaultContent);
         }

         // Default content is ok, ignore it
         if(isDefaultContent && oldContentPtr)
         {
            delete oldContentPtr;
            oldContentPtr = NULL;
         }
      }
   }
   else
   {
      sipXresult = SIPX_RESULT_INVALID_ARGS;
      OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
         "sipxCreatePublisher: argument szEventType is NULL");
   }

   if(oldContentPtr)
   {
      OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
         "sipxCreatePublisher: content already exists for resourceId: %s and eventType: %s",
         szResourceId ? szResourceId : "<null>",
         szEventType ? szEventType : "<null>");

      sipXresult = SIPX_RESULT_INVALID_ARGS;
      delete oldContentPtr;
      oldContentPtr = NULL;
   }
   else if(szEventType && *szEventType && szResourceId &&
          (nContentLength > 0 || nContentLength == -1))
   {
      // No prior content published
      // Create a publish structure for the SIPX_PUB handle
      SIPX_PUBLISH_DATA* pData = new SIPX_PUBLISH_DATA();
      pData->mutex.acquire();
      // Register the publisher handle
      UtlBoolean res = gPubHandleMap.allocHandle(*phPub, pData);

      if (res)
      {
         pData->pInst = pInst;
         pData->resourceId = szResourceId;
         pData->eventType = szEventType;

         // Create a new HttpBody to publish for the resourceId and eventType
         HttpBody* content = new HttpBody(pContent, nContentLength, szContentType);

         // No publisher for this event type
         if(publishMgr == NULL)
         {
            // Enable the event type for the SUBSCRIBE Server
            pInst->pSubscribeServer->enableEventType(pData->eventType);
            publishMgr = pInst->pSubscribeServer->getPublishMgr(pData->eventType);
         }

         // Publish the content
         publishMgr->publish(pData->resourceId,
            pData->eventType,
            pData->eventType,
            1, // one content type for event
            &content);

         pData->mutex.release();
         sipXresult = SIPX_RESULT_SUCCESS;
      } 
      else
      {
         // handle allocation failure
         OsSysLog::add(FAC_SIPXTAPI, PRI_ERR, 
            "allocHandle failed to allocate a handle");
         delete pData;
      }
   }

   return sipXresult;
}


SIPXTAPI_API SIPX_RESULT sipxPublisherUpdate(const SIPX_PUB hPub,
                                             const char* szContentType,
                                             const char* pContent,
                                             const int nContentLength)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxPublisherUpdate");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxUpdatePublisher hPub=%d szContentType=\"%s\" pContent=\"%s\" nContentLength=%d",
      hPub,
      szContentType ? szContentType : "<null>",
      pContent ? pContent : "<null>",
      nContentLength);

   SIPX_RESULT sipXresult = SIPX_RESULT_FAILURE;

   if(szContentType && *szContentType &&
      (nContentLength > 0 || nContentLength == -1) && 
      pContent && *pContent)
   {
      SIPX_PUBLISH_DATA* pData = (SIPX_PUBLISH_DATA*) sipxPublishLookup(hPub, SIPX_LOCK_WRITE, stackLogger);

      if (pData)
      {
         SipSubscribeServer* pSubscribeServer = pData->pInst->pSubscribeServer;
         UtlString resourceId(pData->resourceId);
         UtlString eventType(pData->eventType);

         sipxPublishReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);

         if (pSubscribeServer)
         {
            SipPublishContentMgr* publishMgr = pSubscribeServer->getPublishMgr(eventType);

            if(publishMgr)
            {
               HttpBody* newContent = new HttpBody(pContent, nContentLength, szContentType);

               // Publish the state change
               publishMgr->publish(resourceId,
                  eventType,
                  eventType,
                  1, // one content type for event
                  &newContent);

               sipXresult = SIPX_RESULT_SUCCESS;
            }
            else
            {
               OsSysLog::add(FAC_SIPXTAPI, PRI_ERR,
                  "sipxUpdatePublisher: no publisher for event type: %s",
                  eventType.data());

               sipXresult = SIPX_RESULT_FAILURE;
            }
         }         
      }      
   }
   else
   {
      // content size < 0 || content is null
      sipXresult = SIPX_RESULT_INVALID_ARGS;
   }

   return sipXresult;
}
