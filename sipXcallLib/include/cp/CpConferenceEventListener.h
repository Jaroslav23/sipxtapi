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

#ifndef CpConferenceEventListener_h__
#define CpConferenceEventListener_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <net/SipDialog.h>
#include <cp/CpDefs.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
// STRUCTS
// TYPEDEFS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

class CpConferenceEvent
{
public:
   UtlString m_sConferenceId; // id of conference
   SipDialog* m_pSipDialog; // sip dialog of call if available
   CP_CONFERENCE_CAUSE m_cause;

   CpConferenceEvent(CP_CONFERENCE_CAUSE cause,
                     const UtlString& sConferenceId,
	                  const SipDialog* pSipDialog = NULL)
      : m_cause(cause)
      , m_sConferenceId(sConferenceId)
      , m_pSipDialog(NULL)
   {
      if (pSipDialog)
      {
         m_pSipDialog = new SipDialog(*pSipDialog);
      }
   }

   CpConferenceEvent()
      : m_cause(CP_CONFERENCE_CAUSE_NORMAL)
      , m_pSipDialog(NULL)
   {
   }

   ~CpConferenceEvent()
   {
      delete m_pSipDialog;
      m_pSipDialog = NULL;
   }

   CpConferenceEvent(const CpConferenceEvent& event)
      : m_pSipDialog(NULL)
   {
      *this = event;
   }

   CpConferenceEvent& operator=(const CpConferenceEvent& event)
   {
      if (&event == this)
      {
         return *this;
      }

	   m_sConferenceId = event.m_sConferenceId;
      // get rid of old sip dialog if it exists
      if (m_pSipDialog)
      {
         delete m_pSipDialog;
         m_pSipDialog = NULL;
      }
      if (event.m_pSipDialog)
      {
         // copy new sip dialog
         m_pSipDialog = new SipDialog(*(event.m_pSipDialog));
      }
      m_cause = event.m_cause;
      return *this;
   }
};


/**
* Listener for Conference events
*/
class CpConferenceEventListener
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */

   /* ============================ CREATORS ================================== */
public:
   CpConferenceEventListener() {}
   virtual ~CpConferenceEventListener() {}

   /* ============================ MANIPULATORS ============================== */

   virtual void OnConferenceCreated(const CpConferenceEvent& event) = 0;

   virtual void OnConferenceDestroyed(const CpConferenceEvent& event) = 0;

   virtual void OnConferenceCallAdded(const CpConferenceEvent& event) = 0;

   virtual void OnConferenceCallAddFailure(const CpConferenceEvent& event) = 0;

   virtual void OnConferenceCallRemoved(const CpConferenceEvent& event) = 0;

   virtual void OnConferenceCallRemoveFailure(const CpConferenceEvent& event) = 0;

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // CpConferenceEventListener_h__
