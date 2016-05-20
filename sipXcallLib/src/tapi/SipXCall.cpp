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

#include <utl/UtlVoidPtr.h>
#include <utl/UtlHashMapIterator.h>
#include <net/SipUserAgent.h>
#include <net/SipLine.h>
#include "tapi/SipXHandleMap.h"
#include "tapi/SipXCall.h"
#include "tapi/SipXLine.h"
#include "tapi/SecurityHelper.h"
#include "tapi/SipXCallEventListener.h"
#include "cp/XCpCallManager.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
SipXHandleMap gCallHandleMap(1, SIPX_CALL_NULL);  /**< Global Map of call handles */

// GLOBAL FUNCTIONS

/* ============================ FUNCTIONS ================================= */


UtlBoolean sipxCallSetState(SIPX_CALL hCall,
                            SIPX_CALLSTATE_EVENT event,
                            SIPX_CALLSTATE_CAUSE cause)
{
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallSetState");
   SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, logItem);

   if (pData)
   {
      pData->m_callState = event;
      pData->m_callStateCause = cause;

      if (event == CALLSTATE_CONNECTED)
      {
         // we also have focus
         pData->m_bInFocus = TRUE;
      }
      else if (event == CALLSTATE_BRIDGED)
      {
         // we don't have focus
         pData->m_bInFocus = FALSE;
      }

      sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, logItem);
      return TRUE;
   }

   return FALSE;
}

UtlBoolean sipxCallSetAbstractCallId(SIPX_CALL hCall, const UtlString& sAbstractCallId)
{
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallSetAbstractCallId");
   SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, logItem);

   if (pData)
   {
      pData->m_abstractCallId = sAbstractCallId;
      sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, logItem);
      return TRUE;
   }

   return FALSE;
}

SIPX_CALL sipxCallLookupHandleBySessionCallId( const UtlString& sessionCallID, SIPX_INST pInst )
{
   SIPX_CALL hCall = 0;
   SIPX_CALL_DATA* pData = NULL;
   OsStatus status;

   gCallHandleMap.lock();
   // control iterator scope
   {
      UtlHashMapIterator iter(gCallHandleMap);

      UtlInt* pIndex = NULL;
      UtlVoidPtr* pObj = NULL;

      while ((pIndex = dynamic_cast<UtlInt*>(iter())))       
      {
         pObj = dynamic_cast<UtlVoidPtr*>(gCallHandleMap.findValue(pIndex));

         assert(pObj); // if it's NULL, then it's a bug
         if (pObj)
         {
            pData = (SIPX_CALL_DATA*)pObj->getValue();
            assert(pData);

            if (pData)
            {
               status = pData->m_mutex.acquire();
               assert(status == OS_SUCCESS);

               if (pData->m_sipDialog.getCallId().compareTo(sessionCallID) == 0 && 
                   pData->m_pInst == pInst)
               {
                  hCall = pIndex->getValue();
                  status = pData->m_mutex.release();
                  assert(status == OS_SUCCESS);
                  break;
               }

               status = pData->m_mutex.release();
               assert(status == OS_SUCCESS);
            }            
         }
      }
   }
   gCallHandleMap.unlock();

   return hCall;
}


SIPX_CALL sipxCallLookupHandleByCallId( const UtlString& callID, SIPX_INST pInst )
{
   SIPX_CALL hCall = SIPX_CALL_NULL;
   SIPX_CALL_DATA* pData = NULL;
   OsStatus status;

   gCallHandleMap.lock();
   // control iterator scope
   {
      UtlHashMapIterator iter(gCallHandleMap);

      UtlInt* pIndex = NULL;
      UtlVoidPtr* pObj = NULL;

      while ((pIndex = dynamic_cast<UtlInt*>(iter())))       
      {
         pObj = dynamic_cast<UtlVoidPtr*>(gCallHandleMap.findValue(pIndex));

         assert(pObj); // if it's NULL, then it's a bug
         if (pObj)
         {
            pData = (SIPX_CALL_DATA*)pObj->getValue();
            assert(pData);

            if (pData)
            {
               status = pData->m_mutex.acquire();
               assert(status == OS_SUCCESS);

               if (pData->m_abstractCallId.compareTo(callID) == 0 && 
                  pData->m_pInst == pInst)
               {
                  hCall = pIndex->getValue();
                  status = pData->m_mutex.release();
                  assert(status == OS_SUCCESS);
                  break;
               }

               status = pData->m_mutex.release();
               assert(status == OS_SUCCESS);
            }            
         }
      }
   }
   gCallHandleMap.unlock();

   return hCall;
}

/**
* Frees call object.
*/

void sipxCallObjectFree(const SIPX_CALL hCall, const OsStackTraceLogger& oneBackInStack)
{
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallObjectFree", oneBackInStack);

   gCallHandleMap.lock();
   SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, logItem);

   if (pData)
   {
      const void* pRC = gCallHandleMap.removeHandle(hCall); 
      gCallHandleMap.unlock(); // we can release lock now
      assert(pRC); // if NULL, then something is bad :(

      pData->m_pInst->decrementCallCount();
      destroyCallData(pData);
   }
   else
   {
      gCallHandleMap.unlock(); // we can release lock now
   }
}

/**
* Finds call by call handle and acquires lock on it.
* OsMutex is used internally instead of OsRWMutex to allow recursive locking.
*/

SIPX_CALL_DATA* sipxCallLookup(const SIPX_CALL hCall,
                               SIPX_LOCK_TYPE type,
                               const OsStackTraceLogger& oneBackInStack)
{
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallLookup", oneBackInStack);
   SIPX_CALL_DATA* pRC = NULL;
   OsStatus status = OS_FAILED;

   gCallHandleMap.lock();
   pRC = (SIPX_CALL_DATA*)gCallHandleMap.findHandle(hCall);

   if (pRC)
   {
      if (validCallData(pRC))
      {
         switch (type)
         {
         case SIPX_LOCK_READ:
            status = pRC->m_mutex.acquire();
            assert(status == OS_SUCCESS);
            break;
         case SIPX_LOCK_WRITE:
            status = pRC->m_mutex.acquire();
            assert(status == OS_SUCCESS);
            break;
         default:
            break;
         }
      }
      else // call was found but call data is not completely valid
      {
         // fire assert, something is not right, fix it
         assert(false); // this only works in debug mode
         pRC = NULL; // we only get here in release mode
      }
   }
   gCallHandleMap.unlock();

   return pRC;
}

/**
* Checks whether call data is valid.
*/

UtlBoolean validCallData(SIPX_CALL_DATA* pData)
{
   return (pData &&
      !pData->m_abstractCallId.isNull() && 
      pData->m_pInst &&
      pData->m_pInst->pCallManager && 
      pData->m_pInst->pRefreshManager &&
      pData->m_pInst->pLineManager);
}

/**
* Releases call lock.
*/

