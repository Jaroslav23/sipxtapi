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
#include <utl/UtlInt.h>
#include <utl/UtlVoidPtr.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/UtlSListIterator.h>
#include "tapi/SipXConference.h"
#include "tapi/SipXHandleMap.h"
#include "tapi/SipXCall.h"
#include "tapi/SipXCallEventListener.h"
#include "cp/XCpCallManager.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
SipXHandleMap gConfHandleMap(1, SIPX_CONF_NULL);  /**< Global Map of conf handles */
extern SipXHandleMap gCallHandleMap;

// GLOBAL FUNCTIONS

/* ============================ FUNCTIONS ================================= */


UtlBoolean sipxRemoveCallHandleFromConf(const SIPX_CONF hConf, 
                                        const SIPX_CALL hCall) 
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxRemoveCallHandleFromConf");

   UtlBoolean bFound = FALSE;
   SIPX_CONF_DATA* pConfData = sipxConfLookup(hConf, SIPX_LOCK_WRITE, stackLogger);

   if (pConfData)
   {
      UtlInt hCallItem(hCall);
      bFound = pConfData->m_hCalls.destroy(&hCallItem);
      sipxConfReleaseLock(pConfData, SIPX_LOCK_WRITE, stackLogger);

      if (bFound)
      {
         // also clear reference from call to conference
         sipxCallSetConf(hCall, SIPX_CONF_NULL);
      }
   }

   return bFound;
}

