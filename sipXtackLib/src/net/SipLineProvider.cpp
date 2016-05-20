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

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <net/SipLine.h>
#include <net/SipLineProvider.h>
#include <net/SipLineCredential.h>
#include <net/SipMessage.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

UtlBoolean SipLineProvider::getCredentialForMessage(const SipMessage& sipResponse, // message with authentication request
                                                    const SipMessage& sipRequest, // original sip request
                                                    SipLineCredential& lineCredential) const
{
   UtlBoolean result = FALSE;
   Url lineUri;
   UtlString userId;

   // get lineUri, userId from SipMessage
   extractLineData(sipRequest, lineUri, userId);

   // get copy of SipLine
   SipLine sipLine;
   UtlBoolean lineFound = findLineCopy(lineUri, userId, sipLine); // slow lookup during authentication is ok
   if (lineFound)
   {
      // get authentication details from response
      HttpMessage::HttpEndpointEnum authorizationEntity = 
         HttpMessage::getAuthorizationEntity(sipResponse.getResponseStatusCode());
      UtlString nonce;
      UtlString opaque;
      UtlString realm;
      UtlString scheme;
      UtlString algorithm;
      UtlString qop;
      UtlString callId;
      UtlString stale;

      sipResponse.getAuthenticationData(
         &scheme, &realm, &nonce, &opaque,
         &algorithm, &qop, authorizationEntity, &stale);

      SipLineCredential sipCredential;
      UtlBoolean foundCredential = sipLine.getCredential(scheme, realm, sipCredential);
      if (foundCredential)
      {
         lineCredential = sipCredential;
         result = TRUE;
      }
   }

   return result;
}

UtlBoolean SipLineProvider::getProxyServersForMessage(const SipMessage& sipRequest,
                                                      UtlString& proxyServer) const
{
   Url lineUri;
   UtlString userId;

   // get lineUri, userId from SipMessage
   extractLineData(sipRequest, lineUri, userId);

   // do fast lookup to get proxy servers
   return getLineProxyServers(lineUri, proxyServer);
}

void SipLineProvider::extractLineData(const SipMessage& sipMsg,
                                      Url& lineUri,
                                      UtlString& userId)
{
   int seqNum;
   UtlString seqMethod;
   sipMsg.getCSeqField(&seqNum, &seqMethod);

   // get LINEID, lineUri, userId
   if (!sipMsg.isFromThisSide())
   {
      // inbound message
      if (seqMethod.compareTo(SIP_REGISTER_METHOD) == 0)
      {
         // this is REGISTER request or response, RequestUri is different from toUri, use toUri
         Url toUrl;
         sipMsg.getToUrl(toUrl);
         lineUri = SipLine::getLineUri(toUrl); // get lineUri from toUrl
      }
      else
      {
         // INVITE, INFO... These may also have different RequestUri from toUri, if there is redirection 3xx.
         // See RFC-2833 - 8.2.2.1 To and Request-URI
         if (!sipMsg.isResponse())
         {
            // this is inbound request, use request-uri
            UtlString sRequestUri;
            sipMsg.getRequestUri(&sRequestUri);
            Url requestUri(sRequestUri);
            lineUri = SipLine::getLineUri(requestUri); // RequestUri is definitely us, To header field not necessarily..
         }
         else
         {
            // this is inbound response, use from url
            Url fromUrl;
            sipMsg.getFromUrl(fromUrl);
            lineUri = SipLine::getLineUri(fromUrl); // get lineUri from fromUrl
         }
      }
   }
   else
   {
      // outbound message
      if (!sipMsg.isResponse() || seqMethod.compareTo(SIP_REGISTER_METHOD) == 0)
      {
         // this is REGISTER request or response, use fromUrl
         // or it is some outbound request other than REGISTER
         Url fromUrl;
         sipMsg.getFromUrl(fromUrl);
         lineUri = SipLine::getLineUri(fromUrl); // get lineUri from fromUrl
      }
      else
      {
         // outbound response, use toUrl
         Url toUrl;
         sipMsg.getToUrl(toUrl);
         lineUri = SipLine::getLineUri(toUrl); // get lineUri from toUrl
      }
   }

   lineUri.getUserId(userId); // get userId from lineUri
}

void SipLineProvider::getFullLineUrl(const SipMessage& sipRequest, UtlString& sFullLineUrl) const
{
   sFullLineUrl.remove(0);

   Url lineUri;
   UtlString userId;
   SipLineProvider::extractLineData(sipRequest, lineUri, userId);
   SipLine sipLine;
   if (findLineCopy(lineUri, userId, sipLine))
   {
      // we found line
      sFullLineUrl = sipLine.getFullLineUrl().toString();
      return;
   }

   // we didn't find line
   sipRequest.getRequestUri(&sFullLineUrl);
}

/* ============================ INQUIRY =================================== */

UtlBoolean SipLineProvider::lineExists(const SipMessage& sipMsg) const
{
   Url lineUri;
   UtlString userId;

   // get lineUri, userId
   extractLineData(sipMsg, lineUri, userId);

   return lineExists(lineUri, userId);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