void sipxCallReleaseLock(SIPX_CALL_DATA* pData,
                         SIPX_LOCK_TYPE type,
                         const OsStackTraceLogger& oneBackInStack)
{
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallReleaseLock", oneBackInStack);
   OsStatus status;

   if (type != SIPX_LOCK_NONE)
   {
      if (validCallData(pData))
      {
         switch (type)
         {
         case SIPX_LOCK_READ:
            status = pData->m_mutex.release();
            assert(status == OS_SUCCESS);
            break;
         case SIPX_LOCK_WRITE:
            status = pData->m_mutex.release();
            assert(status == OS_SUCCESS);
            break;
         default:
            break;
         }
      }
      else
      {
         // something is bad if call data is not valid, fix bug
         assert(false);
      }
   }   
}

void destroyCallData(SIPX_CALL_DATA* pData)
{
   if (pData)
   {
      // no need to release mutex, nobody should be waiting on it or its a bug
      delete pData;
   }
}


UtlBoolean sipxCallGetState(SIPX_CALL hCall, 
                            SIPX_CALLSTATE_EVENT& lastEvent,
                            SIPX_CALLSTATE_CAUSE& lastCause) 
{
   UtlBoolean bSuccess = FALSE;
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetState");

   SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ, logItem);
   if (pData)
   {
      lastEvent = pData->m_callState;
      lastCause = pData->m_callStateCause;
      bSuccess = TRUE;
      sipxCallReleaseLock(pData, SIPX_LOCK_READ, logItem);
   }

   return bSuccess;
}


UtlBoolean sipxCallGetCommonData(SIPX_CALL hCall,
                                 SIPX_INSTANCE_DATA** pInst,
                                 UtlString* pCallId,
                                 UtlString* pSessionCallId,
                                 UtlString* pRemoteField,
                                 UtlString* pLocalField,
                                 UtlString* pGhostCallId, 
                                 UtlString* pRemoteContactAddress) 
{
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetCommonData");

   UtlBoolean bSuccess = FALSE;
   SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ, logItem);

   if (pData)
   {
      if (pInst)
      {
         *pInst = pData->m_pInst;
      }

      if (pCallId)
      {
          *pCallId = pData->m_abstractCallId;
      }

      if (pSessionCallId)
      {
         *pSessionCallId = pData->m_sipDialog.getCallId();
      }

      if (pRemoteField)
      {
         pData->m_sipDialog.getRemoteField(*pRemoteField);
      }

      if (pLocalField)
      {
         pData->m_sipDialog.getLocalField(*pLocalField);
      }

      if (pRemoteContactAddress)
      {
         pData->m_sipDialog.getRemoteContact(*pRemoteContactAddress);
      }
      bSuccess = TRUE;

      sipxCallReleaseLock(pData, SIPX_LOCK_READ, logItem);
   }

   return bSuccess;
}


// returns ids of XCpCall and XCpConference
SIPXTAPI_API SIPX_RESULT sipxGetAllAbstractCallIds(SIPX_INST hInst, UtlSList& idList)
{
   SIPX_RESULT rc = SIPX_RESULT_FAILURE;

   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);
   if (pInst)
   {
      OsStatus status = pInst->pCallManager->getAbstractCallIds(idList);
      if (status == OS_SUCCESS)
      {
         rc = SIPX_RESULT_SUCCESS;
      }
   }
   else
   {
      rc = SIPX_RESULT_INVALID_ARGS;
   }

   return rc;
}


// Destroy all calls, no need to send simulated DESTROYED events, as the way
// we shutdown OsServerTasks is going to change (optionally they will wait until all messages
// are processed)
void sipxCallDestroyAll(const SIPX_INST hInst) 
{
   SIPX_CALL hCall = SIPX_CALL_NULL;        
   UtlSList sessionCallIdList;
   UtlString* pSessionCallId;

   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);
   if (pInst)
   {
      sipxGetAllAbstractCallIds(hInst, sessionCallIdList);

      // iterate over all session call ids
      UtlSListIterator itor(sessionCallIdList);
      while(pSessionCallId = dynamic_cast<UtlString*>(itor()))
      {
         hCall = sipxCallLookupHandleBySessionCallId(*pSessionCallId, pInst);

         if (hCall)
         {
            sipxCallDestroy(&hCall);
         }
      }

      sessionCallIdList.destroyAll();
   }
}


SIPX_CONF sipxCallGetConf(SIPX_CALL hCall) 
{
   SIPX_CONF hConf = SIPX_CONF_NULL;
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetConf");

   SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ, logItem);
   if (pData)
   {
      hConf = pData->m_hConf;
      sipxCallReleaseLock(pData, SIPX_LOCK_READ, logItem);
   }

   return hConf;
}

void sipxCallSetConf(SIPX_CALL hCall, SIPX_CONF hConf)
{
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetConf");

   SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, logItem);
   if (pData)
   {
      pData->m_hConf = hConf;
      sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, logItem);
   }
}

SIPX_CONTACT_TYPE sipxCallGetLineContactType(SIPX_CALL hCall) 
{
   SIPX_CONTACT_TYPE contactType = CONTACT_AUTO;
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetLineContactType");

   SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ, logItem);
   if (pData)
   {
      SIPX_LINE hLine = pData->m_hLine;
      sipxCallReleaseLock(pData, SIPX_LOCK_READ, logItem);

      SIPX_LINE_DATA* pLineData = sipxLineLookup(hLine, SIPX_LOCK_READ, logItem);

      if (pLineData)
      {
         contactType = pLineData->m_contactType;
         sipxLineReleaseLock(pLineData, SIPX_LOCK_READ, logItem);
      }
   }

   return contactType;
}

// this function assumes call is not in conference
SIPX_RESULT sipxCallDrop(SIPX_CALL& hCall)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallDrop");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallDrop hCall=%d",
      hCall);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);

   if (pData)
   {
      assert(pData->m_hConf == SIPX_CONF_NULL);
      if (pData->m_callState != CALLSTATE_DISCONNECTED &&
         pData->m_callState != CALLSTATE_DESTROYED)
      {
         // if call is not disconnected or destroyed, allow drop. Otherwise call will destroy itself, therefore
         // do not allow explicit drop.
         pData->m_pInst->pCallManager->dropAbstractCallConnection(pData->m_abstractCallId, pData->m_sipDialog);

         hCall = SIPX_CALL_NULL;
         sr = SIPX_RESULT_SUCCESS;
      }

      sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
   }

   return sr;
}


