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

#ifndef SipInfoStatusEventListener_h__
#define SipInfoStatusEventListener_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
// STRUCTS
// TYPEDEFS

typedef enum 
{
    SIPXTACK_MESSAGE_OK = 0,
    SIPXTACK_MESSAGE_FAILURE,
    SIPXTACK_MESSAGE_SERVER_FAILURE,
    SIPXTACK_MESSAGE_GLOBAL_FAILURE,
} SIPXTACK_MESSAGE_STATUS;

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

class SipInfoStatusEvent
{
public:
   UtlString m_sCallId;
   SIPXTACK_MESSAGE_STATUS m_status;
   int m_iResponseCode;
   UtlString m_sResponseText;
   void* m_pCookie;

   SipInfoStatusEvent() : m_status(SIPXTACK_MESSAGE_OK)
      , m_iResponseCode(0)
      , m_pCookie(NULL)
   {

   }

   SipInfoStatusEvent(const UtlString& sCallId,
                      SIPXTACK_MESSAGE_STATUS status,
                      int iResponseCode,
                      const UtlString& sResponseText,
                      void* pCookie = NULL)
      : m_sCallId(sCallId)
      , m_status(status)
      , m_iResponseCode(iResponseCode)
      , m_sResponseText(sResponseText)
      , m_pCookie(pCookie)
   {

   }

   ~SipInfoStatusEvent()
   {
      m_pCookie = NULL;
   }

   SipInfoStatusEvent(const SipInfoStatusEvent& event)
   {
      *this = event;
   }

   SipInfoStatusEvent& operator=(const SipInfoStatusEvent& event)
   {
      if (&event == this)
      {
         return *this;
      }

      m_sCallId = event.m_sCallId;
      m_status = event.m_status;
      m_iResponseCode = event.m_iResponseCode;
      m_sResponseText = event.m_sResponseText;
      m_pCookie = event.m_pCookie;

      return *this;
   }
};


/**
* Listener for Info events
*/
class SipInfoStatusEventListener
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */

   /* ============================ CREATORS ================================== */
public:
   SipInfoStatusEventListener() {}
   virtual ~SipInfoStatusEventListener() {}

   /* ============================ MANIPULATORS ============================== */

   virtual void OnResponse(const SipInfoStatusEvent& event) = 0;

   virtual void OnNetworkError(const SipInfoStatusEvent& event) = 0;

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // SipInfoStatusEventListener_h__
