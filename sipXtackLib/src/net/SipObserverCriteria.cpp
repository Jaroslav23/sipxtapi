//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <net/SipObserverCriteria.h>
#include <net/SipDialog.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipObserverCriteria::SipObserverCriteria(void* observerData,
                                         OsMsgQ* messageQueue,
                                          const char* sipMethod,
                                          UtlBoolean wantRequests,
                                          UtlBoolean wantResponses,
                                          UtlBoolean wantIncoming,
                                          UtlBoolean wantOutGoing,
                                          const char* eventName,
                                          const SipDialog* pSipDialog
                                          ) :
UtlString(sipMethod ? sipMethod : "")
{
   mObserverData = observerData;
   mpMessageObserverQueue = messageQueue;
   mWantsRequests = wantRequests;
   mWantsResponses = wantResponses;
   mWantsIncoming = wantIncoming;
   mWantsOutGoing = wantOutGoing;
   mEventName = eventName ? eventName : "";

   // Make a copy of the sip dialog
   if (pSipDialog != NULL)
      m_pSipDialog = new SipDialog(*pSipDialog);
   else
      m_pSipDialog = NULL ;
}

// Copy constructor
SipObserverCriteria::SipObserverCriteria(const SipObserverCriteria& rSipObserverCriteria)
{
}


// Destructor
SipObserverCriteria::~SipObserverCriteria()
{
   if (m_pSipDialog != NULL)
   {
      delete m_pSipDialog ;
      m_pSipDialog = NULL ;
   }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipObserverCriteria& 
SipObserverCriteria::operator=(const SipObserverCriteria& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */
OsMsgQ* SipObserverCriteria::getObserverQueue()
{
    return(mpMessageObserverQueue);
}

void* SipObserverCriteria::getObserverData()
{
    return(mObserverData);
}

void SipObserverCriteria::getEventName(UtlString& eventName) const
{
    eventName = mEventName;
}

const SipDialog* SipObserverCriteria::getSipDialog() const
{
    return (m_pSipDialog);
}

/* ============================ INQUIRY =================================== */

UtlBoolean SipObserverCriteria::wantsRequests() const
{
    return(mWantsRequests);
}

UtlBoolean SipObserverCriteria::wantsResponses() const
{
    return(mWantsResponses);
}

UtlBoolean SipObserverCriteria::wantsIncoming() const
{
    return(mWantsIncoming);
}

UtlBoolean SipObserverCriteria::wantsOutGoing() const
{
    return(mWantsOutGoing);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

