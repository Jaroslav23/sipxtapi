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

#ifndef SipInfoEventListener_h__
#define SipInfoEventListener_h__

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
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

class SipInfoEvent
{
public:
   UtlString m_sCallId;
   UtlString m_sContentType;
   const char* m_pContent;
   size_t m_nContentLength;

   SipInfoEvent()
      : m_pContent(NULL)
      , m_nContentLength(0)
   {

   }

   SipInfoEvent(const UtlString& sCallId,
                const UtlString& sContentType = NULL,
                const char* pContent = NULL,
                size_t nContentLength = 0)
      : m_sCallId(sCallId)
      , m_sContentType(sContentType)
      , m_pContent(NULL)
      , m_nContentLength(nContentLength)
   {
      if (nContentLength > 0)
      {
         m_pContent = (char*)malloc(nContentLength);
         memcpy((void*)m_pContent, (void*)pContent, nContentLength);
      }
   }

   ~SipInfoEvent()
   {
      if (m_pContent)
      {
         free((void*)m_pContent);
         m_pContent = NULL;
      }
   }

   SipInfoEvent(const SipInfoEvent& event)
      : m_pContent(NULL)
      , m_nContentLength(0)
   {
      *this = event;
   }

   SipInfoEvent& operator=(const SipInfoEvent& event)
   {
      if (&event == this)
      {
         return *this;
      }

      m_sCallId = event.m_sCallId;
      m_sContentType = event.m_sContentType;

      if (event.m_pContent && event.m_nContentLength > 0)
      {
         if (m_pContent)
         {
            free((void*)m_pContent);
         }
         m_pContent = (char*)malloc(event.m_nContentLength);
         memcpy((void*)m_pContent, (void*)event.m_pContent, event.m_nContentLength);
      }
      else
      {
         m_pContent = NULL;
      }
      m_nContentLength = event.m_nContentLength;

      return *this;
   }
};


/**
* Listener for Info message events
*/
class SipInfoEventListener
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */

   /* ============================ CREATORS ================================== */
public:
   SipInfoEventListener() {}
   virtual ~SipInfoEventListener() {}

   /* ============================ MANIPULATORS ============================== */

   virtual void OnInfoMessage(const SipInfoEvent& event) = 0;

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // SipInfoEventListener_h__
