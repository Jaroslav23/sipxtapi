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
#if defined(_VXWORKS)
#   include <taskLib.h>
#   include <netinet/in.h>
#endif

#include <stdio.h>
#include <stdlib.h>

// APPLICATION INCLUDES
#include "utl/UtlHashBagIterator.h"
#include "os/OsDateTime.h"
#include "os/OsQueuedEvent.h"
#include "os/OsTimer.h"
#include "os/OsEventMsg.h"
#include "os/OsConfigDb.h"
#include "os/OsRWMutex.h"
#include "os/OsReadLock.h"
#include "os/OsWriteLock.h"
#include "os/OsLock.h"
#include "net/Url.h"
#include "net/SipLineMgr.h"
#include "net/SipObserverCriteria.h"
#include "net/SipUserAgent.h"
#include "net/SipMessage.h"
#include "net/NetMd5Codec.h"
#include "net/SipRefreshMgr.h"
#include <net/SipLineCredential.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC INITIALIZERS

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SipLineMgr::SipLineMgr(SipRefreshMgr *refershMgr)
: OsServerTask("SipLineMgr-%d")
, SipLineProvider()
, m_pRefreshMgr(refershMgr)
, m_mutex(OsMutex::Q_FIFO)
{
}

SipLineMgr::~SipLineMgr()
{
   OsSysLog::add(FAC_LINE_MGR, PRI_INFO, "SipLineMgr is shutting down...");

   dumpLines();
   waitUntilShutDown();

   // refresh manager is not deleted by us
   m_pRefreshMgr = NULL;
}

void SipLineMgr::startLineMgr()
{
   if(!isStarted())
   {
      OsSysLog::add(FAC_LINE_MGR, PRI_INFO, "SipLineMgr is starting...");
      // start the thread
      start();
   }
}

UtlBoolean SipLineMgr::handleMessage(OsMsg &eventMessage)
{
   UtlBoolean messageProcessed = FALSE;

   int msgType = eventMessage.getMsgType();
   int msgSubType = eventMessage.getMsgSubType();

   // do nothing with the message

   return  messageProcessed;
}

UtlBoolean SipLineMgr::addLine(SipLine& line)
{
   OsLock lock(m_mutex);

   if (m_listList.add(line))
   {
      syslog(FAC_LINE_MGR, PRI_INFO, "SipLineMgr::addLine added line: %s",
         line.getLineUri().toString().data());

      return TRUE;
   }

   return FALSE;
}

UtlBoolean SipLineMgr::deleteLine(const Url& lineUri)
{
   Url fullLineUrl;
   {
     OsLock lock(m_mutex);

     SipLine *pLine = m_listList.getLine(lineUri);
     if (!pLine)
     {
       syslog(FAC_LINE_MGR, PRI_ERR, "SipLineMgr::deleteLine unable to delete line (not found): %s",
         lineUri.toString().data());
       return FALSE;
     }

     fullLineUrl = pLine->getFullLineUrl();
     pLine = NULL;

     if (m_listList.remove(lineUri))
     {
       syslog(FAC_LINE_MGR, PRI_INFO, "SipLineMgr::deleteLine deleted line: %s",
         lineUri.toString().data());

       if(fullLineUrl.isNull() == FALSE)
       {
         m_pRefreshMgr->deleteUser(fullLineUrl);
       }

       return TRUE;
     }
   }

   return FALSE;
}

void SipLineMgr::deleteAllLines()
{
   OsLock lock(m_mutex);

   m_listList.removeAll();
}

UtlBoolean SipLineMgr::registerLine(const Url& lineURI)
{
   Url preferredContact;
   UtlBoolean bAllowContactOverride;
   Url fullLineUrl;
   SIP_TRANSPORT_TYPE preferredTransport;

   {
      OsLock lock(m_mutex); // scoped lock

      SipLine *pLine = m_listList.getLine(lineURI);

      if (!pLine)
      {
         syslog(FAC_LINE_MGR, PRI_ERR, "unable to enable line (not found): %s",
            lineURI.toString().data());
         return FALSE;
      }

      pLine->setState(SipLine::LINE_STATE_TRYING);
      preferredContact = pLine->getPreferredContactUri();
      fullLineUrl = pLine->getFullLineUrl();
      bAllowContactOverride = pLine->getAllowContactOverride();
      preferredTransport = pLine->getPreferredTransport();
      pLine = NULL;
   }

   if (!m_pRefreshMgr->newRegisterMsg(fullLineUrl, preferredContact, bAllowContactOverride,
      preferredTransport, -1))
   {
      //duplicate ...call reregister
      m_pRefreshMgr->reRegister(fullLineUrl);
   }

   syslog(FAC_LINE_MGR, PRI_INFO, "enabled line: %s", lineURI.toString().data());

   return TRUE;
}

UtlBoolean SipLineMgr::unregisterLine(const Url& lineURI)
{
   SipLine::LineStateEnum lineState = SipLine::LINE_STATE_UNKNOWN;

   Url fullLineUrl;
   {
      OsLock lock(m_mutex); // scoped lock

      SipLine *pLine = m_listList.getLine(lineURI);
      if (!pLine)
      {
         syslog(FAC_LINE_MGR, PRI_ERR, "SipLineMgr::unregisterLine unable to disable line (not found): %s",
            lineURI.toString().data());
         return FALSE;
      }

      lineState = pLine->getState();
      fullLineUrl = pLine->getFullLineUrl();
      pLine = NULL;
   }

   if (lineState == SipLine::LINE_STATE_REGISTERED ||
       lineState == SipLine::LINE_STATE_TRYING)
   {
      m_pRefreshMgr->unRegisterUser(fullLineUrl);
      syslog(FAC_LINE_MGR, PRI_INFO, "SipLineMgr::unregisterLine disabled line: %s",
         lineURI.toString().data());
      return TRUE;
   }

   return FALSE;
}

