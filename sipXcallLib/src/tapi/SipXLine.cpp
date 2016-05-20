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

#include <os/OsLock.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/UtlSListIterator.h>
#include <net/SipUserAgent.h>
#include "net/SipLineMgr.h"
#include <net/SipTransport.h>
#include "tapi/SipXLine.h"
#include "tapi/SipXHandleMap.h"
#include "tapi/SipXEvents.h"
#include <tapi/SipXLineEventListener.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
SipXHandleMap gLineHandleMap(1, SIPX_LINE_NULL);  /**< Global Map of line handles */

// GLOBAL FUNCTIONS

/* ============================ FUNCTIONS ================================= */


SIPX_LINE sipxLineLookupHandle(SIPX_INSTANCE_DATA* pInst,
                               const char* szLineURI, 
                               const char* szRequestUri) 
{ 
   SIPX_LINE hLine = SIPX_LINE_NULL; 

   hLine = sipxLineLookupHandleByURI(pInst, szLineURI);
   if (!hLine)
   {
      hLine = sipxLineLookupHandleByURI(pInst, szRequestUri);
   }

   return hLine;
}


static SIPX_LINE_DATA* createLineData(SIPX_INSTANCE_DATA* pInst, const Url& lineUri, const Url& fullLineUrl)
{
   SIPX_LINE_DATA* pData = new SIPX_LINE_DATA();
   // if there is allocation failure, std::bad_alloc exception is thrown
   // unless we use _set_new_handler to set new handler
   // we do not handle std::bad_alloc thus program will exit, which is ok
   // if we want to check for NULL, then we would have to use
   // new(std:::nothrow) instead of just new

   pData->m_fullLineUrl = fullLineUrl;
   pData->m_lineUri = lineUri;
   pData->m_pInst = pInst;

   return pData;
}

/**
 * @brief Finds a line by handle and acquires a lock on it.
 */

SIPX_LINE_DATA* sipxLineLookup(const SIPX_LINE hLine,
                               SIPX_LOCK_TYPE type,
                               const OsStackTraceLogger& oneBackInStack)
{
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxLineLookup", oneBackInStack);
   SIPX_LINE_DATA* pRC = NULL;
   OsStatus status = OS_FAILED; 

   gLineHandleMap.lock();
   pRC = (SIPX_LINE_DATA*)gLineHandleMap.findHandle(hLine);

   if (pRC)
   {
      if (validLineData(pRC))
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
         assert(false);
         pRC = NULL;
      }
   }

   gLineHandleMap.unlock();

   return pRC;
}