SIPX_CONF sipxConfLookupHandleByConfId(const UtlString& confID, SIPX_INST pInst)
{
   SIPX_CONF hConf = SIPX_CONF_NULL;
   SIPX_CONF_DATA* pData = NULL;
   OsStatus status;

   gConfHandleMap.lock();
   // control iterator scope
   {
      UtlHashMapIterator iter(gConfHandleMap);

      UtlInt* pIndex = NULL;
      UtlVoidPtr* pObj = NULL;

      while ((pIndex = dynamic_cast<UtlInt*>(iter())))       
      {
         pObj = dynamic_cast<UtlVoidPtr*>(gConfHandleMap.findValue(pIndex));

         assert(pObj); // if it's NULL, then it's a bug
         if (pObj)
         {
            pData = (SIPX_CONF_DATA*)pObj->getValue();
            assert(pData);

            if (pData)
            {
               status = pData->m_mutex.acquire();
               assert(status == OS_SUCCESS);

               if (pData->m_sConferenceId.compareTo(confID) == 0 && 
                  pData->m_pInst == pInst)
               {
                  hConf = pIndex->getValue();
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
   gConfHandleMap.unlock();

   return hConf;
}

SIPX_CONF_DATA* sipxConfLookup(const SIPX_CONF hConf,
                               SIPX_LOCK_TYPE type,
                               const OsStackTraceLogger& oneBackInStack) 
{
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfLookup", oneBackInStack);
   OsStatus status = OS_FAILED;
   SIPX_CONF_DATA* pRC = NULL;
   
   gConfHandleMap.lock();
   pRC = (SIPX_CONF_DATA*)gConfHandleMap.findHandle(hConf);

   if (validConfData(pRC))
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
   else
   {
      pRC = NULL;
   }

   gConfHandleMap.unlock();

   return pRC;
}


void sipxConfReleaseLock(SIPX_CONF_DATA* pData,
                         SIPX_LOCK_TYPE type,
                         const OsStackTraceLogger& oneBackInStack) 
{
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfReleaseLock", oneBackInStack);
   OsStatus status;

   if (type != SIPX_LOCK_NONE)
   {
      if (validConfData(pData))
      {
         switch (type)
         {
         case SIPX_LOCK_READ:
            status = pData->m_mutex.release();
            assert(status == OS_SUCCESS);
            break ;
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


void sipxConfFree(const SIPX_CONF hConf) 
{
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxConfFree");
   SIPX_CONF_DATA* pData = NULL;
   
   gConfHandleMap.lock();
   pData = sipxConfLookup(hConf, SIPX_LOCK_WRITE, logItem);
   assert(pData);

   if (pData)
   {
      assert(pData->m_hCalls.entries() == 0);
      const void* pRC = gConfHandleMap.removeHandle(hConf);
      assert(pRC);
      gConfHandleMap.unlock();

      // decrease conference counter
      pData->m_pInst->decrementConferenceCount();
      delete pData;
   }
   else
   {
      // pData is NULL
      gConfHandleMap.unlock();
   }
}


void sipxConferenceDestroyAll(const SIPX_INST hInst) 
{
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceDestroyAll");
   gConfHandleMap.lock();

   UtlHashMapIterator pubIter(gConfHandleMap);
   UtlInt* pKey;
   UtlVoidPtr* pValue;
   SIPX_CONF hConf;

   while ((pKey = dynamic_cast<UtlInt*>(pubIter())))
   {
      pValue = dynamic_cast<UtlVoidPtr*>(gConfHandleMap.findValue(pKey));
      assert(pValue);

      hConf = (SIPX_CONF)pValue->getValue();

      if (hConf)
      {
         bool bRemove = false;

         SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_READ, logItem);
         if (pData)
         {
            if (pData->m_pInst == hInst)
            {
               bRemove = true;
            }
            sipxConfReleaseLock(pData, SIPX_LOCK_READ, logItem);
         }

         if (bRemove)
         {
            sipxConferenceDestroy(hConf);
         }
      }
   }

   gConfHandleMap.unlock();
}


UtlBoolean validConfData(const SIPX_CONF_DATA* pData) 
{
   UtlBoolean bValid = FALSE;

   if (pData)
   {
      bValid = TRUE;
   }

   return bValid;
}


UtlBoolean sipxAddCallHandleToConf(const SIPX_CALL hCall,
                                   const SIPX_CONF hConf)
{
   UtlBoolean res = FALSE;    
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxAddCallHandleToConf");

   SIPX_CONF_DATA* pConfData = sipxConfLookup(hConf, SIPX_LOCK_WRITE, logItem);
   if (pConfData)
   {
      pConfData->m_hCalls.append(new UtlInt(hCall));
      sipxConfReleaseLock(pConfData, SIPX_LOCK_WRITE, logItem);

      SIPX_CALL_DATA* pCallData = sipxCallLookup(hCall, SIPX_LOCK_WRITE, logItem);

      if (pCallData)
      {
         pCallData->m_hConf = hConf;
         res = TRUE;

         sipxCallReleaseLock(pCallData, SIPX_LOCK_WRITE, logItem);
      }
      else
      {
         // restore conference nCalls if pCallData not found
         pConfData = sipxConfLookup(hConf, SIPX_LOCK_WRITE, logItem);

         if (pConfData)
         {
            pConfData->m_hCalls.destroy(&UtlInt(hCall));
            sipxConfReleaseLock(pConfData, SIPX_LOCK_WRITE, logItem);
         }
      }
   }

   return res;
}

/***************************************************************************
* Public Conference Related Functions
***************************************************************************/


SIPXTAPI_API SIPX_RESULT sipxConferenceGetCalls(const SIPX_CONF hConf,
                                                SIPX_CALL hCalls[],
                                                const size_t iMax,
                                                size_t* nActual)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceGetCalls");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxConferenceGetCalls hConf=%d",
      hConf);

   SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;
   *nActual = 0;

   if (hConf != SIPX_CONF_NULL && iMax > 0)
   {
      SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_READ, stackLogger);
      if (pData)
      {
         UtlSListIterator itor(pData->m_hCalls);
         size_t idx = 0;
         while (itor() && idx < iMax)
         {
            UtlInt *phCall = dynamic_cast<UtlInt*>(itor.item());
            if (phCall)
            {
               hCalls[idx++] = (unsigned int)phCall->getValue();
            }
         }
         *nActual = idx;
         sipxConfReleaseLock(pData, SIPX_LOCK_READ, stackLogger);

         rc = SIPX_RESULT_SUCCESS;
      }
      else
      {
         rc = SIPX_RESULT_FAILURE;
      }
   }

   return rc;
}


SIPXTAPI_API SIPX_RESULT sipxConferenceCreate(const SIPX_INST hInst,
                                              SIPX_CONF *phConference,
                                              const char* szConferenceUri)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceCreate");
   SIPX_RESULT rc = SIPX_RESULT_FAILURE;

   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxConferenceCreate hInst=%p phConference=%p szConferenceUri=%s",
      hInst, phConference, szConferenceUri);

   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   if (pInst)
   {
      if (phConference)
      {
         *phConference = SIPX_CONF_NULL;

         SIPX_CONF_DATA* pData = new SIPX_CONF_DATA();
         pData->m_mutex.acquire();
         UtlBoolean res = gConfHandleMap.allocHandle(*phConference, pData);

         if (res)
         {
            // Increment conference counter
            pInst->incrementConferenceCount();

            // Init conference data
            pData->m_pInst = pInst;

            // create shell call for conference
            pData->m_pInst->pCallManager->createConference(pData->m_sConferenceId, szConferenceUri);

            pData->m_mutex.release();
            rc = SIPX_RESULT_SUCCESS;
         }
         else
         {
            // handle allocation failure
            OsSysLog::add(FAC_SIPXTAPI, PRI_ERR, 
               "allocHandle failed to allocate a handle");
            delete pData;
         }
      }
      else
      {
         rc = SIPX_RESULT_INVALID_ARGS;
      }
   }
   else
   {
      rc = SIPX_RESULT_INVALID_ARGS;
   }
   
   return rc;
}

SIPXTAPI_API SIPX_RESULT sipxConferenceJoin(const SIPX_CONF hConf,
                                            const SIPX_CALL hCall)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceJoin");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxConferenceJoin hConf=%d hCall=%d",
      hConf, hCall);

   SIPX_RESULT rc = SIPX_RESULT_FAILURE;

   if (hConf != SIPX_CONF_NULL && hCall != SIPX_CALL_NULL)
   {
      SIPX_CONF_DATA* pConfData = sipxConfLookup(hConf, SIPX_LOCK_READ, stackLogger);

      if (pConfData)
      {
         UtlString conferenceId(pConfData->m_sConferenceId);
         size_t nCalls = pConfData->m_hCalls.entries();
         SIPX_INSTANCE_DATA* pInst1 = pConfData->m_pInst;
         sipxConfReleaseLock(pConfData, SIPX_LOCK_READ, stackLogger);

         if (pInst1 && nCalls < CONF_MAX_CONNECTIONS)
         {
            SIPX_CALL_DATA* pCallData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);
            if (pCallData && !pCallData->m_sipDialog.getRemoteField().isNull() && pCallData->m_hConf == SIPX_CONF_NULL)
            {
               SipDialog sipDialog(pCallData->m_sipDialog);
               SIPX_INSTANCE_DATA* pInst2 = pConfData->m_pInst;
               sipxCallReleaseLock(pCallData, SIPX_LOCK_READ, stackLogger);

               if (pInst1 == pInst2)
               {
                  // call and conference must exist in the same call manager
                  if (pInst1->pCallManager->conferenceJoin(conferenceId, sipDialog) == OS_SUCCESS)
                  {
                     // m_abstractCallId of call will be updated if asynchronous operation succeeds
                     rc = SIPX_RESULT_SUCCESS;
                  }
               }
            }
         }
      }
   }

   return rc;
}

