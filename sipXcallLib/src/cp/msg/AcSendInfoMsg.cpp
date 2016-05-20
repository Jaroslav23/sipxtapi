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
#include <cp/msg/AcSendInfoMsg.h>

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

AcSendInfoMsg::AcSendInfoMsg(const SipDialog& sipDialog,
                             const UtlString& sContentType,
                             const char* pContent,
                             const size_t nContentLength,
                             void* pCookie)
: AcCommandMsg(AC_SEND_INFO)
, m_sipDialog(sipDialog)
, m_sContentType(sContentType)
, m_pContent(NULL)
, m_nContentLength(nContentLength)
, m_pCookie(pCookie)
{
   if (m_nContentLength > 0 && pContent)
   {
      m_pContent = (char*)malloc(sizeof(char) * m_nContentLength);
      if (m_pContent)
      {
         memcpy(m_pContent, pContent, m_nContentLength);
      }
   }
}

AcSendInfoMsg::~AcSendInfoMsg()
{
   if (m_pContent)
   {
      free((void*)m_pContent);
      m_pContent = NULL;
   }
   m_pCookie = NULL;
}

OsMsg* AcSendInfoMsg::createCopy(void) const
{
   return new AcSendInfoMsg(m_sipDialog, m_sContentType, m_pContent, m_nContentLength, m_pCookie);
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

