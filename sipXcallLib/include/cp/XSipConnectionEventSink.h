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

#ifndef XSipConnectionEventSink_h__
#define XSipConnectionEventSink_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <net/SipInfoStatusEventListener.h>
#include <net/SipSecurityEventListener.h>
#include <cp/CpDefs.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * XSipConnectionEventSink is interface providing functions to fire various events
 * related to SipConnection. Implementor is responsible for routing events to proper
 * listeners for next upper layer (sipXtapi).
 */
class XSipConnectionEventSink
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /* ============================ MANIPULATORS ============================== */

   /** Fires sipx media event to media listener */
   virtual void fireSipXMediaEvent(CP_MEDIA_EVENT event,
                                   CP_MEDIA_CAUSE cause,
                                   CP_MEDIA_TYPE  type,
                                   intptr_t pEventData1 = 0,
                                   intptr_t pEventData2 = 0) = 0;

   /** Fires sipx call event to call event listener */
   virtual void fireSipXCallEvent(CP_CALLSTATE_EVENT eventCode,
                                  CP_CALLSTATE_CAUSE causeCode,
                                  const UtlString& sOriginalSessionCallId = NULL,
                                  int sipResponseCode = 0,
                                  const UtlString& sResponseText = NULL,
                                  const UtlString& sReferredBy = NULL,
                                  const UtlString& sReferTo = NULL) = 0;

   /** Fire info status event */
   virtual void fireSipXInfoStatusEvent(CP_INFOSTATUS_EVENT event,
                                        SIPXTACK_MESSAGE_STATUS status,
                                        const UtlString& sResponseText,
                                        int responseCode = 0,
                                        void* pCookie = NULL) = 0;

   /** Fire info message event */
   virtual void fireSipXInfoEvent(const UtlString& sContentType,
                                  const char* pContent = NULL,
                                  size_t nContentLength = 0) = 0;

   /** Fire security event */
   virtual void fireSipXSecurityEvent(SIPXTACK_SECURITY_EVENT event,
                                      SIPXTACK_SECURITY_CAUSE cause,
                                      const UtlString& sSRTPkey,
                                      void* pCertificate,
                                      size_t nCertificateSize,
                                      const UtlString& sSubjAltName,
                                      const UtlString& sSessionCallId,
                                      const UtlString& sRemoteAddress) = 0;

   /** Fires sipx RTP redirect event to event listener */
   virtual void fireSipXRtpRedirectEvent(CP_RTP_REDIRECT_EVENT eventCode,
                                         CP_RTP_REDIRECT_CAUSE causeCode) = 0;

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // XSipConnectionEventSink_h__