SIPXTAPI_API SIPX_RESULT sipxConferenceSplit(const SIPX_CONF hConf,
                                             const SIPX_CALL hCall)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceSplit");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxConferenceSplit hConf=%d hCall=%d",
      hConf, hCall);

   SIPX_RESULT rc = SIPX_RESULT_FAILURE;

   if (hConf != SIPX_CONF_NULL && hCall != SIPX_CALL_NULL)
   {
      SIPX_CONF_DATA* pConfData = sipxConfLookup(hConf, SIPX_LOCK_READ, stackLogger);

      if (pConfData)
      {
         UtlString conferenceId(pConfData->m_sConferenceId);
         SIPX_INSTANCE_DATA* pInst1 = pConfData->m_pInst;
         sipxConfReleaseLock(pConfData, SIPX_LOCK_READ, stackLogger);

         if (pInst1)
         {
            SIPX_CALL_DATA* pCallData = sipxCallLookup(hCall, SIPX_LOCK_READ, stackLogger);
            if (pCallData)
            {
               if (!pCallData->m_sipDialog.getRemoteField().isNull() && pCallData->m_hConf == hConf &&
                  pInst1 == pCallData->m_pInst)
               {
                  UtlString newCallId;
                  if (pInst1->pCallManager->conferenceSplit(conferenceId, pCallData->m_sipDialog, newCallId) == OS_SUCCESS)
                  {
                     pCallData->m_splitCallId = newCallId;
                     rc = SIPX_RESULT_SUCCESS;
                  }
               }
               sipxCallReleaseLock(pCallData, SIPX_LOCK_READ, stackLogger);
            }
         }
      }
   }

   return rc;
}


