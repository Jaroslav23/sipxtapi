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
#include <net/SipDialog.h>
#include <cp/msg/ScTimerMsg.h>

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

ScTimerMsg::ScTimerMsg(PayloadTypeEnum payloadType,
                       const UtlString& sCallId,
                       const UtlString& sLocalTag,
                       const UtlString& sRemoteTag,
                       UtlBoolean isFromLocal)
: CpTimerMsg(CpTimerMsg::CP_SIP_CONNECTION_TIMER)
, m_payloadType(payloadType)
, m_sCallId(sCallId)
, m_sLocalTag(sLocalTag)
, m_sRemoteTag(sRemoteTag)
, m_isFromLocal(isFromLocal)
{

}

ScTimerMsg::ScTimerMsg(const ScTimerMsg& rhs)
: CpTimerMsg(rhs)
, m_payloadType(rhs.m_payloadType)
, m_sCallId(rhs.m_sCallId)
, m_sLocalTag(rhs.m_sLocalTag)
, m_sRemoteTag(rhs.m_sRemoteTag)
, m_isFromLocal(rhs.m_isFromLocal)
{
}

OsMsg* ScTimerMsg::createCopy(void) const
{
   return new ScTimerMsg(*this);
}

ScTimerMsg::~ScTimerMsg()
{

}

/* ============================ MANIPULATORS ============================== */

ScTimerMsg& ScTimerMsg::operator=(const ScTimerMsg& rhs)
{
   if (this == &rhs)
   {
      return *this;
   }

   CpTimerMsg::operator=(rhs); // assign fields for parent class
   
   m_payloadType = rhs.m_payloadType;
   m_sCallId = rhs.m_sCallId;
   m_sLocalTag = rhs.m_sLocalTag;
   m_sRemoteTag = rhs.m_sRemoteTag;
   m_isFromLocal = rhs.m_isFromLocal;

   return *this;
}

/* ============================ ACCESSORS ================================= */

void ScTimerMsg::getSipDialog(SipDialog& sipDialog) const
{
   sipDialog = SipDialog(m_sCallId, m_sLocalTag, m_sRemoteTag, m_isFromLocal);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
