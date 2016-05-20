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

#ifndef SipXConferenceEventListener_h__
#define SipXConferenceEventListener_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsSharedServerTask.h>
#include <cp/CpConferenceEventListener.h>
#include "tapi/sipXtapi.h"
#include <tapi/sipXtapiEvents.h>
#include <tapi/SipXCall.h>

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

/**
* Listener for conference events
*/
class SipXConferenceEventListener : public OsSharedServerTask, public CpConferenceEventListener
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */

   /* ============================ CREATORS ================================== */
public:
   SipXConferenceEventListener(SIPX_INST pInst);
   virtual ~SipXConferenceEventListener();

   /* ============================ MANIPULATORS ============================== */

   virtual void OnConferenceCreated(const CpConferenceEvent& event);

   virtual void OnConferenceDestroyed(const CpConferenceEvent& event);

   virtual void OnConferenceCallAdded(const CpConferenceEvent& event);

   virtual void OnConferenceCallAddFailure(const CpConferenceEvent& event);

   virtual void OnConferenceCallRemoved(const CpConferenceEvent& event);

   virtual void OnConferenceCallRemoveFailure(const CpConferenceEvent& event);

   virtual UtlBoolean handleMessage(OsMsg& rRawMsg);

   /** Fire even manually. Avoid using if possible. Let events work automatically. */
   void sipxFireConferenceEvent(SIPX_CONFERENCE_EVENT event,
	                             SIPX_CONFERENCE_CAUSE cause,
	                             const UtlString& sConferenceId,
							           const SipDialog* pSipDialog = NULL);

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   SipXConferenceEventListener(const SipXConferenceEventListener& rhs);

   SipXConferenceEventListener& operator=(const SipXConferenceEventListener& rhs);

   void handleConferenceEvent(SIPX_CONFERENCE_EVENT event,
                              SIPX_CONFERENCE_CAUSE cause,
                              const UtlString& sConferenceId,
                              const SipDialog* pSipDialog = NULL);

   SIPX_INST m_pInst;
};

#endif // SipXConferenceEventListener_h__
