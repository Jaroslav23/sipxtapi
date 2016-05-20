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
#include <os/OsSysLog.h>
#include <utl/UtlHashMapIterator.h>
#include "net/SipLineList.h"
#include <net/SipLineAlias.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

SipLineList::SipLineList()
: m_lineMap()
, m_lineAliasMap()
{

}

SipLineList::~SipLineList()
{    
   m_lineMap.destroyAll();
   m_lineAliasMap.destroyAll();
}

UtlBoolean SipLineList::add(SipLine *pLine)
{
   if (pLine && !lineExists(*pLine))
   {
      // if it doesn't already exist, add to bag
      m_lineMap.insertKeyAndValue(pLine->getLineUri().toString().clone(), pLine);
      return TRUE;
   }

   return FALSE;
}

UtlBoolean SipLineList::add(const SipLine& line)
{
   SipLine* pLine = new SipLine(line);

   if (add(pLine))
   {
      // addition successful
      return TRUE;
   }
   else
   {
      // delete copy
      delete pLine;
      pLine = NULL;
      return FALSE;
   }
}

UtlBoolean SipLineList::addAlias(const Url& aliasUri, const Url& lineUri)
{
   UtlBoolean result = FALSE;

   if (m_lineMap.contains(&lineUri.toString()) && !m_lineAliasMap.contains(&aliasUri.toString()))
   {
      // line exists and alias doesn't, add alias
      m_lineAliasMap.insertKeyAndValue(aliasUri.toString().clone(),
         new SipLineAlias(aliasUri, lineUri));
      result = TRUE;
   }

   return result;
}

UtlBoolean SipLineList::remove(const SipLine& line)
{
   removeAliasesForLine(line.getLineUri());
   return m_lineMap.destroy(&line.getLineUri().toString());
}

UtlBoolean SipLineList::remove(const Url& lineIdentityUri)
{
   removeAliasesForLine(lineIdentityUri);
   return m_lineMap.destroy(&lineIdentityUri.toString());
}

UtlBoolean SipLineList::removeAlias(const Url& aliasUri)
{
   return m_lineAliasMap.destroy(&aliasUri.toString());
}

void SipLineList::removeAll()
{
   m_lineMap.destroyAll();
   m_lineAliasMap.destroyAll();
}

UtlBoolean SipLineList::lineExists(const SipLine& line, UtlBoolean bConsiderAliases) const
{
   UtlBoolean result = m_lineMap.contains(&line.getLineUri().toString());
   if (!result && bConsiderAliases)
   {
      // not found, try aliases
      result = m_lineAliasMap.contains(&line.getLineUri().toString());
   }

   return result;
}

UtlBoolean SipLineList::lineExists(const Url& lineUri, UtlBoolean bConsiderAliases) const
{
   UtlBoolean result = m_lineMap.contains(&lineUri.toString());

   if (!result && bConsiderAliases)
   {
      // not found, try aliases
      result = m_lineAliasMap.contains(&lineUri.toString());
   }

   return result;
}

SipLine* SipLineList::getLine(const Url& lineUri, UtlBoolean bConsiderAliases) const
{
   SipLine *pLine = dynamic_cast<SipLine*>(m_lineMap.findValue(&lineUri.toString()));

   if (!pLine && bConsiderAliases)
   {
      SipLineAlias *pLineAlias = dynamic_cast<SipLineAlias*>(m_lineAliasMap.findValue(&lineUri.toString()));
      if (pLineAlias)
      {
         Url originalLineUri = pLineAlias->getOriginalLineUri();
         // repeat lookup with alias original line uri
         pLine = dynamic_cast<SipLine*>(m_lineMap.findValue(&originalLineUri.toString()));
      }
   }

   return pLine;
}

SipLine* SipLineList::getLineByUserId(const UtlString& userId) const
{
   SipLine* pLine = NULL;

   if (!userId.isNull())
   {
      UtlHashMapIterator itor(m_lineMap);
      UtlContainable* pKey = NULL;

      while ((pKey = itor()) != NULL)
      {
         pLine = dynamic_cast<SipLine*>(itor.value());
         if (pLine && !pLine->getUserId().compareTo(userId))
         {
            return pLine;
         }
      }
   }

   return NULL;
}

