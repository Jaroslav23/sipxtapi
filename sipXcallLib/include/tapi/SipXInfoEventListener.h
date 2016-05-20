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

#ifndef SipXInfoEventListener_h__
#define SipXInfoEventListener_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsSharedServerTask.h>
#include <net/SipInfoEventListener.h>
#include "tapi/sipXtapi.h"
#include <tapi/sipXtapiEvents.h>

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
* Listener for Info message events
*/
class SipXInfoEventListener : public OsSharedServerTask, public SipInfoEventListener
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */

   /* ============================ CREATORS ================================== */
public:
   SipXInfoEventListener(SIPX_INST pInst);
   virtual ~SipXInfoEventListener();

   /* ============================ MANIPULATORS ============================== */

   virtual void OnInfoMessage(const SipInfoEvent& event);

   virtual UtlBoolean handleMessage(OsMsg& rRawMsg);

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   SipXInfoEventListener(const SipXInfoEventListener& rhs);

   SipXInfoEventListener& operator=(const SipXInfoEventListener& rhs);
  
   void handleInfoEvent(const UtlString& sAbstractCallId,
                        const UtlString& sContentType,
                        const char* pContent,
                        size_t nContentLength);

   SIPX_INST m_pInst;
};

#endif // SipXInfoEventListener_h__