SIPXTAPI_API SIPX_RESULT sipxConferenceAdd(const SIPX_CONF hConf,
                                           const SIPX_LINE hLine,
                                           const char* szAddress,
                                           SIPX_CALL* phNewCall,
                                           SIPX_FOCUS_CONFIG takeFocus,
                                           SIPX_CALL_OPTIONS* options)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceAdd");

   SIPX_RESULT rc = SIPX_RESULT_FAILURE;
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxConferenceAdd hConf=%d hLine=%d szAddress=%s ", hConf, hLine, szAddress);

   SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_WRITE, stackLogger);
   if (pData)
   {
      SIPX_INSTANCE_DATA* pInst = pData->m_pInst;
      size_t nCalls = pData->m_hCalls.entries();
      UtlString conferenceId(pData->m_sConferenceId);
      sipxConfReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);

      if (pInst && nCalls < CONF_MAX_CONNECTIONS)
      {
         // conference was found
         // create sip call-id for conference call
         UtlString sipCallId = pInst->pCallManager->getNewSipCallId();

         // create SIPX_CALL_DATA structure with conferenceId
         SIPX_RESULT res = sipxCallCreateHelper(pInst,
            hLine,
            NULL,
            hConf,
            phNewCall,
            conferenceId,// becomes abstractCallId
            sipCallId,
            true);// bIsConferenceCall, will not create it in call manager

         if (res == SIPX_RESULT_SUCCESS)
         {
            // connect call
            rc = sipxCallConnect(*phNewCall, szAddress, takeFocus, options, sipCallId);
            if (rc != SIPX_RESULT_SUCCESS)
            {
               // destroy the call
               sipxCallObjectFree(*phNewCall, stackLogger);
            }
         }
         else
         {
            rc = SIPX_RESULT_OUT_OF_RESOURCES;
         }
      }      
   }

   return rc;
}


SIPXTAPI_API SIPX_RESULT sipxConferenceRemove(const SIPX_CONF hConf,
                                              SIPX_CALL* hCall)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceRemove");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxConferenceRemove hConf=%d hCall=%d",
      hConf, hCall);

   SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;

   if (hConf != SIPX_CONF_NULL && hCall && *hCall != SIPX_CALL_NULL)
   {
      SIPX_CONF_DATA* pConfData = sipxConfLookup(hConf, SIPX_LOCK_READ, stackLogger);
      if (pConfData)
      {
         SIPX_INSTANCE_DATA* pInst = pConfData->m_pInst;
         UtlString conferenceId(pConfData->m_sConferenceId);

         SIPX_CALL_DATA* pCallData = sipxCallLookup(*hCall, SIPX_LOCK_READ, stackLogger);
         if (pCallData)
         {            
            rc = SIPX_RESULT_FAILURE;

            if (pInst->pCallManager->dropConferenceConnection(conferenceId, pCallData->m_sipDialog) == OS_SUCCESS)
            {
               // conference call list is updated in reaction to CONFERENCE_CALL_REMOVED
               *hCall = SIPX_CALL_NULL;
               rc = SIPX_RESULT_SUCCESS;
            }
            sipxCallReleaseLock(pCallData, SIPX_LOCK_READ, stackLogger);
         }
         sipxConfReleaseLock(pConfData, SIPX_LOCK_READ, stackLogger);
      }
   }

   return rc;
}

