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

#ifndef SipContact_h__
#define SipContact_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <utl/UtlInt.h>
#include <net/SipTransport.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * SIP_CONTACT_TYPE is an enumeration of possible address types for use with
 * SIP contacts and SDP connection information.  Application developers and 
 * choose to setup calls with specific contact types (e.g. use my local IP 
 * address, a stun-derived IP address, turn-derived IP address, etc).  Unless
 * you have complete knowledge and control of your network environment, you
 * should likely use CONTACT_AUTO.
 */
typedef enum
{
    SIP_CONTACT_AUTO = 0,   /**< Automatic contact selection; used for API 
                             parameters */
    SIP_CONTACT_LOCAL,      /**< Local address for a particular interface */
    SIP_CONTACT_NAT_MAPPED, /**< NAT mapped address (e.g. STUN)           */
    SIP_CONTACT_RELAY      /**< Relay address (e.g. TURN)                */
} SIP_CONTACT_TYPE;

/**
 * Stores information about a sip contact. Local contacts are created automatically by sipXtackLib
 * when UDP/TCP servers are started. Nat mapped contacts are created if STUN request
 * succeeds and a nat binding is discovered.
 *
 * This class doesn't contain userId part to make it usable for constructing contacts
 * with any userId.
 */
class SipContact : public UtlCopyableContainable
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

   /* ============================ CREATORS ================================== */

   /**
    * Constructor.
    */
   SipContact(int contactId,
              SIP_CONTACT_TYPE contactType,
              SIP_TRANSPORT_TYPE transportType,
              const UtlString& ipAddress,
              int port,
              const UtlString& adapterName = NULL,
              const UtlString& adapterIp = NULL);

   /**
    * Destructor.
    */
   virtual ~SipContact();

   /**
    * Copy constructor.
    */
   SipContact(const SipContact& rhs);

   /* ============================ MANIPULATORS ============================== */

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
    * Builds a contact uri without userId part.
    */
   void buildContactUri(Url& contactUri) const;

   /**
    * Builds a contact uri with given userId.
    */
   void buildContactUri(const UtlString& userId, Url& contactUri) const;

   /**
    * Builds a contact uri with given userId and displayName.
    */
   void buildContactUri(const UtlString& displayName, const UtlString& userId, Url& contactUri) const;

   /**
    * Builds a contact uri with given userId, ip, port, transport.
    */
   static void buildContactUri(Url& contactUri,
                               const UtlString& displayName,
                               const UtlString& userId,
                               const UtlString& ipAddress,
                               int port = PORT_NONE,
                               SIP_TRANSPORT_TYPE transportType = SIP_TRANSPORT_UDP);

   /* ============================ ACCESSORS ================================= */

   void setContactId(int contactId) { m_contactId = contactId; }
   int getContactId() const { return m_contactId; }
   SIP_CONTACT_TYPE getContactType() const { return m_contactType; }
   void setContactType(SIP_CONTACT_TYPE contactType) { m_contactType = contactType; }
   SIP_TRANSPORT_TYPE getTransportType() const { return m_transportType; }
   void getIpAddress(UtlString& ipAddress) const { ipAddress = m_ipAddress; }
   UtlString getIpAddress() const { return m_ipAddress; }
   int getPort() const { return m_port; }
   void getAdapterName(UtlString& adapterName) const { adapterName = m_adapterName; }
   UtlString getAdapterName() const { return m_adapterName; }
   void getAdapterIp(UtlString& adapterIp) const { adapterIp = m_adapterIp; }
   UtlString getAdapterIp() const { return m_adapterIp; }

   /* ============================ INQUIRY =================================== */

   UtlBoolean hasAdapterName(const UtlString& adapterName) const;
   UtlBoolean hasAdapterIp(const UtlString& adapterIp) const;
   UtlBoolean hasIpAddress(const UtlString& ipAddress) const;

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   SipContact& operator=(const SipContact& rhs);

   int m_contactId; ///< unique id assigned to contact
   SIP_CONTACT_TYPE m_contactType; ///< type of contact - if it comes from NAT, local adapter
   SIP_TRANSPORT_TYPE m_transportType; ///< sip transport for which contact was created
   UtlString m_ipAddress; ///< ip address of contact
   int m_port; ///< port for contact -1 means hidden 5060
   UtlString m_adapterName; ///< optional network adapter name
   UtlString m_adapterIp; ///< optional network adapter ip
};

#endif // SipContact_h__
