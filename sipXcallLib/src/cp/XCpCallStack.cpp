//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsLock.h>
#include <os/OsReadLock.h>
#include <os/OsWriteLock.h>
#include <os/OsPtrLock.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/UtlSList.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlPtr.h>
#include <cp/XCpCallIdUtil.h>
#include <cp/XCpCallStack.h>
#include <cp/XCpAbstractCall.h>
#include <cp/XCpCall.h>
#include <cp/XCpConference.h>
#include <cp/msg/AcGainFocusMsg.h>
#include <cp/msg/AcYieldFocusMsg.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

XCpCallStack::XCpCallStack()
: m_sAbstractCallInFocus(NULL)
, m_focusMutex(OsMutex::Q_FIFO)
, m_memberMutex(OsMutex::Q_FIFO)
{

}

XCpCallStack::~XCpCallStack()
{
   yieldFocus(FALSE);
   // no lock needed, we are in destructor
   m_abstractCallIdMap.destroyAll();
   m_sipCallIdMap.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

UtlBoolean XCpCallStack::findAbstractCall(const UtlString& sAbstractCallId,
                                          OsPtrLock<XCpAbstractCall>& ptrLock) const
{
   OsReadLock lock(m_memberMutex);
   // cannot reuse existing method, as OsPtrLock<XCpAbstractCall>& cannot be cast to OsPtrLock<XCpCall>&
   XCpAbstractCall* pAbstractCall = dynamic_cast<XCpAbstractCall*>(m_abstractCallIdMap.findValue(&sAbstractCallId));
   if (pAbstractCall)
   {
      ptrLock = pAbstractCall;
      return TRUE;
   }

   ptrLock = NULL;
   return FALSE;
}

UtlBoolean XCpCallStack::findAbstractCall(const SipDialog& sSipDialog,
                                          OsPtrLock<XCpAbstractCall>& ptrLock) const
{
   OsReadLock lock(m_memberMutex);
   XCpAbstractCall* pNotEstablishedMatch = NULL;
   UtlPtr<XCpAbstractCall>* pAbstractPtr = NULL;

   // use fast lookup by sip call-id
   UtlString sSipCallId(sSipDialog.getCallId());
   UtlSList* pList = dynamic_cast<UtlSList*>(m_sipCallIdMap.findValue(&sSipCallId));
   if (pList)
   {
      // iterate through list and ask every call if it has given sip dialog
      UtlSListIterator callListItor(*pList);
      XCpAbstractCall* pAbstractCall = NULL;

      while(callListItor()) // go to next pair
      {
         pAbstractPtr = dynamic_cast<UtlPtr<XCpAbstractCall>*>(callListItor.item());
         if (pAbstractPtr && pAbstractPtr->getValue())
         {
            pAbstractCall = pAbstractPtr->getValue();
            SipDialog::DialogMatchEnum matchResult = pAbstractCall->hasSipDialog(sSipDialog);
            if (matchResult == SipDialog::DIALOG_ESTABLISHED_MATCH ||
               matchResult == SipDialog::DIALOG_INITIAL_INITIAL_MATCH)
            {
               // perfect match, call-id and both tags match
               // initial match is also perfect if supplied dialog is initial
               ptrLock = pAbstractCall;
               return TRUE;
            }
            else if (matchResult != SipDialog::DIALOG_MISMATCH)
            {
               // partial match, call-id match but only 1 tag matches, 2nd tag is not present
               pNotEstablishedMatch = pAbstractCall; // lock it later
            }
         }
      }
   }

   if (pNotEstablishedMatch)
   {
      ptrLock = pNotEstablishedMatch;
      return TRUE;
   }

   return FALSE;
}

UtlBoolean XCpCallStack::findSomeAbstractCall(const UtlString& sAvoidAbstractCallId,
                                              OsPtrLock<XCpAbstractCall>& ptrLock) const
{
   OsReadLock lock(m_memberMutex);

   // try to get next call
   {
      UtlHashMapIterator callMapItor(m_abstractCallIdMap);
      XCpAbstractCall* pAbstractCall = NULL;

      while(callMapItor()) // go to next pair
      {
         pAbstractCall = dynamic_cast<XCpAbstractCall*>(callMapItor.value());
         if (pAbstractCall && sAvoidAbstractCallId.compareTo(pAbstractCall->getId()) != 0)
         {
            // we found some call and sAvoidAbstractCallId is different than its Id
            ptrLock = pAbstractCall; // lock call
            return TRUE;
         }
      }
   }

   ptrLock = NULL;
   return FALSE;
}

UtlBoolean XCpCallStack::findCall(const UtlString& sId,
                                  OsPtrLock<XCpCall>& ptrLock) const
{
   OsReadLock lock(m_memberMutex);
   XCpCall* pCall = dynamic_cast<XCpCall*>(m_abstractCallIdMap.findValue(&sId));
   if (pCall)
   {
      ptrLock = pCall;
      return TRUE;
   }

   ptrLock = NULL;
   return FALSE;
}

UtlBoolean XCpCallStack::findCall(const SipDialog& sSipDialog,
                                  OsPtrLock<XCpCall>& ptrLock) const
{
   OsReadLock lock(m_memberMutex);
   XCpCall* pNotEstablishedMatch = NULL;
   UtlPtr<XCpAbstractCall>* pAbstractPtr = NULL;

   // use fast lookup by sip call-id
   UtlString sSipCallId(sSipDialog.getCallId());
   UtlSList* pList = dynamic_cast<UtlSList*>(m_sipCallIdMap.findValue(&sSipCallId));
   if (pList)
   {
      // iterate through list and ask every call if it has given sip dialog
      UtlSListIterator callListItor(*pList);
      XCpCall* pCall = NULL;

      while(callListItor()) // go to next pair
      {
         pAbstractPtr = dynamic_cast<UtlPtr<XCpAbstractCall>*>(callListItor.item());
         if (pAbstractPtr)
         {
            pCall = dynamic_cast<XCpCall*>(pAbstractPtr->getValue());
            if (pCall)
            {
               SipDialog::DialogMatchEnum matchResult = pCall->hasSipDialog(sSipDialog);
               if (matchResult == SipDialog::DIALOG_ESTABLISHED_MATCH ||
                  matchResult == SipDialog::DIALOG_INITIAL_INITIAL_MATCH)
               {
                  // perfect match, call-id and both tags match
                  // initial match is also perfect if supplied dialog is initial
                  ptrLock = pCall;
                  return TRUE;
               }
               else if (matchResult != SipDialog::DIALOG_MISMATCH)
               {
                  // partial match, call-id match but only 1 tag matches, 2nd tag is not present
                  pNotEstablishedMatch = pCall; // lock it later
               }
            }
         }
      }
   }

   if (pNotEstablishedMatch)
   {
      ptrLock = pNotEstablishedMatch;
      return TRUE;
   }

   return FALSE;
}

UtlBoolean XCpCallStack::findConference(const UtlString& sId,
                                        OsPtrLock<XCpConference>& ptrLock) const
{
   OsReadLock lock(m_memberMutex);
   XCpConference* pConference = dynamic_cast<XCpConference*>(m_abstractCallIdMap.findValue(&sId));
   if (pConference)
   {
      ptrLock = pConference;
      return TRUE;
   }

   ptrLock = NULL;
   return FALSE;
}

UtlBoolean XCpCallStack::findConference(const SipDialog& sSipDialog,
                                        OsPtrLock<XCpConference>& ptrLock) const
{
   OsReadLock lock(m_memberMutex);
   XCpConference* pNotEstablishedMatch = NULL;
   UtlPtr<XCpAbstractCall>* pAbstractPtr = NULL;

   // use fast lookup by sip call-id
   UtlString sSipCallId(sSipDialog.getCallId());
   UtlSList* pList = dynamic_cast<UtlSList*>(m_sipCallIdMap.findValue(&sSipCallId));
   if (pList)
   {
      // iterate through list and ask every call if it has given sip dialog
      UtlSListIterator conferenceListItor(*pList);
      XCpConference* pConference = NULL;
      while(conferenceListItor()) // go to next pair
      {
         pAbstractPtr = dynamic_cast<UtlPtr<XCpAbstractCall>*>(conferenceListItor.item());
         if (pAbstractPtr)
         {
            pConference = dynamic_cast<XCpConference*>(pAbstractPtr->getValue());
            if (pConference)
            {
               SipDialog::DialogMatchEnum matchResult = pConference->hasSipDialog(sSipDialog);
               if (matchResult == SipDialog::DIALOG_ESTABLISHED_MATCH ||
                  matchResult == SipDialog::DIALOG_INITIAL_INITIAL_MATCH)
               {
                  // perfect match, call-id and both tags match
                  // initial match is also perfect if supplied dialog is initial
                  ptrLock = pConference;
                  return TRUE;
               }
               else if (matchResult != SipDialog::DIALOG_MISMATCH)
               {
                  // partial match, call-id match but only 1 tag matches, 2nd tag is not present
                  pNotEstablishedMatch = pConference; // lock it later
               }
            }
         }
      }
   }

   if (pNotEstablishedMatch)
   {
      ptrLock = pNotEstablishedMatch;
      return TRUE;
   }

   return FALSE;
}

UtlBoolean XCpCallStack::findHandlingAbstractCall(const SipMessage& rSipMessage,
                                                  OsPtrLock<XCpAbstractCall>& ptrLock) const
{
   SipDialog sipDialog(&rSipMessage);
   return findAbstractCall(sipDialog, ptrLock);
}

UtlBoolean XCpCallStack::findConferenceByUri(const Url& requestUri, OsPtrLock<XCpConference>& ptrLock) const
{
   UtlPtr<XCpAbstractCall>* pAbstractPtr = NULL;
   XCpConference* pPartialMatch = NULL;
   XCpConference* pConference = NULL;
   Url conferenceUri;

   OsReadLock lock(m_memberMutex);
   UtlHashMapIterator conferenceMapItor(m_abstractCallIdMap);

   while(conferenceMapItor())
   {
      pConference = dynamic_cast<XCpConference*>(conferenceMapItor.value());
      if (pConference)
      {
         pConference->getConferenceUri(conferenceUri);
         if (!conferenceUri.isNull())
         {
            if (conferenceUri.isUserHostEqual(requestUri))
            {
               ptrLock = pConference;
               return TRUE;
            }
            else if (!pPartialMatch && conferenceUri.isUserEqual(requestUri))
            {
               pPartialMatch = pConference;
            }
         }
      }
   }

   if (pPartialMatch)
   {
      ptrLock = pPartialMatch;
      return TRUE;
   }

   return FALSE;
}

UtlBoolean XCpCallStack::push(XCpCall& call)
{
   OsReadLock lock(m_memberMutex); // we use write lock only when deleting, not needed when adding

   UtlCopyableContainable *pKey = call.getId().clone();
   UtlContainable *pResult = m_abstractCallIdMap.insertKeyAndValue(pKey, &call);
   if (pResult)
   {
      return TRUE;
   }
   else
   {
      delete pKey;
      pKey = NULL;
      return FALSE;
   }
}

UtlBoolean XCpCallStack::push(XCpConference& conference)
{
   OsReadLock lock(m_memberMutex); // we use write lock only when deleting, not needed when adding

   UtlCopyableContainable *pKey = conference.getId().clone();
   UtlContainable *pResult = m_abstractCallIdMap.insertKeyAndValue(pKey, &conference);
   if (pResult)
   {
      return TRUE;
   }
   else
   {
      delete pKey;
      pKey = NULL;
      return FALSE;
   }
}

UtlBoolean XCpCallStack::deleteCall(const UtlString& sCallId)
{
   OsWriteLock lock(m_memberMutex);
   // yield focus
   // nobody will be able to give us focus back after we yield it, because we hold m_memberMutex
   yieldFocus(sCallId);

   // avoid findCall, as we don't need that much locking
   UtlContainable *pValue = NULL;
   UtlContainable *pKey = m_abstractCallIdMap.removeKeyAndValue(&sCallId, pValue);
   if(pKey)
   {
      XCpCall *pCall = dynamic_cast<XCpCall*>(pValue);
      if (pCall)
      {
         // call was found
         pCall->acquireExclusive(); // lock the call exclusively for delete
         delete pCall;
         pCall = NULL;
      }
      else
      {
         OsSysLog::add(FAC_CP, PRI_WARNING, "Unexpected state. Key was removed from call hashmap but value was NULL.");
      }
      delete pKey;
      pKey = NULL;
      return TRUE;
   }
   return FALSE;
}

UtlBoolean XCpCallStack::deleteConference(const UtlString& sConferenceId)
{
   OsWriteLock lock(m_memberMutex);
   // yield focus
   // nobody will be able to give us focus back after we yield it, because we hold m_memberMutex
   yieldFocus(sConferenceId);

   // avoid findConference, as we don't need that much locking
   UtlContainable *pValue = NULL;
   UtlContainable *pKey = m_abstractCallIdMap.removeKeyAndValue(&sConferenceId, pValue);
   if(pKey)
   {
      XCpConference *pConference = dynamic_cast<XCpConference*>(pValue);
      if (pConference)
      {
         // conference was found
         pConference->acquireExclusive(); // lock the conference exclusively for delete
         delete pConference;
         pConference = NULL;
      }
      else
      {
         OsSysLog::add(FAC_CP, PRI_WARNING, "Unexpected state. Key was removed from conference hashmap but value was NULL.");
      }
      delete pKey;
      pKey = NULL;
      return TRUE;
   }
   return FALSE;
}

UtlBoolean XCpCallStack::deleteAbstractCall(const UtlString& sAbstractCallId)
{
   XCpCallIdUtil::ID_TYPE type = XCpCallIdUtil::getIdType(sAbstractCallId);
   UtlBoolean result = FALSE;

   switch(type)
   {
   case XCpCallIdUtil::ID_TYPE_CALL:
      {
         deleteCall(sAbstractCallId);
      }
   case XCpCallIdUtil::ID_TYPE_CONFERENCE:
      {
         deleteConference(sAbstractCallId);
      }
   default:
      break;
   }

   return result;
}

OsStatus XCpCallStack::gainFocus(const UtlString& sAbstractCallId,
                                 UtlBoolean bGainOnlyIfNoFocusedCall /*= FALSE*/)
{
#ifndef DISABLE_LOCAL_AUDIO
   OsStatus result = OS_FAILED;
   OsLock lock(m_focusMutex);

   if (!m_sAbstractCallInFocus.isNull())
   {
      // some call is focused
      if (bGainOnlyIfNoFocusedCall)
      {
         // some call is focused, then we don't want to focus
         return OS_SUCCESS;
      }

      if (m_sAbstractCallInFocus.compareTo(sAbstractCallId) != 0) // check that calls aren't the same
      {
         result = yieldFocus(sAbstractCallId, FALSE); // defocus focused call
      }
      else
      {
         return OS_SUCCESS; // gain focus attempted on call in focus
      }
   }

   {
      OsPtrLock<XCpAbstractCall> ptrLock; // scoped auto pointer lock
      UtlBoolean resFind = findAbstractCall(sAbstractCallId, ptrLock);
      if (resFind)
      {
         // send gain focus command to new call
         AcGainFocusMsg gainFocusCommand;
         // we found call and have a lock on it
         ptrLock->postMessage(gainFocusCommand);
         m_sAbstractCallInFocus = sAbstractCallId;
         result = OS_SUCCESS;
      }
   }

   return result;
#else
   return OS_SUCCESS;
#endif
}

OsStatus XCpCallStack::gainNextFocus(const UtlString& sAvoidAbstractCallId)
{
#ifndef DISABLE_LOCAL_AUDIO
   OsStatus result = OS_FAILED;
   OsLock lock(m_focusMutex); // need to hold lock of the whole time to ensure consistency

   if (m_sAbstractCallInFocus.isNull())
   {
      // no call has focus
      OsPtrLock<XCpAbstractCall> ptrLock; // scoped auto pointer lock
      UtlBoolean resFind = findSomeAbstractCall(sAvoidAbstractCallId, ptrLock);// avoids sAvoidAbstractCallId when looking for next call
      if (resFind)
      {
         // send gain focus command to new call
         AcGainFocusMsg gainFocusCommand;
         // we found call and have a lock on it
         ptrLock->postMessage(gainFocusCommand);
         m_sAbstractCallInFocus = ptrLock->getId();
         result = OS_SUCCESS;
      }
   }

   return result;
#else
   return OS_SUCCESS;
#endif
}

OsStatus XCpCallStack::yieldFocus(const UtlString& sAbstractCallId,
                                  UtlBoolean bShiftFocus /*= TRUE*/)
{
#ifndef DISABLE_LOCAL_AUDIO
   OsStatus result = OS_FAILED;
   OsLock lock(m_focusMutex); // need to hold lock of the whole time to ensure consistency

   if (m_sAbstractCallInFocus.compareTo(sAbstractCallId) == 0)
   {
      {
         OsPtrLock<XCpAbstractCall> ptrLock; // scoped auto pointer lock
         UtlBoolean resFind = findAbstractCall(m_sAbstractCallInFocus, ptrLock);
         if (resFind)
         {
            // send defocus command to old call
            AcYieldFocusMsg defocusCommand;
            // we found call and have a lock on it
            ptrLock->postMessage(defocusCommand);
            result = OS_SUCCESS;
         }
      }

      if (bShiftFocus)
      {
         UtlString sAvoidAbstractCallId(m_sAbstractCallInFocus);
         m_sAbstractCallInFocus.remove(0);
         result = gainNextFocus(sAvoidAbstractCallId);
      }
      else
      {
         m_sAbstractCallInFocus.remove(0);
      }
   }

   return result;
#else
   return OS_SUCCESS;
#endif
}

OsStatus XCpCallStack::yieldFocus(UtlBoolean bShiftFocus /*= TRUE*/)
{
#ifndef DISABLE_LOCAL_AUDIO
   OsStatus result = OS_FAILED;
   OsLock lock(m_focusMutex); // need to hold lock of the whole time to ensure consistency

   if (!m_sAbstractCallInFocus.isNull())
   {
      {
         OsPtrLock<XCpAbstractCall> ptrLock; // scoped auto pointer lock
         UtlBoolean resFind = findAbstractCall(m_sAbstractCallInFocus, ptrLock);
         if (resFind)
         {
            // send defocus command to old call
            AcYieldFocusMsg defocusCommand;
            // we found call and have a lock on it
            ptrLock->postMessage(defocusCommand);
            result = OS_SUCCESS;
         }
      }

      if (bShiftFocus)
      {
         UtlString sAvoidAbstractCallId(m_sAbstractCallInFocus);
         m_sAbstractCallInFocus.remove(0);
         result = gainNextFocus(sAvoidAbstractCallId);
      }
      else
      {
         m_sAbstractCallInFocus.remove(0);
      }
   }

   return result;
#else
   return OS_SUCCESS;
#endif
}

void XCpCallStack::shutdownAllAbstractCallThreads()
{
   OsReadLock lock(m_memberMutex);

   UtlHashMapIterator callMapItor(m_abstractCallIdMap);
   XCpAbstractCall* pCall = NULL;

   while(callMapItor()) // go to next pair
   {
      pCall = dynamic_cast<XCpAbstractCall*>(callMapItor.value());
      if (pCall)
      {
         pCall->requestShutdown();
      }
   }
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

int XCpCallStack::getCallCount() const
{
   OsReadLock lock(m_memberMutex);
   int count = 0;

   UtlHashMapIterator callMapItor(m_abstractCallIdMap);
   XCpAbstractCall* pCall = NULL;

   while(callMapItor()) // go to next pair
   {
      pCall = dynamic_cast<XCpAbstractCall*>(callMapItor.value());
      if (pCall)
      {
         count += pCall->getCallCount();
      }
   }

   return count;
}

OsStatus XCpCallStack::getCallIds(UtlSList& callIdList) const
{
   OsReadLock lock(m_memberMutex);

   UtlHashMapIterator callMapItor(m_abstractCallIdMap);
   XCpCall* pCall = NULL;

   while(callMapItor()) // go to next pair
   {
      // rely on dynamic cast to return only XCpCall instances
      pCall = dynamic_cast<XCpCall*>(callMapItor.value());
      if (pCall)
      {
         callIdList.insert(pCall->getId().clone());
      }
   }

   return OS_SUCCESS;
}

OsStatus XCpCallStack::getConferenceIds(UtlSList& conferenceIdList) const
{
   OsReadLock lock(m_memberMutex);

   UtlHashMapIterator conferenceMapItor(m_abstractCallIdMap);
   XCpConference* pConference = NULL;
   while(conferenceMapItor()) // go to next pair
   {
      // rely on dynamic cast to return only XCpConference instances
      pConference = dynamic_cast<XCpConference*>(conferenceMapItor.value());
      if (pConference)
      {
         conferenceIdList.insert(pConference->getId().clone());
      }
   }

   return OS_SUCCESS;
}

void XCpCallStack::onConnectionAdded(const UtlString& sSipCallId,
                                     XCpAbstractCall* pAbstractCall)
{
   OsReadLock lock(m_memberMutex); // we use write lock only when deleting, not needed when adding

   UtlSList* pList = dynamic_cast<UtlSList*>(m_sipCallIdMap.findValue(&sSipCallId));
   if (!pList)
   {
      // list doesn't exist, add it
      pList = new UtlSList();
      m_sipCallIdMap.insertKeyAndValue(sSipCallId.clone(), pList);
   }

   if (pList)
   {
      // list exists, just add new item. Wrap it in UtlPtr, as otherwise it would get deleted when m_sipCallIdMap is destroyed
      // we are not allowed to delete XCpAbstractCall in this list
      pList->insert(new UtlPtr<XCpAbstractCall>(pAbstractCall, FALSE));
   }
}

void XCpCallStack::onConnectionRemoved(const UtlString& sSipCallId,
                                       XCpAbstractCall* pAbstractCall)
{
   OsWriteLock lock(m_memberMutex);

   UtlSList* pList = dynamic_cast<UtlSList*>(m_sipCallIdMap.findValue(&sSipCallId));
   if (pList)
   {
      UtlPtr<XCpAbstractCall> ptr(pAbstractCall, FALSE);
      pList->destroy(&ptr); // remove object by equality
   }
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

