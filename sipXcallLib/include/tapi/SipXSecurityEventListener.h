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

#ifndef SipXSecurityEventListener_h__
#define SipXSecurityEventListener_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsSharedServerTask.h>
#include "net/SipSecurityEventListener.h"
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
 * Listener for Security events
 */
class SipXSecurityEventListener : public OsSharedServerTask, public SipSecurityEventListener
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */

   /* ============================ CREATORS ================================== */
public:

   SipXSecurityEventListener(SIPX_INST pInst);

   virtual ~SipXSecurityEventListener();

   /* ============================ MANIPULATORS ============================== */

   virtual void OnEncrypt(const SipSecurityEvent& event);

   virtual void OnDecrypt(const SipSecurityEvent& event);

   virtual void OnTLS(const SipSecurityEvent& event);

   virtual UtlBoolean handleMessage(OsMsg& rRawMsg);

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   SipXSecurityEventListener(const SipXSecurityEventListener& rhs);

   SipXSecurityEventListener& operator=(const SipXSecurityEventListener& rhs);

   void handleSecurityEvent(const UtlString& sSRTPkey,
                            void* pCertificate,
                            size_t nCertificateSize,
                            SIPX_SECURITY_EVENT event,
                            SIPX_SECURITY_CAUSE cause,
                            const UtlString& sSubjAltName,
                            const UtlString& sSessionCallId,
                            const UtlString& sRemoteAddress);

   SIPX_INST m_pInst;
};

#endif // SipXSecurityEventListener_h__
