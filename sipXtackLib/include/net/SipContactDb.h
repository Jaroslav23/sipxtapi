//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _SipContactDb_h_
#define _SipContactDb_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <utl/UtlHashMap.h>
#include <os/OsMutex.h>
#include <os/OsSocket.h>
#include <net/SipContact.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class SipContactDb
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
    SipContactDb();

    virtual ~SipContactDb();

    /**
     * Inserts a contact into the contact table.  Fails if there is
     * already an entry with the same ip address, port, type and transport.
     *
     * @param sipContact Reference to a contact structure, which will be
     *        copied, and the copy will be added to the DB. Id will be
     *        assigned to passed contact.
     * @return TRUE if operation succeeded
     */
    UtlBoolean addContact(SipContact& sipContact);

    /**
     * Removes a contact record from the DB.  
     *
     * @param id Key value (the contact id) used to find
     *        a matching record for deletion.
     */
    UtlBoolean deleteContact(int contactId);
    
    /** 
     * Finds a contact in the DB, by contactId.
     *
     * @param id The contactId of the record to find.
     * @return copy of SipContact, which must be deleted after usage by caller
     */
    SipContact* find(int contactId) const;

    /**
     * Finds the first contact which satisfies given filter.
     */
    SipContact* find(SIP_CONTACT_TYPE typeFilter = SIP_CONTACT_AUTO,
                     SIP_TRANSPORT_TYPE transportFilter = SIP_TRANSPORT_AUTO) const;

    /**
     * Finds the first contact which satisfies given filter.
     */
    SipContact* find(const UtlString& adapterIp,
                     SIP_CONTACT_TYPE typeFilter = SIP_CONTACT_AUTO,
                     SIP_TRANSPORT_TYPE transportFilter = SIP_TRANSPORT_AUTO) const;

    /**
     * Populates contacts list with all of the contacts stored in this DB.
     */
    void getAll(UtlSList& contacts) const;
    
    /**
     * Populates contact list with all of the contacts
     * stored in this DB that match a particular adapter name and type filter.
     */
    void getAllForAdapterName(UtlSList& contacts,
                              const UtlString& adapterName,
                              SIP_CONTACT_TYPE typeFilter = SIP_CONTACT_AUTO,
                              SIP_TRANSPORT_TYPE transportFilter = SIP_TRANSPORT_AUTO) const;

    /**
     * Populates contact list with all of the contacts
     * stored in this DB that match a particular adapter ip and type filter.
     */
    void getAllForAdapterIp(UtlSList& contacts,
                            const UtlString& adapterIp,
                            SIP_CONTACT_TYPE typeFilter = SIP_CONTACT_AUTO,
                            SIP_TRANSPORT_TYPE transportFilter = SIP_TRANSPORT_AUTO) const;

    
/* ============================ MANIPULATORS ============================== */

    void enableTurn(UtlBoolean bEnable);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

    /** Checks this database for a duplicate record by key */
    UtlBoolean contactExists(int id) const;

    /** Checks this database for a duplicate record by ipAddress and port */
    UtlBoolean contactExists(const UtlString& ipAddress,
                             int port,
                             SIP_CONTACT_TYPE type,
                             SIP_TRANSPORT_TYPE transportType) const;

    /** Checks this database for a duplicate sip contact */
    UtlBoolean contactExists(const SipContact& sipContact) const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    /** Disabled copy constructor */
    SipContactDb(const SipContactDb& rSipContactDb);

    //** Disabled assignment operator */
    SipContactDb& operator=(const SipContactDb& rhs);

    /**
     * Assigns next free id to contact.
     * 
     * @param contact Reference to the SipContact object to be
     *        modified.
     */
    UtlBoolean assignContactId(SipContact& sipContact);

    UtlHashMap m_contacts; ///< hash map storage for contact information, keyed by Contact Record ID
    int m_nextContactId; ///< next free contact id
    UtlBoolean m_bTurnEnabled;
    mutable OsMutex m_mutex;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipContactDb_h_