// either hLine or szLine must be available
SIPX_RESULT sipxCallCreateHelper(const SIPX_INST hInst,
                                 const SIPX_LINE hLine,
                                 const char* szLine,
                                 const SIPX_CONF hConf,
                                 SIPX_CALL* phCall,
                                 const UtlString& sAbstractCallId,
                                 const UtlString& sSipCallId,
                                 bool bIsConferenceCall)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallCreateHelper");
   OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG, 
      "sipxCallCreateHelper, hInst=%i, hLine=%i, hConf=%i",
      hInst, hLine, hConf);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst)
   {
      if (phCall)
      {
         // set line URI either by handle, or string
         UtlString sLineUri;
         UtlString sFullLineUrl;
         if (hLine != SIPX_LINE_NULL)
         {
            SIPX_LINE_DATA* pLine = sipxLineLookup(hLine, SIPX_LOCK_READ, stackLogger);
            if (pLine)
            {
               sLineUri = pLine->m_lineUri.toString();
               sFullLineUrl = pLine->m_fullLineUrl.toString();
               sipxLineReleaseLock(pLine, SIPX_LOCK_READ, stackLogger);
            }
         }
         else
         {
            sLineUri = SipLine::getLineUri(UtlString(szLine)).toString();// unique line id, sip uri
            sFullLineUrl = SipLine::buildFullLineUrl(UtlString(szLine)).toString();// used in from field, sip url
         }
         
         // we now must have a valid line URI - should be not null
         if (!sLineUri.isNull())
         {
            Url lineUri(sLineUri); // reconstruct lineURI Url object from string
            SIPX_CALL_DATA* pData = new SIPX_CALL_DATA();
            // Set line URI in call
            pData->m_fullLineUrl = sFullLineUrl;
            pData->m_lineUri = lineUri; // keep also original lineURI, which never gets a tag
            pData->m_mutex.acquire();
            UtlBoolean res = gCallHandleMap.allocHandle(*phCall, pData);

            if (res)
            {
               // Increment call count
               pInst->incrementCallCount();

               if (sAbstractCallId.isNull())
               {
                  if (!bIsConferenceCall)
                  {
                     // we don't want to select callId, generate one
                     pData->m_abstractCallId = pInst->pCallManager->getNewCallId();
                  }
                  else
                  {
                     pData->m_abstractCallId = pInst->pCallManager->getNewConferenceId();
                  }
               }
               else
               {
                  // we want custom callId, use it
                  pData->m_abstractCallId = sAbstractCallId;
               }
               
               // Set Conference handle
               pData->m_hConf = hConf;

               // Set Line handle
               pData->m_hLine = hLine;

               // Store Instance
               pData->m_pInst = pInst;

               // m_sipDialog will be set when call is connected, but for conference
               // we set it here, so that event system can find this call for dialtone event
               pData->m_sipDialog = SipDialog(sSipCallId); // tags are unknown

               if (!bIsConferenceCall)
               {
                  // only create real call if it is requested
                  // for conference add, we don't create it here
                  pInst->pCallManager->createCall(pData->m_abstractCallId);
               }

               pData->m_mutex.release();

               sr = SIPX_RESULT_SUCCESS;
            }
            else
            {
               // handle allocation failure
               OsSysLog::add(FAC_SIPXTAPI, PRI_ERR, 
                  "allocHandle failed to allocate a handle");
               // don't release mutex, no need for it
               delete pData;
            }
         }
      }
      else
      {
         sr = SIPX_RESULT_INVALID_ARGS;
      }
   }

   return sr;
}


/*********************************************************************/
/*       Public call handling functions                              */
/*********************************************************************/


SIPXTAPI_API SIPX_RESULT sipxCallCreate(const SIPX_INST hInst,
                                        const SIPX_LINE hLine,
                                        SIPX_CALL* phCall)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallCreate");

   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallCreate hInst=%p hLine=%d phCall=%p",
      hInst, hLine, phCall);
   SIPX_RESULT rc = SIPX_RESULT_FAILURE;

   if (phCall)
   {
      rc = sipxCallCreateHelper(hInst, hLine, NULL, SIPX_CONF_NULL, phCall);
   }
   
   return rc;
}


SIPXTAPI_API SIPX_RESULT sipxCallCreateOnVirtualLine(const SIPX_INST hInst,
                                                     const char* szLine,
                                                     SIPX_CALL* phCall)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallCreateOnVirtualLine");

   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallCreateOnVirtualLine hInst=%p szLine=%s phCall=%p",
      hInst, szLine, phCall);

   SIPX_RESULT rc = SIPX_RESULT_FAILURE;

   if (phCall && szLine)
   {
      rc = sipxCallCreateHelper(hInst, SIPX_LINE_NULL, szLine, SIPX_CONF_NULL, phCall);
   }

   return rc;
}


SIPXTAPI_API SIPX_RESULT sipxCallReject(const SIPX_CALL hCall)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallReject");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallReject hCall=%d",
      hCall);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   SIPX_CALL_DATA *pCallData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);
   if (pCallData)
   {   
      if (!pCallData->m_sipDialog.getRemoteField().isNull() && !pCallData->m_sipDialog.getCallId().isNull())
      {
         // call has been connected
         pCallData->m_pInst->pCallManager->rejectAbstractCallConnection(pCallData->m_abstractCallId,
            pCallData->m_sipDialog);
         sr = SIPX_RESULT_SUCCESS;
      }

      sipxCallReleaseLock(pCallData, SIPX_LOCK_READ, stackLogger);
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxCallRedirect(const SIPX_CALL hCall, const char* szForwardURL)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallRedirect");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallRedirect hCall=%d forwardURL=%s",
      hCall, szForwardURL);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   SIPX_CALL_DATA *pCallData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);
   if (pCallData)
   {
      if (!pCallData->m_sipDialog.getRemoteField().isNull() && !pCallData->m_sipDialog.getCallId().isNull() && szForwardURL)
      {
         pCallData->m_pInst->pCallManager->redirectAbstractCallConnection(pCallData->m_abstractCallId,
            pCallData->m_sipDialog,
            szForwardURL);
         sr = SIPX_RESULT_SUCCESS;
      }

      sipxCallReleaseLock(pCallData, SIPX_LOCK_READ, stackLogger);
   }
   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxCallAccept(const SIPX_CALL hCall,
                                        SIPX_CALL_OPTIONS* options,
                                        int bSendSdp)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallAccept");
   UtlBoolean bEnableLocationHeader = FALSE;
   SIPX_CONTACT_ID contactId = SIPX_AUTOMATIC_CONTACT_ID;
   SIPX_TRANSPORT_TYPE transport = TRANSPORT_AUTO;
   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   char* pLocationHeader = NULL;

   if (options)
   {
      if (options->cbSize == sizeof(SIPX_CALL_OPTIONS))
      {
         bEnableLocationHeader = options->sendLocation;
         contactId = options->contactId;
         transport = options->transportType;
      }
      else
      {
         // size mismatch
         assert(false);
         return SIPX_RESULT_INVALID_ARGS;
      }
   }

   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallAccept hCall=%d bEnableLocationHeader=%d contactId=%d bSendEarlyMedia=%s",
      hCall, bEnableLocationHeader, contactId, 
      bSendSdp ? "true" : "false");

   if (contactId < SIPX_AUTOMATIC_CONTACT_ID || contactId == 0)
   {
      // wrong contactID
      return SIPX_RESULT_INVALID_ARGS;
   }

   SIPX_CALL_DATA *pCallData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);
   if (pCallData)
   {
      SIPX_INSTANCE_DATA* pInst = pCallData->m_pInst;
      assert(pInst);

      if (contactId > 0)
      {
         // verify contactId
         SipContact* pContact = pInst->pSipUserAgent->getContactDb().find(contactId);
         if (!pContact)
         {
            // contactId is invalid
            sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE, stackLogger);
            return SIPX_RESULT_INVALID_ARGS;
         }
         // contactId is valid, verify if contact transport matches specified transport
         if (transport != TRANSPORT_AUTO &&
            transport != (SIPX_TRANSPORT_TYPE)pContact->getTransportType())
         {
            delete pContact;
            pContact = NULL;
            sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE, stackLogger);
            return SIPX_RESULT_INVALID_ARGS;
         }
         else
         {
            transport = (SIPX_TRANSPORT_TYPE)pContact->getTransportType();
            delete pContact;
            pContact = NULL;
         }
      }

      // check whether the call has a sessionId - then it's real
      if (!pCallData->m_sipDialog.getRemoteField().isNull() && !pCallData->m_sipDialog.getCallId().isNull())
      {
         // setup location header
         pLocationHeader = (bEnableLocationHeader) ? pCallData->m_pInst->szLocationHeader : NULL;

         // just posts message
         pCallData->m_pInst->pCallManager->acceptAbstractCallConnection(pCallData->m_abstractCallId,
            pCallData->m_sipDialog,
            bSendSdp,
            pLocationHeader,
            contactId,
            (SIP_TRANSPORT_TYPE)transport);

         sr = SIPX_RESULT_SUCCESS;
      }

      sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE, stackLogger);
   }// else call not found

   return sr;
}

