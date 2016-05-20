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
#include <cp/XSipConnectionContext.h>
#include <mi/CpMediaInterface.h>

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

XSipConnectionContext::XSipConnectionContext()
: m_memberMutex(OsRWMutex::Q_FIFO)
, m_mediaConnectionId(CpMediaInterface::INVALID_CONNECTION_ID)
, m_mediaEventConnectionId(CpMediaInterface::INVALID_CONNECTION_ID)
, m_bSupressCallEvents(FALSE)
{

}

XSipConnectionContext::~XSipConnectionContext()
{

}

/* ============================ MANIPULATORS ============================== */

OsStatus XSipConnectionContext::acquireRead()
{
   return m_memberMutex.acquireRead();
}

OsStatus XSipConnectionContext::acquireWrite()
{
   return m_memberMutex.acquireWrite();
}

OsStatus XSipConnectionContext::tryAcquireRead()
{
   return m_memberMutex.tryAcquireRead();
}

OsStatus XSipConnectionContext::tryAcquireWrite()
{
   return m_memberMutex.tryAcquireWrite();
}

OsStatus XSipConnectionContext::releaseRead()
{
   return m_memberMutex.releaseRead();
}

OsStatus XSipConnectionContext::releaseWrite()
{
   return m_memberMutex.releaseWrite();
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
