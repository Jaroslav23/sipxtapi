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
#include <cp/msg/ScCommandMsg.h>

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

ScCommandMsg::ScCommandMsg(SubTypesEnum subType, const SipDialog& sipDialog)
: OsMsg(CpMessageTypes::SC_COMMAND, (unsigned char)subType)
, m_sipDialog(sipDialog)
{

}

ScCommandMsg::ScCommandMsg(const ScCommandMsg& rMsg)
: OsMsg(rMsg)
, m_sipDialog(rMsg.m_sipDialog)
{

}

ScCommandMsg::~ScCommandMsg()
{

}

OsMsg* ScCommandMsg::createCopy(void) const
{
   return new ScCommandMsg((SubTypesEnum)getMsgSubType(), m_sipDialog);
}

/* ============================ MANIPULATORS ============================== */

ScCommandMsg& ScCommandMsg::operator=(const ScCommandMsg& rhs)
{
   if (this == &rhs)
   {
      return *this;
   }

   OsMsg::operator=(rhs); // assign fields for parent class

   m_sipDialog = rhs.m_sipDialog;

   return *this;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