void SipLineMgr::getLineCopies(UtlSList& lineList) const
{
   OsLock lock(m_mutex); // scoped lock
   m_listList.getLineCopies(lineList);
}

void SipLineMgr::getLineUris(UtlSList& lineUris) const
{
   OsLock lock(m_mutex); // scoped lock
   m_listList.getLineUris(lineUris);
}

UtlBoolean SipLineMgr::getLineCopy(const Url& lineUri, SipLine& sipLine) const
{
   OsLock lock(m_mutex); // scoped lock

   SipLine *pLine = m_listList.getLine(lineUri); // this is fast lookup by hashcode
   if (pLine)
   {
      sipLine = *pLine;
      return TRUE;
   }

   return FALSE;
}

size_t SipLineMgr::getNumLines() const
{
   OsLock lock(m_mutex); // scoped lock
   return m_listList.getLinesCount();
}

// requires external m_mutex lock
const SipLine* SipLineMgr::findLine(const Url& lineUri,
                                    const UtlString& userId) const
{
   return m_listList.findLine(lineUri, userId);
}

UtlBoolean SipLineMgr::lineExists(const Url& lineUri,
                                  const UtlString& userId) const
{
   OsLock lock(m_mutex); // scoped lock
   return findLine(lineUri, userId) != NULL; // if we found something, then line exists
}

UtlBoolean SipLineMgr::addCredentialForLine(const Url& lineUri,
                                            const UtlString& strRealm,
                                            const UtlString& strUserID,
                                            const UtlString& strPasswd,
                                            const UtlString& type)
{
   OsLock lock(m_mutex); // scoped lock

   SipLine *pLine = m_listList.getLine(lineUri);
   if (pLine)
   {
      return pLine->addCredential(strRealm , strUserID, strPasswd, type);
   }

   return FALSE;
}

UtlBoolean SipLineMgr::addCredentialForLine(const Url& lineUri,
                                            const SipLineCredential& credential)
{
   OsLock lock(m_mutex); // scoped lock

   SipLine *pLine = m_listList.getLine(lineUri);
   if (pLine)
   {
      return pLine->addCredential(credential);
   }

   return FALSE;
}

UtlBoolean SipLineMgr::deleteCredentialForLine(const Url& lineUri,
                                               const UtlString& strRealm,
                                               const UtlString& type)
{
   OsLock lock(m_mutex); // scoped lock

   SipLine *pLine = m_listList.getLine(lineUri);
   if (pLine)
   {
      return pLine->removeCredential(type, strRealm);
   }

   return FALSE;
}

UtlBoolean SipLineMgr::deleteAllCredentialsForLine(const Url& lineUri)
{
   OsLock lock(m_mutex); // scoped lock

   SipLine *pLine = m_listList.getLine(lineUri);
   if (pLine)
   {
      pLine->removeAllCredentials();
      return TRUE;
   }

   return FALSE;
}

UtlBoolean SipLineMgr::findLineCopy(const Url& lineUri,
                                    const UtlString& userId,
                                    SipLine& sipLine) const
{
   OsLock lock(m_mutex); // scoped lock

   const SipLine *pLine = findLine(lineUri, userId);
   if (pLine)
   {
      // if line was found, copy contents into sipLine
      sipLine = *pLine;
   }

   return pLine != NULL;
}

UtlBoolean SipLineMgr::setLineProxyServers(const Url& lineUri,
                                           const UtlString& proxyServers)
{
   OsLock lock(m_mutex); // scoped lock

   SipLine *pLine = m_listList.getLine(lineUri);
   if (pLine)
   {
      pLine->setProxyServers(proxyServers);
      return TRUE;
   }

   return FALSE;
}

UtlBoolean SipLineMgr::getLineProxyServers(const Url& lineUri,
                                           UtlString& proxyServers) const
{
   OsLock lock(m_mutex); // scoped lock

   SipLine *pLine = m_listList.getLine(lineUri);
   if (pLine)
   {
      proxyServers = pLine->getProxyServers();
      return TRUE;
   }

   return FALSE;
}

UtlBoolean SipLineMgr::setStateForLine(const Url& lineUri,
                                       SipLine::LineStateEnum state)
{
   OsLock lock(m_mutex); // scoped lock

   SipLine *pLine = m_listList.getLine(lineUri);
   if (pLine)
   {
      pLine->setState(state);
      return TRUE;
   }

   return FALSE;
}

UtlBoolean SipLineMgr::getStateForLine(const Url& lineUri, SipLine::LineStateEnum& state)
{
  OsLock lock(m_mutex); // scoped lock

  SipLine *pLine = m_listList.getLine(lineUri);
  if (pLine)
  {
    state = pLine->getState();
    return TRUE;
  }

  return FALSE;
}

void SipLineMgr::dumpLines()
{
   // external lock is assumed
   m_listList.dumpLines();
}

UtlBoolean SipLineMgr::addLineAlias(const Url& aliasUri, const Url& lineURI)
{
   OsLock lock(m_mutex); // scoped lock

   return m_listList.addAlias(aliasUri, lineURI);
}

UtlBoolean SipLineMgr::deleteLineAlias(const Url& aliasUri)
{
   OsLock lock(m_mutex); // scoped lock

   return m_listList.removeAlias(aliasUri);
}