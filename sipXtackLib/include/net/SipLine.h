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

#ifndef SipLine_h__
#define SipLine_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlHashBag.h>
#include <utl/UtlCopyableContainable.h>
#include <utl/UtlString.h>
#include "HttpMessage.h"
#include "net/Url.h"
#include <net/SipTransport.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

class SipLineCredential;

/**
 * Class representing a SIP line. Has identity URI, credentials, contact associated with it.
 * Currently only 1 credential per realm is supported of any type.
 * It is not thread safe.
 *
 * Line stores internally so called lineUri and fullLineUrl.
 * - LineURI is meant to be used by hashmaps/hashbags to construct hash, basically uniquely identify
 *   a line. It does't contain any parameters or display name. It is used to match inbound requests to a line.
 *   Additionally, port is not present in LineURI. Our lines are available on all ports of SipUserAgent,
 *   therefore we don't support matching by port.
 * - FullLineUrl is modified value passed into constructor stripped of transport parameter and field parameters.
 *   This is not a sip uri but full url including <>, display name. It is meant to be used to construct sip message
 *   from field.
 */
class SipLine : public UtlCopyableContainable
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

   /** Line states */
   typedef enum
   {
      LINE_STATE_UNKNOWN  = 0,
      LINE_STATE_REGISTERED,  ///< successful registration, registration no expired on server
      LINE_STATE_DISABLED,    ///< not registering
      LINE_STATE_FAILED,      ///< failed registration
      LINE_STATE_PROVISIONED, ///< don't send registration, but enabled because server provisioned it.
      LINE_STATE_TRYING,      ///< registration message sent, awaiting response
      LINE_STATE_EXPIRED      ///< registration expired on server
   } LineStateEnum;

   /* ============================ CREATORS ================================== */
   ///@name Creators
   //@{

   SipLine(const Url& fullLineUrl = "", ///< full line url. field parameters will be cut off
           LineStateEnum state = LINE_STATE_UNKNOWN);

   virtual ~SipLine();
   SipLine(const SipLine& rSipLine);
   SipLine& operator=(const SipLine& rSipLine);

   //@}

   /* ============================ MANIPULATORS ============================== */
   ///@name Manipulators
   //@{

   /**
    * Get the ContainableType for a UtlContainable-derived class.
    */
   virtual UtlContainableType getContainableType() const;

   /** Calculate a hash code for this object. */
   virtual unsigned hash() const;

   /** Compare this object to another object. */
   virtual int compareTo(UtlContainable const *compareContainable) const;

   /** Creates a copy of object */
   virtual UtlCopyableContainable* clone() const;

   /**
    * Constructs line identity uri from given url. <>, parameters and display name
    * will be cut off. This is used in general for converting To/From header field into line uri
    * for line lookup. We deliberately do not adhere strictly to RFC3261 (19.1.4 URI Comparison),
    * because we want line to be transport independent. Url scheme will be overridden to sip if it
    * is sips, in other cases we keep it. This is because we want line lookups not to take security
    * into account. For example sip:alice@example.com and sips:alice@example.com must refer to the same
    * resource. This behaviour has no effect on whether connection is secure or not, it is just to simplify
    * hashmap lookups.
    *
    * All parameters are stripped from input url:
    * - transport, method, ttl, user, maddr and also user made ones
    *
    */
   static Url getLineUri(const Url& url);

   /**
    * Constructs line identity uri from given string. The same as getLineUri taking Url parameter.
    */
   static Url getLineUri(const UtlString& sUrl);

   /**
    * Tests if given line uris are equal.
    */
   static UtlBoolean areLineUrisEqual(const Url& leftUri, const Url& rightUri);

   /**
    * Constructs full line url from given url. Keeps display name, sip uri, but strips
    * transport parameter. Field parameters are removed. Contains <>. Useful for new
    * sip request messages, can be used in from field.
    */
   static Url buildFullLineUrl(const Url& url);

   /**
   * Constructs full line url from given url. Keeps display name, sip uri, but strips
   * transport parameter. Field parameters are removed. Contains <>. Useful for new
   * sip request messages, can be used in from field.
   */
   static Url buildFullLineUrl(const UtlString& sUrl);
   //@}

   /* ============================ ACCESSORS ================================= */
   ///@name Accessors
   //@{

   /** Gets state of line */
   LineStateEnum getState() const;

   /** Gets state of line as string */
   UtlString getStateAsString() const;

   /** Sets state of line. Changing state doesn't trigger any actions. */
   void setState(LineStateEnum state);

   /** Gets UserId part of userEnteredUrl */
   UtlString getUserId() const;

   /** Gets the user entered url used to construct line. */
   Url getFullLineUrl() const;

   /**
    * Gets line identityUri. It doesn't fieldParameters, display name or brackets.
    */
   Url getLineUri() const;

   /** Sets line proxy servers. If not set, default proxy servers might be used. */
   UtlString getProxyServers() const;
   void setProxyServers(const UtlString& proxyServers);

   /** Returns number of credentials stored in line */
   int getNumOfCredentials() const;

   /** Adds credentials for this line */
   UtlBoolean addCredential(const UtlString& strRealm,
                            const UtlString& strUserID,
                            const UtlString& strPassword,
                            const UtlString& strType);

   /** Adds credentials for this line */
   UtlBoolean addCredential(const SipLineCredential& lineCredential);

   /** Gets credentials for given realm. Type is currently ignored. */
   UtlBoolean getCredential(const UtlString& type,
                            const UtlString& realm,
                            UtlString& userID,
                            UtlString& passwordMD5Digest) const;

   /**
    * Gets credentials for given realm. This method can be used to access
    * original clear text password from SipLineCredentials.
    */
   UtlBoolean getCredential(const UtlString& type,
                            const UtlString& realm,
                            SipLineCredential& lineCredentials) const;

   /** Removes credentials for a particular realm */
   UtlBoolean removeCredential(const UtlString& type,
                               const UtlString& realm
                               );

   /** Removes all credentials for this line */
   void removeAllCredentials();

   /**
    * Set the preferred host/ip for the contact in subsequent registers.
    */
   void setPreferredContact(const UtlString& contactAddress,
                            int contactPort);
   
   /** 
    * Get Preferred host/ip for the contact in subsequent registers.
    */
   Url getPreferredContactUri() const;

   /**
    * If true then contact in REGISTER messages may be overridden if better
    * contact is found.
    */
   UtlBoolean getAllowContactOverride() const { return m_bAllowContactOverride; }
   void setAllowContactOverride(UtlBoolean val) { m_bAllowContactOverride = val; }

   /**
    * Gets preferred transport for REGISTER messages.
    */
   SIP_TRANSPORT_TYPE getPreferredTransport() const;
   void setPreferredTransport(SIP_TRANSPORT_TYPE transport);

   //@}
   /* ============================ INQUIRY =================================== */
   ///@name Inquiry
   //@{

   /** Whether credentials for this realm already exist. */
   UtlBoolean realmExists(const UtlString& realm) const;

   //@}

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   Url m_lineUri; /// <line key which is the "userEnteredUrl" stripped of display name, angle brackets and all parameters (basically the URI).
   Url m_fullLineUrl; ///< line Url used to construct sip message from field. Doesn't contain transport parameter

   LineStateEnum m_currentState; ///< current state of line
   Url m_preferredContactUri; ///< contact that will be used in SIP messages
   UtlBoolean m_bAllowContactOverride; ///< when true then a better contact may be selected if found
   SIP_TRANSPORT_TYPE m_preferredTransport; ///< preferred transport for REGISTER messages
   UtlString m_proxyServers; ///< SIP proxy servers address:port, separated by ,

   mutable UtlHashBag m_credentials; ///< bag of SipLineCredential

   void copyCredentials(const SipLine& rSipLine);
   Url buildLineContact(const UtlString& address = NULL, int port = PORT_NONE);

   static const char* convertLineStateToString(LineStateEnum lineState);

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

#endif // SipLine_h__
