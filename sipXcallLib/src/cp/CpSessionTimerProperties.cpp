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
#include <os/OsDateTime.h>
#include <cp/CpSessionTimerProperties.h>

// DEFINES
#define MIN_SESSION_EXPIRES 90
#define INITIAL_SESSION_EXPIRES 1800
#define REFRESHER_UAS "uas"
#define REFRESHER_UAC "uac"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

CpSessionTimerProperties::CpSessionTimerProperties()
: m_sessionExpires(INITIAL_SESSION_EXPIRES)
, m_initialSessionExpires(INITIAL_SESSION_EXPIRES)
, m_minSessionExpires(MIN_SESSION_EXPIRES)
, m_lastRefreshTimestamp(0)
{

}

CpSessionTimerProperties::~CpSessionTimerProperties()
{

}

/* ============================ MANIPULATORS ============================== */

void CpSessionTimerProperties::reset(UtlBoolean bFullReset)
{
   if (bFullReset)
   {
      m_sessionExpires = m_initialSessionExpires;
      m_minSessionExpires = MIN_SESSION_EXPIRES;
   }


   m_sRefresher = m_sInitialRefresher;
}

void CpSessionTimerProperties::onSessionRefreshed()
{
   m_lastRefreshTimestamp = OsDateTime::getSecsSinceEpoch();
}

/* ============================ ACCESSORS ================================= */

void CpSessionTimerProperties::configureRefresher(const UtlString& refresher, UtlBoolean bIsOutboundTransaction)
{
   if (bIsOutboundTransaction)
   {
      if (refresher.compareTo(REFRESHER_UAC) == 0)
      {
         m_sRefresher = CP_SESSION_REFRESH_LOCAL;
      }
      else if (refresher.compareTo(REFRESHER_UAS) == 0)
      {
         m_sRefresher = CP_SESSION_REFRESH_REMOTE;
      }
      else
      {
         // we may choose refresher
         m_sRefresher = (m_sInitialRefresher != CP_SESSION_REFRESH_AUTO ? m_sInitialRefresher : CP_SESSION_REFRESH_LOCAL);
      }
   }
   else
   {
      if (refresher.compareTo(REFRESHER_UAC) == 0)
      {
         m_sRefresher = CP_SESSION_REFRESH_REMOTE;
      }
      else if (refresher.compareTo(REFRESHER_UAS) == 0)
      {
         m_sRefresher = CP_SESSION_REFRESH_LOCAL;
      }
      else
      {
         // we may choose refresher
         m_sRefresher = (m_sInitialRefresher != CP_SESSION_REFRESH_AUTO ? m_sInitialRefresher : CP_SESSION_REFRESH_LOCAL);
      }
   }
}

UtlString CpSessionTimerProperties::getRefresher(UtlBoolean bIsOutboundTransaction) const
{
   if (bIsOutboundTransaction)
   {
      switch (m_sRefresher)
      {
      case CP_SESSION_REFRESH_LOCAL:
         return REFRESHER_UAC;
      case CP_SESSION_REFRESH_REMOTE:
         return REFRESHER_UAS;
      default:
         return NULL;
      }
   }
   else
   {
      switch (m_sRefresher)
      {
      case CP_SESSION_REFRESH_LOCAL:
         return REFRESHER_UAS;
      case CP_SESSION_REFRESH_REMOTE:
         return REFRESHER_UAC;
      default:
         return NULL;
      }
   }
}

void CpSessionTimerProperties::setMinSessionExpires(int val)
{
   // minSe can only be increased
   if (val > m_minSessionExpires &&
       val > MIN_SESSION_EXPIRES)
   {
      m_minSessionExpires = val;
   }

   // setting minSe also has an effect on session expires
   setSessionExpires(m_minSessionExpires);
}

void CpSessionTimerProperties::setSessionExpires(int sessionExpires)
{
   // we may not set lower than minSe
   if (sessionExpires >= m_sessionExpires &&
       sessionExpires >= m_minSessionExpires)
   {
      m_sessionExpires = sessionExpires;
   }
}

/* ============================ INQUIRY =================================== */

UtlBoolean CpSessionTimerProperties::isSessionStale() const
{
   long timeNow = OsDateTime::getSecsSinceEpoch();

   if (timeNow - m_lastRefreshTimestamp > m_sessionExpires)
   {
      return TRUE;
   }

   return FALSE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
