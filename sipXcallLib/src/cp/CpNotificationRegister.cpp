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
#include <utl/UtlInt.h>
#include <utl/UtlPtr.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/UtlSList.h>
#include <utl/UtlSListIterator.h>
#include <net/SipDialog.h>
#include <cp/CpNotificationRegister.h>

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

CpNotificationRegister::CpNotificationRegister()
{

}

CpNotificationRegister::~CpNotificationRegister()
{
   m_register.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

OsStatus CpNotificationRegister::subscribe(CP_NOTIFICATION_TYPE notificationType,
                                           const SipDialog& callbackSipDialog)
{
   OsStatus result = OS_FAILED;

   UtlInt key((int)notificationType);
   UtlSList* pDialogList = dynamic_cast<UtlSList*>(m_register.findValue(&key));
   if (!pDialogList)
   {
      pDialogList = new UtlSList();
      m_register.insertKeyAndValue(key.clone(), pDialogList);
   }

   if (pDialogList)
   {
      pDialogList->append(new SipDialog(callbackSipDialog));
      result = OS_SUCCESS;
   }

   return result;
}

OsStatus CpNotificationRegister::unsubscribe(CP_NOTIFICATION_TYPE notificationType,
                                             const SipDialog& callbackSipDialog)
{
   OsStatus result = OS_FAILED;

   UtlInt key((int)notificationType);
   UtlSList* pDialogList = dynamic_cast<UtlSList*>(m_register.findValue(&key));
   if (!pDialogList)
   {
      return OS_SUCCESS;
   }

   if (pDialogList)
   {
      UtlSListIterator itor(*pDialogList);

      while (itor())
      {
         SipDialog* pSipDialog = dynamic_cast<SipDialog*>(itor.item());
         if (pSipDialog && pSipDialog->compareDialogs(callbackSipDialog) != SipDialog::DIALOG_MISMATCH)
         {
            pDialogList->destroy(pSipDialog);
         }
      }

      result = OS_SUCCESS;
   }

   return result;
}

const UtlSList* CpNotificationRegister::getSubscribedDialogs(CP_NOTIFICATION_TYPE notificationType) const
{
   UtlInt key((int)notificationType);
   UtlSList* pDialogList = dynamic_cast<UtlSList*>(m_register.findValue(&key));
   return pDialogList;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