void sipxLineReleaseLock(SIPX_LINE_DATA* pData,
                         SIPX_LOCK_TYPE type,
                         const OsStackTraceLogger& oneBackInStack) 
{
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxLineReleaseLock", oneBackInStack);
   OsStatus status;

   if (type != SIPX_LOCK_NONE)
   {
      if (validLineData(pData))
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

/**
 * @brief Removes and deletes all lines. Lines are not unregistered.
 */

void sipxLineRemoveAll(const SIPX_INST hInst) 
{
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);
   if (pInst)
   {
      int linesCount = pInst->pLineManager->getNumLines();
      SIPX_LINE lines[1];
      size_t nLines = 0;

      for (size_t i = 0; i < (size_t)linesCount; i++)
      {
         // remove lines 1 by 1
         sipxLineGet(hInst, lines, 1, &nLines);
         if (nLines > 0)
         {
            sipxLineRemove(lines[0]);
         }
         nLines = 0;
      }
   }
}

/**
 * Frees line object by handle.
 */

void sipxLineObjectFree(const SIPX_LINE hLine)
{
   OsStackTraceLogger logItem(FAC_SIPXTAPI, PRI_DEBUG, "sipxLineObjectFree");
   SIPX_LINE_DATA* pData = NULL;

   gLineHandleMap.lock();
   pData = sipxLineLookup(hLine, SIPX_LOCK_WRITE, logItem);
   assert(pData);

   if (pData)
   {
      pData->m_pInst->lock.acquire();
      pData->m_pInst->nLines--;
      assert(pData->m_pInst->nLines >= 0);
      pData->m_pInst->lock.release();

      const void* pRC = gLineHandleMap.removeHandle(hLine);
      assert(pRC);
      gLineHandleMap.unlock();

      UtlVoidPtr* pValue;

      while ((pValue = (UtlVoidPtr*) pData->m_lineAliases.get()))
      {
         Url* pUri = (Url*) pValue->getValue();
         delete pUri;
         delete pValue;
      }

      delete pData;
   }
   else
   {
      // pData is NULL
      gLineHandleMap.unlock();
   }
}


UtlBoolean validLineData(const SIPX_LINE_DATA* pData) 
{
   UtlBoolean bValid = FALSE;

   if (pData && pData->m_pInst && 
      pData->m_pInst->pCallManager)
   {
      bValid = TRUE;
   }

   return bValid;
}


SIPX_LINE sipxLineLookupHandleByURI(SIPX_INSTANCE_DATA* pInst, const char* szURI)
{
   gLineHandleMap.lock(); // global lock for line deletion

   UtlHashMapIterator iter(gLineHandleMap);
   Url urlURI(szURI);

   UtlInt* pIndex = NULL;
   UtlVoidPtr* pObj = NULL;
   SIPX_LINE hLine = SIPX_LINE_NULL;

   // First pass: strict matching
   while ((pIndex = dynamic_cast<UtlInt*>(iter())))
   {
      pObj = dynamic_cast<UtlVoidPtr*>(gLineHandleMap.findValue(pIndex));

      if (pObj)
      {
         // got access to a value, unwrap it
         SIPX_LINE_DATA* pData = (SIPX_LINE_DATA*)pObj->getValue();

         if (pData)
         {
            // got access to line object, lock it
            OsLock lock(pData->m_mutex);

            if (pData->m_pInst == pInst)
            {
               // Check main line definition
               if (urlURI.isUserHostPortEqual(pData->m_lineUri))
               {
                  hLine = pIndex->getValue();
                  break;
               }

               // check for line aliases
               UtlVoidPtr* pValue;
               Url* pUrl;
               UtlSListIterator iterator(pData->m_lineAliases);

               while ((pValue = dynamic_cast<UtlVoidPtr*>(iterator())))
               {
                  pUrl = (Url*)pValue->getValue();

                  if (urlURI.isUserHostPortEqual(*pUrl))
                  {
                     hLine = pIndex->getValue();
                     break;
                  }
               }
            }
         }
      }
   }

   // Second pass: Relax port
   if (hLine == SIPX_LINE_NULL)
   {
      iter.reset();
      while ((pIndex = dynamic_cast<UtlInt*>(iter())))
      {
         pObj = dynamic_cast<UtlVoidPtr*>(gLineHandleMap.findValue(pIndex));

         if (pObj)
         {
            SIPX_LINE_DATA* pData = (SIPX_LINE_DATA*)pObj->getValue();
            if (pData)
            {
               // got access to line object, lock it
               OsLock lock(pData->m_mutex);

               if (pData->m_pInst == pInst)
               {
                  // Check main line definition
                  if (urlURI.isUserHostEqual(pData->m_lineUri))
                  {
                     hLine = pIndex->getValue();
                     break;
                  }

                  // Check for line aliases
                  UtlVoidPtr* pValue;
                  Url* pUrl;
                  UtlSListIterator iterator(pData->m_lineAliases);

                  while ((pValue = dynamic_cast<UtlVoidPtr*>(iterator())))
                  {
                     pUrl = (Url*) pValue->getValue();

                     if (urlURI.isUserHostEqual(*pUrl))
                     {
                        hLine = pIndex->getValue();
                        break;
                     }
                  }
               }
            }
         }
      }
   }

   // Third pass: username only
   if (hLine == SIPX_LINE_NULL)
   {
      iter.reset();
      while ((pIndex = dynamic_cast<UtlInt*>(iter())))
      {
         pObj = dynamic_cast<UtlVoidPtr*>(gLineHandleMap.findValue(pIndex));

         if (pObj)
         {
            SIPX_LINE_DATA* pData = (SIPX_LINE_DATA*) pObj->getValue();

            if (pData)
            {
               // got access to line object, lock it
               OsLock lock(pData->m_mutex);

               if (pData->m_pInst == pInst)
               {
                  UtlString uriUsername;
                  UtlString hostUsername;

                  urlURI.getUserId(uriUsername);
                  pData->m_lineUri.getUserId(hostUsername);

                  if (uriUsername.compareTo(hostUsername) == 0)
                  {
                     hLine = pIndex->getValue();
                     break;
                  }

                  // Check for line aliases
                  UtlVoidPtr* pValue;
                  Url* pUrl;
                  UtlSListIterator iterator(pData->m_lineAliases);

                  while ((pValue = dynamic_cast<UtlVoidPtr*>(iterator())))
                  {
                     pUrl = (Url*) pValue->getValue();
                     UtlString aliasUsername;

                     pUrl->getUserId(aliasUsername);

                     if (uriUsername.compareTo(aliasUsername) == 0)
                     {
                        hLine = pIndex->getValue();
                        break;
                     }
                  }
               }
            }
         }
      }
   }

   gLineHandleMap.unlock();

   return hLine;
}


/*********************************************************************/
/*       Public line handling functions                              */
/*********************************************************************/


SIPXTAPI_API SIPX_RESULT sipxLineRemove(SIPX_LINE hLine)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxLineRemove");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO, "sipxLineRemove hLine=%d", hLine);    

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   if (hLine)
   {
      SIPX_LINE_DATA* pData = sipxLineLookup(hLine, SIPX_LOCK_READ, stackLogger);

      if (pData)
      {
         Url lineURI(pData->m_lineUri);
         SIPX_INSTANCE_DATA* pInst = pData->m_pInst;

         sipxLineReleaseLock(pData, SIPX_LOCK_READ, stackLogger);

         if (pInst)
         {
            // we can't have lock while executing lineManager methods
            pInst->pLineManager->deleteLine(lineURI); 

            sr = SIPX_RESULT_SUCCESS;
         }
         sipxLineObjectFree(hLine);
      }
   }
   else
   {
      sr = SIPX_RESULT_INVALID_ARGS;
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxLineRegister(const SIPX_LINE hLine, const int bRegister)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxLineRegister");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxLineRegister hLine=%d bRegister=%d",
      hLine, bRegister);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   if (hLine)
   {
      SIPX_LINE_DATA* pData = sipxLineLookup(hLine, SIPX_LOCK_WRITE, stackLogger);
      if (pData)
      {
         Url lineURI(pData->m_lineUri);
         SIPX_INSTANCE_DATA* pInst = pData->m_pInst;

         sipxLineReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);

         if (pInst)
         {
            if (bRegister)
            {
               pInst->pLineManager->registerLine(lineURI);
            }
            else
            {
               pInst->pLineManager->unregisterLine(lineURI);
            }
            sr = SIPX_RESULT_SUCCESS;
         }
      }// else line was not found
   }// else hLine == 0

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxLineAddCredential(const SIPX_LINE hLine,                                                 
                                               const char* szUserID,
                                               const char* szPasswd,
                                               const char* szRealm)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxLineAddCredential");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxLineAddCredential hLine=%d userId=%s realm=%s",
      hLine, szUserID, szRealm);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_LINE_DATA* pData = sipxLineLookup(hLine, SIPX_LOCK_READ, stackLogger);

   if (pData)
   {
      Url lineURI(pData->m_lineUri);
      SIPX_INSTANCE_DATA* pInst = pData->m_pInst;

      sipxLineReleaseLock(pData, SIPX_LOCK_READ, stackLogger);

      if (szUserID && szPasswd)
      {
         UtlBoolean rc = pInst->pLineManager->addCredentialForLine(lineURI,
            szRealm,
            szUserID,
            szPasswd,
            HTTP_DIGEST_AUTHENTICATION);

         if (rc)
         {
            sr = SIPX_RESULT_SUCCESS;
         }
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

SIPXTAPI_API SIPX_RESULT sipxLineSetOutboundProxy(const SIPX_LINE hLine,                                                 
                                                  const char* szProxyServers)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxLineSetOutboundProxy");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxLineSetOutboundProxy hLine=%d szProxyServers=%s",
      hLine, szProxyServers);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_LINE_DATA* pData = sipxLineLookup(hLine, SIPX_LOCK_READ, stackLogger);

   if (pData)
   {
      Url lineURI(pData->m_lineUri);
      SIPX_INSTANCE_DATA* pInst = pData->m_pInst;

      sipxLineReleaseLock(pData, SIPX_LOCK_READ, stackLogger);

      UtlBoolean rc = pInst->pLineManager->setLineProxyServers(lineURI, szProxyServers);
      if (rc)
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

SIPXTAPI_API SIPX_RESULT sipxLineAddAlias(const SIPX_LINE hLine, const char* szLineURL) 
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxLineAddAlias");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxLineAddAlias hLine=%d szLineURL=%s",
      hLine, szLineURL);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   if (hLine)
   {
      SIPX_LINE_DATA* pData = sipxLineLookup(hLine, SIPX_LOCK_WRITE, stackLogger);
      if (pData)
      {
         Url aliasUri = SipLine::getLineUri(UtlString(szLineURL));
         UtlBoolean success = pData->m_pInst->pLineManager->addLineAlias(aliasUri, pData->m_lineUri);
         if (success)
         {
            pData->m_lineAliases.append(new UtlVoidPtr(new Url(aliasUri))); // this always succeeds
            sr = SIPX_RESULT_SUCCESS;
         }

         sipxLineReleaseLock(pData, SIPX_LOCK_WRITE, stackLogger);
      }                
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxLineGetContactInfo(const SIPX_LINE  hLine,
                                                char* szContactAddress,
                                                const size_t nContactAddressSize,
                                                int* contactPort,
                                                SIPX_CONTACT_TYPE* contactType,
                                                SIPX_TRANSPORT_TYPE* transport)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxLineGetContactInfo");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO, 
      "sipxLineGetContactInfo hLine=%d", hLine);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;

   SIPX_LINE_DATA* pData = sipxLineLookup(hLine, SIPX_LOCK_READ, stackLogger);
   if (pData)
   {
      SAFE_STRNCPY(szContactAddress, pData->m_contactUrl.getHostAddress().data(), nContactAddressSize);
      *contactPort = pData->m_contactUrl.getHostPort();
      *contactType = pData->m_contactType;
      *transport = pData->m_transport;

      sipxLineReleaseLock(pData, SIPX_LOCK_READ, stackLogger);

      sr = SIPX_RESULT_SUCCESS;
   }  
   
   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxLineFindByURI(const SIPX_INST hInst,
                                           const char* szURI,
                                           SIPX_LINE* phLine) 
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxLineFindByURI");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxLineFindByURI hInst=%p szURI=%s", hInst, szURI);

   SIPX_RESULT sr = SIPX_RESULT_INVALID_ARGS;

   if (hInst && szURI)
   {
      SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);
      *phLine = sipxLineLookupHandleByURI(pInst, szURI);

      if (*phLine != SIPX_LINE_NULL)
      {
         sr = SIPX_RESULT_SUCCESS;
      }
      else
      {
         sr = SIPX_RESULT_FAILURE;
      }
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxLineGet(const SIPX_INST hInst,
                                     SIPX_LINE lines[],
                                     const size_t max,
                                     size_t* actual)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxLineGet");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxLineGet hInst=%p",
      hInst);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);
   assert(pInst);

   if (pInst && lines && actual && max > 0)
   {
      *actual = 0; // zero number of found lines
      SipLine* pLine = NULL;
      UtlSList lineList; // list for holding SipLine objects

      pInst->pLineManager->getLineCopies(lineList);
      UtlSListIterator itor(lineList);
      // iterate through all lines
      while ((pLine = dynamic_cast<SipLine*>(itor())) != NULL && *actual < max)
      {
         lines[*actual] = sipxLineLookupHandleByURI(pInst, pLine->getLineUri().toString());
         *actual = *actual + 1;
      }

      sr = SIPX_RESULT_SUCCESS;
   }
   else
   {
      sr = SIPX_RESULT_INVALID_ARGS;
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxLineGetURI(const SIPX_LINE hLine,
                                        char* szBuffer,
                                        const size_t nBuffer,
                                        size_t* nActual)
{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxLineGetURI");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxLineGetURI hLine=%d",
      hLine);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_LINE_DATA* pData = sipxLineLookup(hLine, SIPX_LOCK_READ, stackLogger);


   if (pData)
   {
      if (szBuffer)
      {
         SAFE_STRNCPY(szBuffer, pData->m_lineUri.toString(), nBuffer);

         *nActual = strlen(szBuffer) + 1;
         sr = SIPX_RESULT_SUCCESS;
      }
      else
      {
         *nActual = strlen(pData->m_lineUri.toString()) + 1;
         sr = SIPX_RESULT_SUCCESS;
      }

      sipxLineReleaseLock(pData, SIPX_LOCK_READ, stackLogger);
   }

   return sr;
}