// for conference call bStopRemoteAudio is ignored as only remote audio is stopped
// that is to prevent focus loss from the conference.
SIPXTAPI_API SIPX_RESULT sipxCallHold(const SIPX_CALL hCall,
                                      int bStopRemoteAudio)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallHold");
   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallHold hCall=%d bStopRemoteAudio=%d",
      hCall, bStopRemoteAudio);

   SIPX_CALL_DATA* pCallData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);

   if (pCallData)
   {
      if (pCallData->m_callState == CALLSTATE_CONNECTED ||
         pCallData->m_callState == CALLSTATE_BRIDGED ||
         pCallData->m_callState == CALLSTATE_REMOTE_HELD)
      {
         if (bStopRemoteAudio)
         {
            pCallData->m_pInst->pCallManager->holdAbstractCallConnection(pCallData->m_abstractCallId,
                                                                         pCallData->m_sipDialog);
         }

         pCallData->m_pInst->pCallManager->holdLocalAbstractCallConnection(pCallData->m_abstractCallId);
         sr = SIPX_RESULT_SUCCESS;
      }
      else
      {
         sr = SIPX_RESULT_INVALID_STATE;
      }
      
      sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE, stackLogger);
   }// no pCallData = call not found

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxCallUnhold(const SIPX_CALL hCall, int bTakeFocus)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallUnhold");
   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallUnhold hCall=%d, bTakeFocus=%d",
      hCall, bTakeFocus);


   SIPX_CALL_DATA* pCallData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);
   if (pCallData)
   {
      if (pCallData->m_callState != CALLSTATE_HELD &&
          pCallData->m_callState != CALLSTATE_REMOTE_HELD &&
          pCallData->m_callState != CALLSTATE_BRIDGED)
      {
         sr = SIPX_RESULT_INVALID_STATE;
      }
      else
      {
         // just posts message. Sends re-INVITE or UPDATE
         pCallData->m_pInst->pCallManager->unholdAbstractCallConnection(pCallData->m_abstractCallId,
                                                                        pCallData->m_sipDialog);
         if (bTakeFocus)
         {
            // just posts message. Takes audio resources for this call.
            pCallData->m_pInst->pCallManager->unholdLocalAbstractCallConnection(pCallData->m_abstractCallId);
         }
         sr = SIPX_RESULT_SUCCESS;
      }

      sipxCallReleaseLock(pCallData, SIPX_LOCK_READ, stackLogger);
   }// no pCallData = call not found

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxCallAnswer(const SIPX_CALL hCall, int bTakeFocus)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallAnswer");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallAnswer hCall=%d",
      hCall);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   SIPX_CALL_DATA *pCallData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);

   if (pCallData)
   {
      if(!pCallData->m_sipDialog.getRemoteField().isNull() && !pCallData->m_sipDialog.getCallId().isNull())
      {
         // this call has a SipConnection, go on
         if (bTakeFocus)
         {
            pCallData->m_pInst->pCallManager->unholdLocalAbstractCallConnection(pCallData->m_abstractCallId);
         }
         
         pCallData->m_pInst->pCallManager->answerAbstractCallConnection(pCallData->m_abstractCallId,
            pCallData->m_sipDialog);

         sr = SIPX_RESULT_SUCCESS;
      }// call found but no SipConnection, probably a call we created but didn't connect

      sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE, stackLogger);
   }// no pCallData = call not found

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxCallAcceptTransfer(const SIPX_CALL hCall)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallAcceptTransfer");
   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO, "sipxCallAcceptTransfer hCall=%d", hCall);
   SIPX_INSTANCE_DATA* pInst = NULL;
   SipDialog sipDialog;
   UtlString abstractCallId;

   SIPX_CALL_DATA* pCallData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);
   if (pCallData)
   {
      pInst = pCallData->m_pInst;
      sipDialog = pCallData->m_sipDialog;
      abstractCallId = pCallData->m_abstractCallId;
      sipxCallReleaseLock(pCallData, SIPX_LOCK_READ, stackLogger);

      if (pInst)
      {
         pInst->pCallManager->acceptAbstractCallTransfer(abstractCallId, sipDialog);
         sr = SIPX_RESULT_SUCCESS;
      }
   }// no pCallData = call not found

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxCallRejectTransfer(const SIPX_CALL hCall)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallRejectTransfer");
   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO, "sipxCallRejectTransfer hCall=%d", hCall);
   SIPX_INSTANCE_DATA* pInst = NULL;
   SipDialog sipDialog;
   UtlString abstractCallId;

   SIPX_CALL_DATA* pCallData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);
   if (pCallData)
   {
      pInst = pCallData->m_pInst;
      sipDialog = pCallData->m_sipDialog;
      abstractCallId = pCallData->m_abstractCallId;
      sipxCallReleaseLock(pCallData, SIPX_LOCK_READ, stackLogger);

      if (pInst)
      {
         pInst->pCallManager->rejectAbstractCallTransfer(abstractCallId, sipDialog);
         sr = SIPX_RESULT_SUCCESS;
      }
   }// no pCallData = call not found

   return sr;
}

// returns Sip CallID
SIPXTAPI_API SIPX_RESULT sipxCallGetSipCallId(const SIPX_CALL hCall,
                                              char* szSipCallId,
                                              const size_t iMaxLength)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetSipCallId");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallGetSipCallId hCall=%d",
      hCall);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   if (szSipCallId)
   {
      UtlString sipCallId;

      if (sipxCallGetCommonData(hCall, NULL, NULL, &sipCallId, NULL, NULL))
      {
         if (iMaxLength > 0)
         {
            SAFE_STRNCPY(szSipCallId, sipCallId, iMaxLength);
            sr = SIPX_RESULT_SUCCESS;
         }
      }// no pCallData = call not found
   }
      
   return sr;
}

