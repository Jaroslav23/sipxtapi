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

#ifndef OsNetworkAdapterInfo_h__
#define OsNetworkAdapterInfo_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <utl/UtlSList.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * Contains information about network adapter like name, description, ip addresses.
 * Each network adapter must have a unique name.
 *
 * This class is immutable.
 */
class OsNetworkAdapterInfo : public UtlCopyableContainable
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   static const UtlContainableType TYPE;    /** < Class type used for runtime checking */

   /* ============================ CREATORS ================================== */

   /**
    * Constructor.
    *
    * @param ipAddresses list of UtlString instances, will be managed by this class.
    */
   OsNetworkAdapterInfo(const UtlString& name,
                        const UtlString& description,
                        UtlSList* pIpAddresses);

   /**
    * Destructor.
    */
   virtual ~OsNetworkAdapterInfo();

   /**
    * Copy constructor.
    */
   OsNetworkAdapterInfo(const OsNetworkAdapterInfo& rhs);

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

   /* ============================ ACCESSORS ================================= */

   void getName(UtlString& name) const { name = m_name; }
   void getDescription(UtlString& description) const { description = m_description; }

   /**
    * Returns list of UtlString entries with ip addresses.
    */
   const UtlSList* getIpAddresses() const { return m_pIpAddresses; }

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsNetworkAdapterInfo& operator=(const OsNetworkAdapterInfo& rhs);

   UtlString m_name; ///< name of network adapter, in Linux "eth0" etc
   UtlString m_description; ///< printable name of network adapter for Windows
   UtlSList* m_pIpAddresses; ///< ip addresses assigned to network adapter
};

#endif // OsNetworkAdapterInfo_h__