SIPXTAPI_API SIPX_RESULT sipxLineAdd(const SIPX_INST hInst,
                                     const char* szLineUrl,
                                     SIPX_LINE* phLine,
                                     SIPX_CONTACT_ID contactId,
                                     SIPX_TRANSPORT_TYPE transport)

{
   OsStackTraceLogger stackLogger(FAC_SIPXTAPI, PRI_DEBUG, "sipxLineAdd");
   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO,
      "sipxLineAdd hInst=%p lineUrl=%s, phLine=%p contactId=%d ",
      hInst, szLineUrl, phLine, contactId);

   SIPX_RESULT sr = SIPX_RESULT_FAILURE;
   SIPX_INSTANCE_DATA* pInst = SAFE_PTR_CAST(SIPX_INSTANCE_DATA, hInst);

   assert(szLineUrl);
   assert(phLine);

   if (pInst)
   {
      if (szLineUrl && phLine)
      {
         Url userEnteredUrl(szLineUrl); // for example "display name"<sip:number@domain;transport=tcp?headerParam=value>;fieldParam=value

         // Set the preferred contact
         SIPX_TRANSPORT_TYPE lineTransport = transport;
         SIPX_CONTACT_TYPE contactType = CONTACT_LOCAL;
         UtlString contactIp;
         int contactPort;

         // try to find contact by ID
         SipContact* pContact = pInst->pSipUserAgent->getContactDb().find(contactId);
         if (!pContact)
         {
            // contact was not found by contactId, use the first local contact, will be overridden later anyway
            // when sending via SipUserAgent
            pContact = pInst->pSipUserAgent->getContactDb().find((SIP_CONTACT_TYPE)contactType,
               (SIP_TRANSPORT_TYPE)lineTransport);
            if (!pContact)
            {
               // contact still not found, cannot continue
               return SIPX_RESULT_INVALID_ARGS;
            }
         }
         
         contactType = (SIPX_CONTACT_TYPE)pContact->getContactType();
         if (lineTransport != TRANSPORT_AUTO &&
            lineTransport != (SIPX_TRANSPORT_TYPE)pContact->getTransportType())
         {
            delete pContact;
            pContact = NULL;
            // contact transport and supplied transport don't match
            return SIPX_RESULT_INVALID_ARGS;
         }
         // if automatic transport was passed, use contact transport type
         lineTransport = (SIPX_TRANSPORT_TYPE)pContact->getTransportType();

         contactIp = pContact->getIpAddress(); // try to suggest contact IP address as well
         contactPort = pContact->getPort();
         delete pContact;
         pContact = NULL;

         SipLine line(userEnteredUrl, SipLine::LINE_STATE_UNKNOWN);
         Url lineUri = line.getLineUri(); // gets unique line uri, for line matching
         Url fullLineUrl = line.getFullLineUrl(); // gets full line url for constructing sip message from field
         line.setPreferredTransport((SIP_TRANSPORT_TYPE)lineTransport);
         // if automatic contact is used, allow contact override, otherwise disable it
         line.setAllowContactOverride(contactId == SIPX_AUTOMATIC_CONTACT_ID);
         line.setPreferredContact(contactIp, contactPort);

         UtlBoolean bRC = pInst->pLineManager->addLine(line);
         if (bRC)
         {
            SIPX_LINE_DATA* pData = createLineData(pInst, lineUri, fullLineUrl);

            if (pData)
            {
               pData->m_contactUrl = line.getPreferredContactUri();
               pData->m_transport = lineTransport;
               pData->m_contactType = contactType;

               UtlBoolean res = gLineHandleMap.allocHandle(*phLine, pData);
               if (res)
               {
                  pData->m_pInst->lock.acquire();
                  pData->m_pInst->nLines++;
                  pData->m_pInst->lock.release();

                  sr = SIPX_RESULT_SUCCESS;

                  pInst->pLineManager->setStateForLine(lineUri, SipLine::LINE_STATE_PROVISIONED);

                  pInst->pLineEventListener->sipxFireLineEvent(szLineUrl,
                                                               LINESTATE_PROVISIONED,
                                                               LINESTATE_PROVISIONED_NORMAL);
               }
               else
               {
                  // handle allocation failure
                  OsSysLog::add(FAC_SIPXTAPI, PRI_ERR, 
                     "allocHandle failed to allocate a handle");

                  // get rid of new SIPX_LINE_DATA
                  delete pData;

                  // delete line from line manager
                  pInst->pLineManager->deleteLine(lineUri);
               }
            }
            else
            {
               sr = SIPX_RESULT_OUT_OF_MEMORY;
            }
         }
      }
      else
      {
         sr = SIPX_RESULT_INVALID_ARGS;
      }
   }

   OsSysLog::add(FAC_SIPXTAPI, PRI_INFO, "sipxLineAdd hLine=%d", *phLine);

   return sr;
}