// returns Line Uri of the call
SIPXTAPI_API SIPX_RESULT sipxCallGetLocalField(const SIPX_CALL hCall,
                                               char* szLocalField,
                                               const size_t iMaxLength)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetLocalField");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallGetLocalField hCall=%d",
      hCall);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   if (szLocalField)
   {
      UtlString localField;

      if (sipxCallGetCommonData(hCall, NULL, NULL, NULL, NULL, &localField))
      {
         if (iMaxLength > 0)
         {
            SAFE_STRNCPY(szLocalField, localField, iMaxLength);
            sr = SIPX_RESULT_SUCCESS;
         }
      }
   }
   
   return sr;
}

// returns remote address
SIPXTAPI_API SIPX_RESULT sipxCallGetRemoteField(const SIPX_CALL hCall,
                                                char* szRemoteField,
                                                const size_t iMaxLength)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetRemoteField");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallGetRemoteField hCall=%d",
      hCall);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   if (szRemoteField)
   {
      UtlString remoteField;

      if (sipxCallGetCommonData(hCall, NULL, NULL, NULL, &remoteField, NULL))
      {
         if (iMaxLength > 0)
         {
            SAFE_STRNCPY(szRemoteField, remoteField, iMaxLength);
            sr = SIPX_RESULT_SUCCESS;
         }
      }
   }
   
   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxCallGetLocalContact(const SIPX_CALL hCall,
                                                 char* szContactAddress,
                                                 const size_t iMaxLength)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetLocalContact");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallGetLocalContact hCall=%d",
      hCall);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);

   if (pData)
   {
      if (pData->m_pInst &&
         !pData->m_abstractCallId.isNull() && 
         !pData->m_sipDialog.getRemoteField().isNull())
      {
         SipDialog sipDialog(pData->m_sipDialog);
         sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger);

         Url contact;
         sipDialog.getLocalContact(contact);

         if (iMaxLength > 0)
         {
            SAFE_STRNCPY(szContactAddress, contact.toString().data(), iMaxLength);
            sr = SIPX_RESULT_SUCCESS;
         }
      }
      else
      {
         sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
      }
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxCallGetConference(const SIPX_CALL hCall,
                                               SIPX_CONF* hConf) 
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetConference");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallGetConference hCall=%d", hCall);

   SIPX_RESULT sr = SIPX_RESULT_INVALID_ARGS;

   if (hConf)
   {
      if (hCall != SIPX_CALL_NULL)
      {
         *hConf = sipxCallGetConf(hCall);

         if (*hConf != SIPX_CONF_NULL)
         {
            sr = SIPX_RESULT_SUCCESS;
         }
         else
         {
            sr = SIPX_RESULT_FAILURE;
         }
      }
   }
   
   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxCallConnect(SIPX_CALL hCall,
                                         const char* szAddress,
                                         SIPX_FOCUS_CONFIG takeFocus,
                                         SIPX_CALL_OPTIONS* options,
                                         const char* szSessionCallId)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallConnect");

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   char* pLocationHeader = NULL; // passed to CallManager

   // default values if options are not passed
   UtlBoolean bEnableLocationHeader = FALSE;
   SIPX_CONTACT_ID contactId = SIPX_AUTOMATIC_CONTACT_ID; // passed to CallManager
   SIPX_TRANSPORT_TYPE transport = TRANSPORT_AUTO;

   if (options)
   {
      if (options->cbSize == sizeof(SIPX_CALL_OPTIONS))
      {
         bEnableLocationHeader = options->sendLocation;
         contactId = options->contactId;
         transport = options->transportType;
      }
      else
      {
         // size mismatch
         assert(false);
         return SIPX_RESULT_INVALID_ARGS;
      }
   }

   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallConnect hCall=%d szAddress=%s contactId=%d bEnableLocationHeader=%d",
      hCall, szAddress, contactId, bEnableLocationHeader);

   assert(szAddress);
   if (!szAddress)
   {
      return SIPX_RESULT_INVALID_ARGS;
   }

   if (contactId < SIPX_AUTOMATIC_CONTACT_ID || contactId == 0)
   {
      return SIPX_RESULT_INVALID_ARGS;
   }

   SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);
   if (pData)
   {
      SIPX_INSTANCE_DATA* pInst = pData->m_pInst;
      assert(pInst);
      assert(pData->m_sipDialog.getRemoteField().isNull());    // should be null

      if (contactId > 0)
      {
         // verify contactId
         SipContact* pContact = pInst->pSipUserAgent->getContactDb().find(contactId);
         if (!pContact)
         {
            // contactId is invalid
            sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
            return SIPX_RESULT_INVALID_ARGS;
         }
         // contactId is valid, verify if contact transport matches specified transport
         if (transport != TRANSPORT_AUTO &&
            transport != (SIPX_TRANSPORT_TYPE)pContact->getTransportType())
         {
            delete pContact;
            pContact = NULL;
            sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
            return SIPX_RESULT_INVALID_ARGS;
         }
         else
         {
            transport = (SIPX_TRANSPORT_TYPE)pContact->getTransportType();
            delete pContact;
            pContact = NULL;
         }
      }
      
      if (pInst && pData->m_sipDialog.getRemoteField().isNull())
      {
         // create sipCallId
         pData->m_sipDialog.setCallId(szSessionCallId);
         if (pData->m_sipDialog.getCallId().isNull())
         {
            pData->m_sipDialog.setCallId(pInst->pCallManager->getNewSipCallId());
         }

         // set location header
         pInst->lock.acquire();
         pLocationHeader = (bEnableLocationHeader) ? pInst->szLocationHeader : NULL;
         pInst->lock.release();

         OsStatus status = OS_FAILED;

         // connect just checks address and posts a message
         if (pData->m_hConf == SIPX_CONF_NULL)
         {
            // call is not part of conference
            status = pInst->pCallManager->connectCall(pData->m_abstractCallId, pData->m_sipDialog,
               szAddress, pData->m_fullLineUrl.toString(), pData->m_sipDialog.getCallId(),
               pLocationHeader, contactId, (SIP_TRANSPORT_TYPE)transport, (CP_FOCUS_CONFIG)takeFocus);
         }
         else
         {
            // call is part of conference
            status = pInst->pCallManager->connectConferenceCall(pData->m_abstractCallId, pData->m_sipDialog,
               szAddress, pData->m_fullLineUrl.toString(), pData->m_sipDialog.getCallId(),
               pLocationHeader, contactId, (SIP_TRANSPORT_TYPE)transport, (CP_FOCUS_CONFIG)takeFocus);
         }
         sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);

         if (status != OS_SUCCESS)
         {
            sr = SIPX_RESULT_BAD_ADDRESS;
         }
         else
         {
            sr = SIPX_RESULT_SUCCESS;
         }
      }
      else
      {
         sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
         sr = SIPX_RESULT_FAILURE;
      }
   }// else call was not found

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxCallDestroy(SIPX_CALL* hCall)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallDestroy");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallDestroy hCall=%d",
      hCall);

   SIPX_CONF hConf = sipxCallGetConf(*hCall);
   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   // if call is part of conference, remove it
   if (hConf != SIPX_CONF_NULL)
   {
      sr = sipxConferenceRemove(hConf, hCall);
   }
   else
   {
      // not part of conference, just drop it
      sr = sipxCallDrop(*hCall);
   }

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxCallStartRtpRedirect(const SIPX_CALL hSrcCall, const SIPX_CALL hDstCall)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallStartRtpRedirect");
   OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallStartRtpRedirect hSrcCall=%d, hDstCall=%d", hSrcCall, hDstCall);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   UtlString sAbstractCallId1;
   UtlString sAbstractCallId2;
   SipDialog sipDialog1;
   SipDialog sipDialog2;
   SIPX_INSTANCE_DATA* pInst1 = NULL;
   SIPX_INSTANCE_DATA* pInst2 = NULL;
   SIPX_CONF callConf1;
   SIPX_CONF callConf2;
   RTP_REDIRECT_STATE rtpRedirectState1;
   RTP_REDIRECT_STATE rtpRedirectState2;
   // lookup 1st call
   SIPX_CALL_DATA* pSrcData = sipxCallLookup(hSrcCall, SIPX_LOCK_READ, stackLogger);
   if (pSrcData)
   {
      sipDialog1 = pSrcData->m_sipDialog;
      pInst1 = pSrcData->m_pInst;
      sAbstractCallId1 = pSrcData->m_abstractCallId;
      callConf1 = pSrcData->m_hConf;
      rtpRedirectState1 = pSrcData->m_rtpRedirectState;
      sipxCallReleaseLock(pSrcData, SIPX_LOCK_READ, stackLogger);
   }
   else
   {
      return SIPX_RESULT_INVALID_ARGS;
   }
   // lookup 2nd call
   SIPX_CALL_DATA* pDstData = sipxCallLookup(hDstCall, SIPX_LOCK_READ, stackLogger);
   if (pDstData)
   {
      sipDialog2 = pDstData->m_sipDialog;
      pInst2 = pDstData->m_pInst;
      sAbstractCallId2 = pDstData->m_abstractCallId;
      callConf2 = pDstData->m_hConf;
      rtpRedirectState2 = pDstData->m_rtpRedirectState;
      sipxCallReleaseLock(pDstData, SIPX_LOCK_READ, stackLogger);
   }
   else
   {
      return SIPX_RESULT_INVALID_ARGS;
   }
   // check if both calls are ok
   if (pInst1 == pInst2 &&
      pInst1->pCallManager && pInst2->pCallManager &&
      !sAbstractCallId1.isNull() && !sAbstractCallId2.isNull() &&
      !sipDialog1.getRemoteField().isNull() && !sipDialog2.getRemoteField().isNull() &&
      callConf1 == SIPX_CONF_NULL && callConf2 == SIPX_CONF_NULL &&
      rtpRedirectState1 == RTP_REDIRECT_STATE_INACTIVE && rtpRedirectState2 == RTP_REDIRECT_STATE_INACTIVE)
   {
      if(pInst1->pCallManager->startCallRedirectRtp(sAbstractCallId1, sAbstractCallId2, sipDialog2) == OS_SUCCESS)
      {
         sr = SIPX_RESULT_SUCCESS;
      }
   }

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxCallStopRtpRedirect(const SIPX_CALL hCall)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallStopRtpRedirect");
   OsSysLog::add(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallStopRtpRedirect hCall=%d", hCall);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   // lookup the call
   SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);
   if (pData)
   {
      SipDialog sipDialog = pData->m_sipDialog;
      SIPX_INSTANCE_DATA* pInst = pData->m_pInst;
      UtlString sAbstractCallId = pData->m_abstractCallId;
      RTP_REDIRECT_STATE rtpRedirectState = pData->m_rtpRedirectState;
      sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger);

      // proceed with stopping RTP redirect
      if (pInst->pCallManager &&
         !sAbstractCallId.isNull() &&
         !sipDialog.getRemoteField().isNull() &&
         rtpRedirectState == RTP_REDIRECT_STATE_ACTIVE)
      {
         if(pInst->pCallManager->stopCallRedirectRtp(sAbstractCallId) == OS_SUCCESS)
         {
            sr = SIPX_RESULT_SUCCESS;
         }
      }
   }
   else
   {
      return SIPX_RESULT_INVALID_ARGS;
   }

   return sr;
}

