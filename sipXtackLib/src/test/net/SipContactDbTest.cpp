//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <os/OsSocket.h>
#include <net/SipContactDb.h>
#include <net/SipContact.h>

/**
 * Unittest for SipContactDb
 */
class SipContactDbTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipContactDbTest);
   CPPUNIT_TEST(testSipContactDb);
   CPPUNIT_TEST_SUITE_END();

public:

   void testSipContactDb()
   {
      // first, create a new contact Db
      SipContactDb pDb;

      // test the inserting of records
      SipContact contact1(-1, SIP_CONTACT_NAT_MAPPED, SIP_TRANSPORT_UDP, "9.9.9.1", 9991, "eth0", "8.9.9.1");
      CPPUNIT_ASSERT(pDb.addContact(contact1));
      CPPUNIT_ASSERT(contact1.getContactId() == 1);

      // test the addition of a duplicate (same IP, port, transport, type)
      // (should fail)
      SipContact contact2(-1, SIP_CONTACT_NAT_MAPPED, SIP_TRANSPORT_UDP, "9.9.9.1", 9991, "eth0", "8.9.9.1");
      CPPUNIT_ASSERT(pDb.addContact(contact2) == FALSE);
      CPPUNIT_ASSERT(contact2.getContactId() == -1);

      // test the addition of same IP, different port
      // (should succeed)
      SipContact contact3(-1, SIP_CONTACT_NAT_MAPPED, SIP_TRANSPORT_UDP, "9.9.9.1", 9992, "eth0", "8.9.9.1");
      CPPUNIT_ASSERT(pDb.addContact(contact3));
      CPPUNIT_ASSERT(contact3.getContactId() == 2);

      // test the addition of transport
      // same adapter
      // (should succeed)
      SipContact contact4(-1, SIP_CONTACT_NAT_MAPPED, SIP_TRANSPORT_TCP, "9.9.9.1", 9992, "eth0", "8.9.9.1");
      CPPUNIT_ASSERT(pDb.addContact(contact4));
      CPPUNIT_ASSERT(contact4.getContactId() == 3);

      // test the addition of different contact type
      // same adapter
      // (should succeed)
      SipContact contact5(-1, SIP_CONTACT_LOCAL, SIP_TRANSPORT_TCP, "9.9.9.1", 9992, "eth0", "8.9.9.1");
      CPPUNIT_ASSERT(pDb.addContact(contact5));
      CPPUNIT_ASSERT(contact5.getContactId() == 4);

      SipContact contact6(-1, SIP_CONTACT_LOCAL, SIP_TRANSPORT_UDP, "9.9.9.1", 9993, "eth1", "8.9.9.1");
      CPPUNIT_ASSERT(pDb.addContact(contact6));
      CPPUNIT_ASSERT(contact6.getContactId() == 5);

      // now test the finding of the records
      SipContact* pFound = NULL;
      // search by ID - positive
      pFound = pDb.find(4);
      CPPUNIT_ASSERT(pFound != NULL);
      CPPUNIT_ASSERT(pFound->getContactId() == 4);
      CPPUNIT_ASSERT(strcmp(pFound->getAdapterName(), "eth0") == 0);
      CPPUNIT_ASSERT(strcmp(pFound->getIpAddress().data(), "9.9.9.1") == 0);
      CPPUNIT_ASSERT(strcmp(pFound->getAdapterIp().data(), "8.9.9.1") == 0);
      CPPUNIT_ASSERT(pFound->getPort() == 9992);
      delete pFound;
      pFound = NULL;

      // search by ID - negative
      pFound = pDb.find(0);
      CPPUNIT_ASSERT(pFound == NULL);

      pFound = pDb.find(SIP_CONTACT_LOCAL, SIP_TRANSPORT_UDP);
      CPPUNIT_ASSERT(pFound != NULL);
      CPPUNIT_ASSERT(pFound->getContactId() == 5);

      pFound = pDb.find(SIP_CONTACT_LOCAL, SIP_TRANSPORT_TLS);
      CPPUNIT_ASSERT(pFound == NULL);

      // get All records
      UtlSList contacts;
      pDb.getAll(contacts);
      CPPUNIT_ASSERT(contacts.entries() == 5);
      contacts.destroyAll();

      // remove records
      CPPUNIT_ASSERT(pDb.deleteContact(5));
      CPPUNIT_ASSERT(pDb.deleteContact(3));
      CPPUNIT_ASSERT(pDb.deleteContact(1));
      CPPUNIT_ASSERT(pDb.deleteContact(2));
      CPPUNIT_ASSERT(pDb.deleteContact(4));
   };
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipContactDbTest);
