//
// Copyright (C) 2007-2008 Jaroslav Libak
// Licensed under the LGPL license.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_SSL

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <string.h>
#include <stdlib.h>
#include <cstdarg>

#include <utl/UtlInt.h>
#include <utl/UtlString.h>
#include <os/OsSSL.h>
#include <sipxunit/TestUtilities.h>

using namespace std;

class OsSSLTest : public CppUnit::TestCase
{

   CPPUNIT_TEST_SUITE(OsSSLTest);
   CPPUNIT_TEST(testSSLInit);
   CPPUNIT_TEST(testGetClientConnection);
   CPPUNIT_TEST(testGetServerConnection);
   CPPUNIT_TEST(printCipherList);
   CPPUNIT_TEST_SUITE_END();

private:

public:
   OsSSLTest()
   {
   }

   void setUp()
   {
   }

   void tearDown()
   {
   }

   ~OsSSLTest()
   {
   }

   void testSSLInit() 
   {
      OsSSL* pInstance = OsSSL::getInstance();
      CPPUNIT_ASSERT(pInstance->getInitResult() == OsSSL::SSL_INIT_SUCCESS);
   }

   void testGetClientConnection() 
   {
      OsSSL* pInstance = OsSSL::getInstance();
      CPPUNIT_ASSERT(pInstance->getInitResult() == OsSSL::SSL_INIT_SUCCESS);
   }

   void testGetServerConnection() 
   {
      OsSSL* pInstance = OsSSL::getInstance();

      SSL* pClientConnection = pInstance->getClientConnection();
      CPPUNIT_ASSERT(pClientConnection != NULL);
      pInstance->releaseConnection(pClientConnection);

      SSL* pServerConnection = pInstance->getServerConnection();
      CPPUNIT_ASSERT(pServerConnection != NULL);
      pInstance->releaseConnection(pServerConnection);

   }

   void printCipherList()
   {
      OsSSL* pInstance = OsSSL::getInstance();
      pInstance->dumpCipherList();
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(OsSSLTest);

#endif