// internal function used for testing only
SIPXTAPI_API SIPX_RESULT sipxCallGetMediaConnectionId(const SIPX_CALL hCall,
                                                      int* connectionId)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetMediaConnectionId");
   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   *connectionId = -1;

   SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);

   if (pData)        
   {
      // find media connectionId
      if (pData->m_pInst &&
          pData->m_pInst->pCallManager &&
          !pData->m_abstractCallId.isNull() && 
          !pData->m_sipDialog.getRemoteField().isNull())
      {
         XCpCallManager* pCallManager = pData->m_pInst->pCallManager;
         UtlString callId(pData->m_abstractCallId);
         SipDialog sipDialog(pData->m_sipDialog);

         sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger);

         pCallManager->getMediaConnectionId(callId, sipDialog, *connectionId);
        
         if (*connectionId != -1)
         {
            sr = SIPX_RESULT_SUCCESS;
         }
      }
      else
      {
         sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
      }
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxCallGetRequestURI(const SIPX_CALL hCall,
                                               char* szRequestUri,
                                               const size_t iMaxLength)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetRequestURI");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallGetRequestURI hCall=%d",
      hCall);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE ;
   SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);

   if (pData)
   {
      if (pData->m_pInst &&
          pData->m_pInst->pCallManager &&
          !pData->m_abstractCallId.isNull() &&
          !pData->m_sipDialog.getRemoteField().isNull()) 
      {
         SipDialog sipDialog(pData->m_sipDialog);
         sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger);

         Url requestUri;
         UtlString sUri;
         if (sipDialog.isLocalInitiatedDialog())
         {
            // outbound call
            sipDialog.getRemoteRequestUri(requestUri);
         }
         else
         {
            // inbound call
            sipDialog.getLocalRequestUri(requestUri);
         }
         requestUri.toString(sUri);

         if (iMaxLength)
         {
            SAFE_STRNCPY(szRequestUri, sUri.data(), iMaxLength);
            sr = SIPX_RESULT_SUCCESS;
         }
      }
      else
      {
         sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
      }
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxCallGetRemoteContact(const SIPX_CALL hCall,
                                                  char* szContactAddress,
                                                  const size_t iMaxLength)
{
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallGetRemoteContact hCall=%d",
      hCall);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   if (szContactAddress)
   {
      UtlString contactAddress;

      if (sipxCallGetCommonData(hCall, NULL, NULL, NULL, NULL, NULL, NULL, &contactAddress))
      {
         if (iMaxLength > 0)
         {
            SAFE_STRNCPY(szContactAddress, contactAddress, iMaxLength);
            sr = SIPX_RESULT_SUCCESS;
         }
      }
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxCallGetRemoteUserAgent(const SIPX_CALL hCall,
                                                    char* szAgent,
                                                    const size_t iMaxLength)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallGetRemoteUserAgent");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallGetRemoteUserAgent hCall=%d",
      hCall);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);

   if (pData)
   {
      if (pData->m_pInst &&
          pData->m_pInst->pCallManager &&
          pData->m_sipDialog.isEstablishedDialog())
      {
         XCpCallManager* pCallManager = pData->m_pInst->pCallManager;
         UtlString callId(pData->m_abstractCallId);
         SipDialog sipDialog(pData->m_sipDialog);
         UtlString userAgent;

         sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger);

         pCallManager->getRemoteUserAgent(callId, sipDialog, userAgent);

         if (iMaxLength)
         {
            SAFE_STRNCPY(szAgent, userAgent.data(), iMaxLength);
            sr = SIPX_RESULT_SUCCESS;
         }
      }
      else
      {
         sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
      }
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxCallStartTone(const SIPX_CALL hCall,
                                           const SIPX_TONE_ID toneId,
                                           const int bLocal,
                                           const int bRemote,
                                           const int duration)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallStartTone");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallStartTone hCall=%d ToneId=%d bLocal=%d bRemote=%d",
      hCall, toneId, bLocal, bRemote);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);

   if (pData)
   {
      int dtmfDuration = duration > 60 ? duration : -1;
      // posts a message
      pData->m_pInst->pCallManager->audioToneStart(pData->m_abstractCallId, toneId, bLocal, bRemote, dtmfDuration);

      sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
      sr = SIPX_RESULT_SUCCESS;
   }

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxCallStopTone(const SIPX_CALL hCall)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallStopTone");

   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallStopTone hCall=%d",
      hCall);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_CALL_DATA* pData = NULL;

   pData =  sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);

   if (pData)
   {
      // posts a message
      pData->m_pInst->pCallManager->audioToneStop(pData->m_abstractCallId);
      sr = SIPX_RESULT_SUCCESS;

      sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
   }

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxCallAudioPlayFileStart(const SIPX_CALL hCall,
                                                    const char* szFile,
                                                    const int bRepeat,
                                                    const int bLocal,
                                                    const int bRemote,
                                                    const int bMixWithMicrophone,
                                                    const float fVolumeScaling,
                                                    void* pCookie) 
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallAudioPlayFileStart");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallAudioPlayFileStart hCall=%d File=%s bLocal=%d bRemote=%d bRepeat=%d bMixWithMic=%d, volScale=%f",
      hCall, szFile, bLocal, bRemote, bRepeat, bMixWithMicrophone, fVolumeScaling);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   // Massage volume scaling into range
   float fDownScaling = fVolumeScaling;
   if (fDownScaling > 1.0)
   {
      fDownScaling = 1.0;
   }
   else if (fDownScaling < 0.0)
   {
      fDownScaling = 0.0;
   }

   if (szFile)
   {
      SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);

      if (pData)
      {
         // posts a message
         pData->m_pInst->pCallManager->audioFilePlay(pData->m_abstractCallId, szFile, bRepeat, bLocal, bRemote,
            bMixWithMicrophone, (int)(fDownScaling * 100.0), pCookie);

         sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);

         sr = SIPX_RESULT_SUCCESS;
      }
   }
   else
   {
      sr = SIPX_RESULT_INVALID_ARGS;
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxCallAudioPlayFileStop(const SIPX_CALL hCall)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallAudioPlayFileStop");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallAudioPlayFileStop hCall=%d", hCall);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);

   if (pData)
   {
      // posts a message
      pData->m_pInst->pCallManager->audioStopPlayback(pData->m_abstractCallId);
      sr = SIPX_RESULT_SUCCESS;

      sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
   }

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxCallAudioPlaybackPause(const SIPX_CALL hCall)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallAudioPlaybackPause");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallAudioPlaybackPause hCall=%d", hCall);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);

   if (pData)
   {
      // posts a message
      pData->m_pInst->pCallManager->pauseAudioPlayback(pData->m_abstractCallId);
      sr = SIPX_RESULT_SUCCESS;

      sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
   }

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxCallAudioPlaybackResume(const SIPX_CALL hCall)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallAudioPlaybackResume");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallAudioPlaybackResume hCall=%d", hCall);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);

   if (pData)
   {
      // posts a message
      pData->m_pInst->pCallManager->resumeAudioPlayback(pData->m_abstractCallId);
      sr = SIPX_RESULT_SUCCESS;

      sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
   }

   return sr;
}



