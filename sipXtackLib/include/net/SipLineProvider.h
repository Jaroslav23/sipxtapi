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

#ifndef SipLineProvider_h__
#define SipLineProvider_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlDefs.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS
class SipMessage;
class Url;
class UtlString;
class SipLine;
class SipLineCredential;

/**
 * Sip line provider is responsible for line operations directly related to SipMessage
 * content.
 */
class SipLineProvider
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   /**
    * Gets SipLineCredential object for line from sipRequest. sipResponse is used
    * to get the correct realm and scheme.
    * SipLineCredential returned will be for correct userid, realm and scheme.
    *
    * @param sipResponse Message sent by remote server to us with authentication request
    * @param sipRequest Original request which needs to be authenticated
    * @param lineCredentials Returned line credentials if found
    */
   UtlBoolean getCredentialForMessage(const SipMessage& sipResponse,
                                      const SipMessage& sipRequest,
                                      SipLineCredential& lineCredential) const;

   /**
    * Gets proxy server that should be used for given message. Proxy server
    * is selected from line, which is being used to send the message.
    */
   UtlBoolean getProxyServersForMessage(const SipMessage& sipRequest,
                                        UtlString& proxyServer) const;

   /** Gets line proxy server. Returns TRUE if line was found. Proxy server might be empty. */
   virtual UtlBoolean getLineProxyServers(const Url& lineUri, UtlString& proxyServers) const = 0;

   /**
   * Tries to find line according to given parameters. First hash lookup by lineUri.
   * If not found by identityUri, try slow scan by userId. This method is slow if userId
   * is provided and lineUri doesn't match.
   *
   * If found, then line is copied into line parameter. It is slower than getLineCopy.
   * Line aliases are considered.
   */
   virtual UtlBoolean findLineCopy(const Url& lineUri,
                                   const UtlString& userId,
                                   SipLine& line) const = 0;

   /**
   * Gets a copy of given line including credentials. Returns FALSE if line was not found.
   * This method uses fast lookup by hashcode. Line aliases are considered.
   */
   virtual UtlBoolean getLineCopy(const Url& lineUri, SipLine& sipLine) const = 0;

   /**
   * Tries to extract line data from given sip message. Doesn't check if line exists.
   * Inbound/outbound messages are detected automatically through SipMessage flag.
   */
   static void extractLineData(const SipMessage& sipMsg,
                               Url& lineUri,
                               UtlString& userId);

   /** 
   * Discovers real line identity from sip request. Returns full line url. This is
   * needed for inbound calls, since request uri can be different than to url after redirect,
   * and may be slightly different than identity of locally defined line (for example by display name).
   */
   void getFullLineUrl(const SipMessage& sipRequest, UtlString& sFullLineUrl) const;

   /* ============================ INQUIRY =================================== */

   /**
   * Tries to find line according to given parameters. First lookup by identityUri. If
   * not found by identityUri, try by userId.
   */
   virtual UtlBoolean lineExists(const Url& lineUri,
                                 const UtlString& userId) const = 0;

   /**
    * Checks if line from SipMessage exists.
    */
   UtlBoolean lineExists(const SipMessage& sipMsg) const;

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // SipLineProvider_h__
