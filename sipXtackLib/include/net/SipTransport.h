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

#ifndef SipTransport_h__
#define SipTransport_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsSocket.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS

class Url;
class UtlString;

// STRUCTS
// TYPEDEFS

/**
 * SIP_TRANSPORT_TYPE defines various protocols use for signaling 
 * transport.
 * Keep in sync with SIPX_TRANSPORT_TYPE.
 */
typedef enum
{
   SIP_TRANSPORT_AUTO = -1, /**< Automatic transport (we don't care) */
   SIP_TRANSPORT_UDP = 0,  /**< Indicator for a UDP socket type. */
   SIP_TRANSPORT_TCP = 1,  /**< Indicator for a TCP socket type. */ 
   SIP_TRANSPORT_TLS = 2,  /**< Indicator for a TLS socket type. */
} SIP_TRANSPORT_TYPE;

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

class SipTransport
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /* ============================ MANIPULATORS ============================== */

   static SIP_TRANSPORT_TYPE getSipTransport(const Url& url);
   static SIP_TRANSPORT_TYPE getSipTransport(const UtlString& strUrl);
   static SIP_TRANSPORT_TYPE getSipTransport(const char* szUrl);
   static SIP_TRANSPORT_TYPE getSipTransport(OsSocket::IpProtocolSocketType protocolType);
   static OsSocket::IpProtocolSocketType getSipTransport(SIP_TRANSPORT_TYPE transport);

   /**
    * Returns string identifier for given transport. For automatic or unknown transport
    * empty string is returned.
    */
   static UtlString getSipTransportString(OsSocket::IpProtocolSocketType protocolType);
   static UtlString getSipTransportString(SIP_TRANSPORT_TYPE transport);

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // SipTransport_h__