SIPXTAPI_API SIPX_RESULT sipxConferenceHold(const SIPX_CONF hConf,
                                            int bBridging)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceHold");

   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxConferenceHold hConf=%d bBridging=%d",
      hConf,
      bBridging);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   if (hConf != SIPX_CONF_NULL)
   {
      SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_WRITE, stackLogger);

      if (pData)
      {
         sr = SIPX_RESULT_INVALID_STATE;

         if (pData->m_hCalls.entries() > 0)
         {
            if (bBridging)
            {
               pData->m_pInst->pCallManager->holdLocalAbstractCallConnection(pData->m_sConferenceId);
               pData->m_confHoldState = CONF_STATE_BRIDGING_HOLD;
               sr = SIPX_RESULT_SUCCESS;
            }
            else
            {
               pData->m_pInst->pCallManager->holdAllConferenceConnections(pData->m_sConferenceId);
               pData->m_confHoldState = CONF_STATE_NON_BRIDGING_HOLD;
               sr = SIPX_RESULT_SUCCESS;
            }
         }

         sipxConfReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
      }
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxConferenceUnhold(const SIPX_CONF hConf)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceUnhold");

   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxConferenceUnHold hConf=%d",
      hConf);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   if (hConf != SIPX_CONF_NULL)
   {
      SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_READ, stackLogger);

      if (pData)
      {
         sr = SIPX_RESULT_INVALID_STATE;

         if (pData->m_hCalls.entries() > 0)
         {
            if (pData->m_confHoldState == CONF_STATE_BRIDGING_HOLD)
            {
               // just posts a message
               pData->m_pInst->pCallManager->unholdLocalAbstractCallConnection(pData->m_sConferenceId);
               pData->m_confHoldState = CONF_STATE_UNHELD;
               sr = SIPX_RESULT_SUCCESS;
            }
            else if (pData->m_confHoldState == CONF_STATE_NON_BRIDGING_HOLD)
            {
               // just posts a message
               pData->m_pInst->pCallManager->unholdAllConferenceConnections(pData->m_sConferenceId);
               pData->m_confHoldState = CONF_STATE_UNHELD;
               sr = SIPX_RESULT_SUCCESS;
            }
         }   

         sipxConfReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
      }
   }

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxConferencePlayAudioFileStart(const SIPX_CONF hConf,
                                                          const char* szFile,
                                                          const int bRepeat,
                                                          const int bLocal,
                                                          const int bRemote,
                                                          const int bMixWithMic,
                                                          const float fVolumeScaling)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferencePlayAudioFileStart");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxConferencePlayAudioFileStart hConf=%d File=%s bLocal=%d bRemote=%d bRepeat=%d",
      hConf, szFile, bLocal, bRemote, bRepeat);

   SIPX_RESULT sr = SIPX_RESULT_INVALID_ARGS;

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

   if (hConf != SIPX_CONF_NULL && szFile && (bLocal || bRemote))
   {
      SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_READ, stackLogger);

      if (pData)
      {
         if (pData->m_hCalls.entries() > 0)
         {
            // conference was found, just repost message
            pData->m_pInst->pCallManager->audioFilePlay(pData->m_sConferenceId, szFile, bRepeat,
               bLocal, bRemote, bMixWithMic, (int) (fDownScaling * 100.0));

            sr = SIPX_RESULT_SUCCESS;
         }         
         sipxConfReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
      }
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxConferencePlayAudioFileStop(const SIPX_CONF hConf)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferencePlayAudioFileStop");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxConferencePlayAudioFileStop hConf=%d", hConf);

   SIPX_RESULT sr = SIPX_RESULT_INVALID_ARGS;

   if (hConf != SIPX_CONF_NULL)
   {
      SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_READ, stackLogger);

      if (pData)
      {
         // conference was found, just repost message
         if (pData->m_hCalls.entries() > 0)
         {
            pData->m_pInst->pCallManager->audioStopPlayback(pData->m_sConferenceId);
            sr = SIPX_RESULT_SUCCESS;
         }

         sipxConfReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
      }
   }

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxConferenceDestroy(SIPX_CONF hConf)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceDestroy");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxConferenceDestroy hConf=%d",
      hConf);

   SIPX_RESULT rc = SIPX_RESULT_INVALID_ARGS;

   SIPX_CONF_DATA *pData = sipxConfLookup(hConf, SIPX_LOCK_READ, stackLogger);
   if (pData)
   {
      pData->m_pInst->pCallManager->dropConference(pData->m_sConferenceId);

	  sipxConfReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
      rc = SIPX_RESULT_SUCCESS;
   }

   return rc;
}

