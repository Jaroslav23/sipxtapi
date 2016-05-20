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

#ifndef SipContactSelector_h__
#define SipContactSelector_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <net/SipTransport.h>
#include <net/SipContact.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS
class SipUserAgent;
class Url;

/**
 * Responsible for selection of the best contact.
 */
class SipContactSelector
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /**
    * Constructor.
    *
    * @param sipUserAgent reference to existing SipUserAgent instance
    */
   SipContactSelector(SipUserAgent& sipUserAgent);

   ~SipContactSelector();

   /* ============================ MANIPULATORS ============================== */

   /**
    * Tries to select the best contact address and port. Contact address
    * can only be considered accurate if either messageLocalIp or targetIpAddress
    * is known.
    */
   void getBestContactAddress(UtlString& contactIp, ///< will receive selected contact ip
      int& contactPort, ///< will receive selected contact port
      SIP_TRANSPORT_TYPE transport, ///< transport which will be used
      const UtlString& messageLocalIp = NULL, ///< ip of local interface via which message should be sent if known
      const UtlString& targetIpAddress = NULL, ///< where we are sending message
      int targetPort = PORT_NONE) const;

   /**
    * Tries to select the best contact uri. Contact address
    * can only be considered accurate if either messageLocalIp or targetIpAddress
    * is known.
    */
   void getBestContactUri(Url& contactUri, ///< will receive contact uri
      const UtlString& userId, ///< userId to use in contact
      SIP_TRANSPORT_TYPE transport, ///< transport which will be used
      const UtlString& messageLocalIp = NULL, ///< ip of local interface via which message should be sent if known
      const UtlString& targetIpAddress = NULL, ///< where we are sending message
      int targetPort = PORT_NONE) const;

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /**
    * Finds the best local contact address for given target ip address and given transport.
    */
   void findBestLocalContactAddress(UtlString& contactIp, ///< will receive selected contact ip
      int& contactPort, ///< will receive selected contact port
      const UtlString& targetIpAddress,///< where we are sending message
      SIP_TRANSPORT_TYPE transport) const;

   /**
    * Finds the best contact address for given target ip address and given transport.
    */
   void findBestContactAddress(UtlString& contactIp, ///< will receive selected contact ip
      int& contactPort, ///< will receive selected contact port
      const UtlString& targetIpAddress,///< where we are sending message
      int targetPort,
      SIP_TRANSPORT_TYPE transport) const;

   /**
    * Finds out if given ip address is a private ip.
    */
   UtlBoolean isPrivateIp(const UtlString& ipAddress) const;

   SipUserAgent& m_sipUserAgent;
};

#endif // SipContactSelector_h__