size_t SipLineList::getLinesCount() const
{
   return m_lineMap.entries();
}

size_t SipLineList::getLineAliasesCount() const
{
   return m_lineAliasMap.entries();
}

//
// Priorities:
//   1. Matches first lineUri
//   2. Matches line alias
//   3. Matches first userId
//
SipLine* SipLineList::findLine(const Url& lineUri,
                               const UtlString& userId,
                               UtlBoolean bConsiderAliases) const
{
   // firt lookup by lineUri and/or alias
   SipLine* pLine = getLine(lineUri, bConsiderAliases);
   if (pLine)
   {
      return pLine;
   }
   else
   {
      // not found, try userId
      if (!userId.isNull())
      {
         // only search by userId if its not empty
         UtlHashMapIterator itor(m_lineMap);
         UtlContainable* pKey = NULL;

         while ((pKey = itor()) != NULL)
         {
            pLine = dynamic_cast<SipLine*>(itor.value());
            if (pLine)
            {
               if (pLine->getUserId().compareTo(userId, UtlString::ignoreCase) == 0)
               {
                  return pLine;
               }
            }
         }
      }
   }

   // is NULL if nothing was found
   return NULL;
}

void SipLineList::dumpLines()
{
   SipLine* pLine = NULL;

   UtlHashMapIterator itor(m_lineMap);
   UtlContainable* pKey = NULL;
   int i = 0;

   while ((pKey = itor()) != NULL)
   {
      pLine = dynamic_cast<SipLine*>(itor.value());
      if (pLine)
      {
         OsSysLog::add(FAC_LINE_MGR, PRI_DEBUG, "LineList %x, Line [%d]: lineURI=%s, lineState=%d",
            this, i++, pLine->getLineUri().toString().data(), (int)pLine->getState());
      }
   }
}

void SipLineList::dumpLineAliases()
{
   SipLineAlias* pLineAlias = NULL;

   UtlHashMapIterator itor(m_lineAliasMap);
   UtlContainable* pKey = NULL;
   int i = 0;

   while ((pKey = itor()) != NULL)
   {
      pLineAlias = dynamic_cast<SipLineAlias*>(itor.value());
      if (pLineAlias)
      {
         OsSysLog::add(FAC_LINE_MGR, PRI_DEBUG, "LineList %x, Line alias [%d]: aliasURI=%s, lineURI=%s",
            this, i++, pLineAlias->getAliasUri().toString().data(), pLineAlias->getOriginalLineUri().toString().data());
      }
   }
}

void SipLineList::getLineCopies(UtlSList& lineList) const
{
   SipLine* pLine = NULL;

   UtlHashMapIterator itor(m_lineMap);
   UtlContainable* pKey = NULL;
   int i = 0;

   while ((pKey = itor()) != NULL)
   {
      pLine = dynamic_cast<SipLine*>(itor.value());
      if (pLine)
      {
         // copy line into list
         lineList.append(new SipLine(*pLine));
      }
   }
}

void SipLineList::getLineUris(UtlSList& lineUris) const
{
   SipLine* pLine = NULL;

   UtlHashMapIterator itor(m_lineMap);
   UtlContainable* pKey = NULL;
   int i = 0;

   while ((pKey = itor()) != NULL)
   {
      pLine = dynamic_cast<SipLine*>(itor.value());
      if (pLine)
      {
         // copy line uri into list
         lineUris.append(pLine->getLineUri().toString().clone());
      }
   }
}

void SipLineList::removeAliasesForLine(const Url& lineURI)
{
   SipLineAlias* pLineAlias = NULL;

   UtlHashMapIterator itor(m_lineAliasMap);
   UtlContainable* pKey = NULL;
   int i = 0;

   while ((pKey = itor()) != NULL)
   {
      pLineAlias = dynamic_cast<SipLineAlias*>(itor.value());
      if (pLineAlias)
      {
         if (SipLine::areLineUrisEqual(lineURI, pLineAlias->getOriginalLineUri()))
         {
            m_lineAliasMap.destroy(itor.key());
         }
      }
   }
}