SIPXTAPI_API SIPX_RESULT sipxConferenceLimitCodecPreferences(const SIPX_CONF hConf,
                                                             const char* szAudioCodecNames,
                                                             const char* szVideoCodecNames) 
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceLimitCodecPreferences");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxConferenceLimitCodecPreferences hCall=%d szAudioCodecNames=\"%s\"",
      hConf,
      (szAudioCodecNames) ? szAudioCodecNames : "");

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   // Test bandwidth for legal values
   if (hConf != SIPX_CONF_NULL)
   {
      SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_READ, stackLogger);

      if (pData)
      {
         if (!pData->m_sConferenceId.isNull())
         {
            // just reposts message
            pData->m_pInst->pCallManager->limitAbstractCallCodecPreferences(pData->m_sConferenceId,
               szAudioCodecNames,
               szVideoCodecNames);
            sr = SIPX_RESULT_SUCCESS;
         }
         else
         {
            sr = SIPX_RESULT_INVALID_ARGS;
         }

         sipxConfReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
      }      
   }
   else
   {
      sr = SIPX_RESULT_INVALID_ARGS;
   }

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxConferenceRenegotiateCodecPreferences(const SIPX_CONF hConf,
                                                                   const char* szAudioCodecNames,
                                                                   const char* szVideoCodecNames) 
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceRenegotiateCodecPreferences");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxConferenceRenegotiateCodecPreferences hCall=%d szAudioCodecNames=\"%s\"",
      hConf,
      (szAudioCodecNames) ? szAudioCodecNames : "");

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   // Test bandwidth for legal values
   if (hConf != SIPX_CONF_NULL)
   {
      SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_READ, stackLogger);

      if (pData)
      {
         if (!pData->m_sConferenceId.isNull())
         {
            // just reposts message
            pData->m_pInst->pCallManager->renegotiateCodecsAllConferenceConnections(pData->m_sConferenceId,
               szAudioCodecNames,
               szVideoCodecNames);

            sr = SIPX_RESULT_SUCCESS;
         }
         else
         {
            sr = SIPX_RESULT_INVALID_ARGS;
         }

         sipxConfReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
      }      
   }
   else
   {
      sr = SIPX_RESULT_INVALID_ARGS;
   }

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxConferenceAudioRecordFileStart(const SIPX_CONF hConf, const char* szFile)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxCallAudioRecordFileStart");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxConferenceAudioRecordFileStart hConf=%d szFile=%s", hConf, szFile);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   
   if (hConf != SIPX_CONF_NULL && szFile)
   {
      SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_READ, stackLogger);

      if (pData)
      {
         // conference was found, just repost message
         pData->m_pInst->pCallManager->audioRecordStart(pData->m_sConferenceId, szFile);
         sr = SIPX_RESULT_SUCCESS;
         sipxConfReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
      }
   }

   return sr;
}

SIPXTAPI_API SIPX_RESULT sipxConferenceAudioRecordFileStop(const SIPX_CONF hConf)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxConferenceAudioRecordFileStop");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxConferenceAudioRecordFileStop hConf=%d", hConf);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   if (hConf != SIPX_CONF_NULL)
   {
      SIPX_CONF_DATA* pData = sipxConfLookup(hConf, SIPX_LOCK_READ, stackLogger);

      if (pData)
      {
         // conference was found, just repost message
         pData->m_pInst->pCallManager->audioRecordStop(pData->m_sConferenceId);
         sr = SIPX_RESULT_SUCCESS;
         sipxConfReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
      }
   }

   return sr;
}