SIPXTAPI_API SIPX_RESULT sipxCallAudioRecordFileStart(const SIPX_CALL hCall,
                                                      const char* szFile) 
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallAudioRecordFileStart");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallAudioRecordFileStart hCall=%d szFile=%s", hCall, szFile);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA *pInst;
   UtlString callId;
   UtlString remoteAddress;

   if (szFile && sipxCallGetCommonData(hCall, &pInst, &callId, NULL, &remoteAddress, NULL))
   {
      if (pInst->pCallManager->audioRecordStart(callId, szFile))
      {
         sr = SIPX_RESULT_SUCCESS;
      }
   }
   else
   {
      sr = SIPX_RESULT_INVALID_ARGS;
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxCallAudioRecordFileStop(const SIPX_CALL hCall) 
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallAudioRecordFileStop");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallAudioRecordFileStop hCall=%d", hCall);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA *pInst;
   UtlString callId;
   UtlString remoteAddress;

   if (sipxCallGetCommonData(hCall, &pInst, &callId, NULL, &remoteAddress, NULL))
   {
      if (pInst->pCallManager->audioRecordStop(callId))
      {
         sr = SIPX_RESULT_SUCCESS;
      }
   }
   else
   {
      sr = SIPX_RESULT_INVALID_ARGS;
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxCallPlayBufferStart(const SIPX_CALL hCall,
                                                 const void* pBuffer,
                                                 const int bufSize,
                                                 const int bufType,
                                                 const int bRepeat,
                                                 const int bLocal,
                                                 const int bRemote,
                                                 void* pCookie)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallPlayBufferStart");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallPlayBufferStart hCall=%d Buffer=%p Size=%d Type=%d bLocal=%d bRemote=%d bRepeat=%d",
      hCall, pBuffer, bufSize, bufType, bLocal, bRemote, bRepeat);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);

   if (pData)
   {
      if (pBuffer)
      {
         SIPX_INSTANCE_DATA* pInst = pData->m_pInst;
         UtlString callId(pData->m_abstractCallId);
         sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);

         if (pInst)
         {
            pInst->pCallManager->audioBufferPlay(callId, pBuffer, bufSize, bufType, bRepeat,
               bLocal, bRemote, FALSE, 100, pCookie);
            sr = SIPX_RESULT_SUCCESS;
         }
      }
      else
      {
         sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
         sr = SIPX_RESULT_INVALID_ARGS;
      }
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxCallPlayBufferStop(const SIPX_CALL hCall)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallPlayBufferStop");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallPlayBufferStop hCall=%d", hCall);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);

   if (pData)
   {
      // just posts a message
      pData->m_pInst->pCallManager->audioStopPlayback(pData->m_abstractCallId);
      sr = SIPX_RESULT_SUCCESS;

      sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxCallBlindTransfer(const SIPX_CALL hCall,
                                               const char* pszAddress)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallBlindTransfer");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallBlindTransfer hCall=%d Address=%s",
      hCall, pszAddress);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   if (pszAddress)
   {
      // first, get rid of any existing ghost connection
      SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);

      if (pData)
      {
         SIPX_INSTANCE_DATA* pInst = pData->m_pInst;
         UtlString callId(pData->m_abstractCallId);
         SipDialog sipDialog(pData->m_sipDialog);
         sipxCallReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);

         // call the transfer
         pInst->pCallManager->transferBlindAbstractCall(callId, sipDialog, pszAddress);
       
         sr = SIPX_RESULT_SUCCESS;
      }
      else
      {
         sr = SIPX_RESULT_INVALID_ARGS;
      }
   }
   else
   {
      sr = SIPX_RESULT_INVALID_ARGS;
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxCallTransfer(const SIPX_CALL hSourceCall,
                                          const SIPX_CALL hTargetCall)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallTransfer");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallTransfer hSourceCall=%d hTargetCall=%d\n",
      hSourceCall, hTargetCall);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA *pInst1;
   SIPX_INSTANCE_DATA *pInst2;
   UtlString sourceCallId;
   SipDialog sourceSipDialog;
   UtlString targetCallId;
   SipDialog targetSipDialog;

   // get info about source call
   SIPX_CALL_DATA* pData = sipxCallLookup(hSourceCall, SIPX_LOCK_READ, stackLogger);
   if (pData)
   {
      pInst1 = pData->m_pInst;
      sourceCallId = pData->m_abstractCallId;
      sourceSipDialog = pData->m_sipDialog;
      sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
   }
   else
   {
      return SIPX_RESULT_INVALID_ARGS;
   }

   // get info about target call
   pData = sipxCallLookup(hTargetCall, SIPX_LOCK_READ, stackLogger);
   if (pData)
   {
      pInst2 = pData->m_pInst;
      targetCallId = pData->m_abstractCallId;
      targetSipDialog = pData->m_sipDialog;
      sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
   }
   else
   {
      return SIPX_RESULT_INVALID_ARGS;
   }

   // call transfer works only within the same instance
   if (pInst1 == pInst2)
   {
      // call the transfer            
      if (pInst1->pCallManager->transferConsultativeAbstractCall(sourceCallId, sourceSipDialog, targetCallId, targetSipDialog) == OS_SUCCESS)
      {
         sr = SIPX_RESULT_SUCCESS;
      }
   }
   else
   {
      sr = SIPX_RESULT_INVALID_ARGS;
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxCallGetAudioRtpSourceIds(const SIPX_CALL hCall,
                                                      unsigned int* iSendSSRC,
                                                      unsigned int* iReceiveSSRC) 
{
   SIPX_RESULT rc = SIPX_RESULT_NOT_IMPLEMENTED;

   return rc;
}


SIPXTAPI_API SIPX_RESULT sipxCallGetAudioRtcpStats(const SIPX_CALL hCall,
                                                   SIPX_RTCP_STATS* pStats)
{
   SIPX_RESULT rc = SIPX_RESULT_NOT_IMPLEMENTED;

   return rc;
}

SIPXTAPI_API SIPX_RESULT sipxCallLimitCodecPreferences(const SIPX_CALL hCall,
                                                       const char* szAudioCodecs,
                                                       const char* szVideoCodecs)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallLimitCodecPreferences");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallLimitCodecPreferences hCall=%d szAudioCodecs=\"%s\"",
      hCall,
      (szAudioCodecs != NULL) ? szAudioCodecs : "");

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   // get info about source call
   SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);
   if (pData)
   {
      SIPX_INSTANCE_DATA *pInst = pData->m_pInst;
      UtlString sCallId = pData->m_abstractCallId;
      SipDialog sipDialog = pData->m_sipDialog;
      sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger);

      if (pInst->pCallManager->limitAbstractCallCodecPreferences(sCallId,
         szAudioCodecs,
         szVideoCodecs) == OS_SUCCESS)
      {
         sr = SIPX_RESULT_SUCCESS;
      }
   }
   else
   {
      return SIPX_RESULT_INVALID_ARGS;
   }

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxCallRenegotiateCodecPreferences(const SIPX_CALL hCall,
                                                             const char* szAudioCodecs,
                                                             const char* szVideoCodecs)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallRenegotiateCodecPreferences");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallRenegotiateCodecPreferences hCall=%d szAudioCodecs=\"%s\"",
      hCall,
      (szAudioCodecs != NULL) ? szAudioCodecs : "");

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   // get info about source call
   SIPX_CALL_DATA* pData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);
   if (pData)
   {
      SIPX_INSTANCE_DATA *pInst = pData->m_pInst;
      UtlString sCallId = pData->m_abstractCallId;
      SipDialog sipDialog = pData->m_sipDialog;
      sipxCallReleaseLock(pData, SIPX_LOCK_READ, stackLogger);

      if (pInst->pCallManager->renegotiateCodecsAbstractCallConnection(sCallId,
         sipDialog,
         szAudioCodecs,
         szVideoCodecs) == OS_SUCCESS)
      {
         sr = SIPX_RESULT_SUCCESS;
      }
   }
   else
   {
      return SIPX_RESULT_INVALID_ARGS;
   }

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxCallMuteInput(const SIPX_CALL hCall, const int bMute)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallMuteInput");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxCallMuteInput hCall=%d bMute=%d", hCall, bMute);

   // Find Call
   SIPX_CALL_DATA* pCallData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, stackLogger);
   if (pCallData)
   {
      SIPX_INSTANCE_DATA *pInst = pCallData->m_pInst;
      SipDialog sipDialog = pCallData->m_sipDialog;
      UtlString callId = pCallData->m_abstractCallId;
      sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE, stackLogger);

      if (sipDialog.isEstablishedDialog())
      {
         if (bMute)
         {
            // local hold
            if (pInst->pCallManager->muteInputAbstractCallConnection(callId, sipDialog) == OS_SUCCESS)
            {
               return SIPX_RESULT_SUCCESS;
            }
         }
         else
         {
            // local unhold
            if (pInst->pCallManager->unmuteInputAbstractCallConnection(callId, sipDialog) == OS_SUCCESS)
            {
               return SIPX_RESULT_SUCCESS;
            }
         }
      }
   }

   return SIPX_RESULT_FAILURE;
}
