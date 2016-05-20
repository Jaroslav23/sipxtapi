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

#ifndef SipSecurityEventListener_h__
#define SipSecurityEventListener_h__

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
   SIPXTACK_SECURITY_UNKNOWN = 0,
   SIPXTACK_SECURITY_ENCRYPT = 1000,
   SIPXTACK_SECURITY_DECRYPT = 2000,
   SIPXTACK_SECURITY_TLS = 4000,
} SIPXTACK_SECURITY_EVENT;

typedef enum
{
   SIPXTACK_SECURITY_CAUSE_UNKNOWN = 0,
   SIPXTACK_SECURITY_CAUSE_NORMAL,
   SIPXTACK_SECURITY_CAUSE_ENCRYPT_SUCCESS,
   SIPXTACK_SECURITY_CAUSE_ENCRYPT_FAILURE_LIB_INIT,
   SIPXTACK_SECURITY_CAUSE_ENCRYPT_FAILURE_BAD_PUBLIC_KEY,
   SIPXTACK_SECURITY_CAUSE_ENCRYPT_FAILURE_INVALID_PARAMETER,
   SIPXTACK_SECURITY_CAUSE_DECRYPT_SUCCESS,
   SIPXTACK_SECURITY_CAUSE_DECRYPT_FAILURE_DB_INIT,
   SIPXTACK_SECURITY_CAUSE_DECRYPT_FAILURE_BAD_DB_PASSWORD,
   SIPXTACK_SECURITY_CAUSE_DECRYPT_FAILURE_INVALID_PARAMETER,
   SIPXTACK_SECURITY_CAUSE_DECRYPT_BAD_SIGNATURE,
   SIPXTACK_SECURITY_CAUSE_DECRYPT_MISSING_SIGNATURE,
   SIPXTACK_SECURITY_CAUSE_DECRYPT_SIGNATURE_REJECTED,
   SIPXTACK_SECURITY_CAUSE_TLS_SERVER_CERTIFICATE,
   SIPXTACK_SECURITY_CAUSE_TLS_BAD_PASSWORD,
   SIPXTACK_SECURITY_CAUSE_TLS_LIBRARY_FAILURE,
   SIPXTACK_SECURITY_CAUSE_REMOTE_HOST_UNREACHABLE,
   SIPXTACK_SECURITY_CAUSE_TLS_CONNECTION_FAILURE,
   SIPXTACK_SECURITY_CAUSE_TLS_HANDSHAKE_FAILURE,
   SIPXTACK_SECURITY_CAUSE_SIGNATURE_NOTIFY,
   SIPXTACK_SECURITY_CAUSE_TLS_CERTIFICATE_REJECTED
} SIPXTACK_SECURITY_CAUSE;

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

class SipSecurityEvent
{
public:
   UtlString           m_sSRTPkey;
   void*               m_pCertificate;
   size_t              m_nCertificateSize;
   SIPXTACK_SECURITY_EVENT m_event;
   SIPXTACK_SECURITY_CAUSE m_cause;
   UtlString           m_sSubjAltName;
   UtlString           m_SessionCallId;
   UtlString           m_sRemoteAddress;

   SipSecurityEvent() : m_sSRTPkey()
      , m_pCertificate(NULL)
      , m_nCertificateSize(0)
      , m_event(SIPXTACK_SECURITY_UNKNOWN)
      , m_cause(SIPXTACK_SECURITY_CAUSE_UNKNOWN)
      , m_sSubjAltName()
      , m_SessionCallId()
      , m_sRemoteAddress()
   {

   }

   ~SipSecurityEvent()
   {
      if (m_pCertificate)
      {
         delete m_pCertificate;
         m_pCertificate = NULL;
      }
   }

   SipSecurityEvent(const SipSecurityEvent& event)
   {
      *this = event;
   }

   SipSecurityEvent& operator=(const SipSecurityEvent& event)
   {
      if (&event == this)
      {
         return *this;
      }

      m_sSRTPkey = event.m_sSRTPkey;
      if (m_pCertificate)
      {
         delete m_pCertificate;
         m_pCertificate = NULL;
      }
      if (event.m_nCertificateSize > 0)
      {
         m_pCertificate = malloc(event.m_nCertificateSize);
         memcpy(m_pCertificate, event.m_pCertificate, event.m_nCertificateSize);
      }
      m_nCertificateSize = event.m_nCertificateSize;
      m_event = event.m_event;
      m_cause = event.m_cause;
      m_sSubjAltName = event.m_sSubjAltName;
      m_SessionCallId = event.m_SessionCallId;
      m_sRemoteAddress = event.m_sRemoteAddress;

      return *this;
   }
};


/**
* Listener for Security events
*/
class SipSecurityEventListener
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */

   /* ============================ CREATORS ================================== */
public:
   SipSecurityEventListener() {}
   virtual ~SipSecurityEventListener() {}

   /* ============================ MANIPULATORS ============================== */

   virtual void OnEncrypt(const SipSecurityEvent& event) = 0;

   virtual void OnDecrypt(const SipSecurityEvent& event) = 0;

   virtual void OnTLS(const SipSecurityEvent& event) = 0;

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


};

#endif // SipSecurityEventListener_h__
