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

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlInt.h>
#include <utl/UtlString.h>
#include <net/SipLine.h>
#include <net/SipLineMgr.h>
#include <net/SipLineCredential.h>
#include <CompareHelper.h>
#include <sipxunit/TestUtilities.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

// DEFINES
#define FULL_LINE_URL_1 "\"John Doe\"<sip:user1:password1@host1:5060;urlparm=value1?headerParam=value1>;fieldParam=value1"
#define FULL_LINE_URL_1_ALIAS "\"John Doe\"<sip:user3:password3@host3;urlparm=value1?headerParam=value1>;fieldParam=value1"
#define FULL_LINE_URL_2 "\"Jane Doe\"<sip:user2:password2@host2:5061;urlparm=value2?headerParam=value2>;fieldParam=value2"
#define USER_ID_1 "user1"
#define USER_ID_2 "user2"
#define IDENTITY_URI_1 "sip:user1@host1"
#define IDENTITY_URI_2 "sip:user2@host2"

#define CREDENTIAL_USERID "John"
#define CREDENTIAL_USERID2 "John2"
#define CREDENTIAL_PASSWORD "password"
#define CREDENTIAL_PASSWORD2 "password2"
#define CREDENTIAL_REALM "test_realm"
#define CREDENTIAL_REALM2 "test_realm2"
#define CREDENTIAL_TYPE "some_type"
#define CREDENTIAL_TYPE2 "some_type2"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

class SipLineMgrTest : public CppUnit::TestCase
{

   CPPUNIT_TEST_SUITE(SipLineMgrTest);
   CPPUNIT_TEST(testAddDeleteLine);
   CPPUNIT_TEST(testAddDeleteAlias);
   CPPUNIT_TEST(testAddDeleteCredentials1);
   CPPUNIT_TEST(testAddDeleteCredentials2);
   CPPUNIT_TEST(testAddDeleteCredentials3);
   CPPUNIT_TEST(testGetNumLines);
   CPPUNIT_TEST(testGetLineCopies);
   CPPUNIT_TEST(testGetLineCopy);
   CPPUNIT_TEST(testGetLineCopyByAlias);
   CPPUNIT_TEST(testGetLineUris);
   CPPUNIT_TEST(testSetStateForLine);
   CPPUNIT_TEST_SUITE_END();

private:

public:
   SipLineMgrTest()
   {
   }

   ~SipLineMgrTest()
   {
   }

   void setUp()
   {
   }

   void tearDown()
   {
   }

   void testAddDeleteLine()
   {
      SipLine line1(FULL_LINE_URL_1);
      SipLineMgr lineMgr;

      CPPUNIT_ASSERT(lineMgr.addLine(line1));
      CPPUNIT_ASSERT(!lineMgr.addLine(line1));
      // now try deletion
      CPPUNIT_ASSERT(lineMgr.deleteLine(line1.getLineUri()));
      CPPUNIT_ASSERT(!lineMgr.deleteLine(line1.getLineUri()));
   }

   void testAddDeleteAlias()
   {
      SipLine line1(FULL_LINE_URL_1);
      SipLineMgr lineMgr;
      Url alias = SipLine::getLineUri(UtlString(FULL_LINE_URL_1_ALIAS));

      lineMgr.addLine(line1);
      // must fail
      CPPUNIT_ASSERT(!lineMgr.deleteLineAlias(alias));
      // must succeed
      CPPUNIT_ASSERT(lineMgr.addLineAlias(alias, line1.getLineUri()));
      // must fail
      CPPUNIT_ASSERT(!lineMgr.addLineAlias(alias, line1.getLineUri()));
      // must succeed
      CPPUNIT_ASSERT(lineMgr.deleteLineAlias(alias));
      // must fail
      CPPUNIT_ASSERT(!lineMgr.deleteLineAlias(alias));

      lineMgr.deleteLine(line1.getLineUri());
   }

   void testAddDeleteCredentials1()
   {
      SipLine line1(FULL_LINE_URL_1);
      Url lineUri = line1.getLineUri();
      SipLineCredential credential(CREDENTIAL_REALM, CREDENTIAL_USERID, CREDENTIAL_PASSWORD, CREDENTIAL_TYPE);
      SipLineMgr lineMgr;

      lineMgr.addLine(line1);
      CPPUNIT_ASSERT(lineMgr.addCredentialForLine(lineUri, credential));
      CPPUNIT_ASSERT(!lineMgr.addCredentialForLine(lineUri, credential)); // 2nd must fail
      CPPUNIT_ASSERT(lineMgr.deleteCredentialForLine(lineUri, CREDENTIAL_REALM, CREDENTIAL_TYPE));
      CPPUNIT_ASSERT(!lineMgr.deleteCredentialForLine(lineUri, CREDENTIAL_REALM, CREDENTIAL_TYPE)); // 2nd must fail
   }

   void testAddDeleteCredentials2()
   {
      SipLine line1(FULL_LINE_URL_1);
      Url lineUri = line1.getLineUri();
      SipLineCredential credential1(CREDENTIAL_REALM, CREDENTIAL_USERID, CREDENTIAL_PASSWORD, CREDENTIAL_TYPE);
      SipLineCredential credential2(CREDENTIAL_REALM2, CREDENTIAL_USERID2, CREDENTIAL_PASSWORD2, CREDENTIAL_TYPE2);
      SipLineMgr lineMgr;

      lineMgr.addLine(line1);
      lineMgr.addCredentialForLine(lineUri, credential1);
      lineMgr.addCredentialForLine(lineUri, credential2);
      CPPUNIT_ASSERT(lineMgr.deleteAllCredentialsForLine(lineUri));
   }

   void testAddDeleteCredentials3()
   {
      SipLine line1(FULL_LINE_URL_1);
      Url lineUri = line1.getLineUri();
      SipLineMgr lineMgr;

      lineMgr.addLine(line1);
      lineMgr.addCredentialForLine(lineUri, CREDENTIAL_REALM, CREDENTIAL_USERID, CREDENTIAL_PASSWORD, CREDENTIAL_TYPE);
      CPPUNIT_ASSERT(lineMgr.deleteCredentialForLine(lineUri, CREDENTIAL_REALM, CREDENTIAL_TYPE));
   }

   void testGetNumLines()
   {
      SipLine line1(FULL_LINE_URL_1);
      SipLine line2(FULL_LINE_URL_2);
      SipLineMgr lineMgr;

      CPPUNIT_ASSERT(lineMgr.getNumLines() == 0);
      lineMgr.addLine(line1);
      CPPUNIT_ASSERT(lineMgr.getNumLines() == 1);
      lineMgr.addLine(line2);
      CPPUNIT_ASSERT(lineMgr.getNumLines() == 2);
      lineMgr.deleteAllLines();
      CPPUNIT_ASSERT(lineMgr.getNumLines() == 0);
   }

   void testGetLineCopies()
   {
      SipLine line1(FULL_LINE_URL_1);
      SipLine line2(FULL_LINE_URL_2);
      SipLineMgr lineMgr;
      lineMgr.addLine(line1);
      lineMgr.addLine(line2);

      UtlSList lineList;
      lineMgr.getLineCopies(lineList);
      CPPUNIT_ASSERT(lineList.entries() == 2);
   }

   void testGetLineCopy()
   {
      SipLine line1(FULL_LINE_URL_1);
      Url lineUri = line1.getLineUri();
      SipLineMgr lineMgr;
      lineMgr.addLine(line1);

      SipLine line2;
      CPPUNIT_ASSERT(lineMgr.getLineCopy(lineUri, line2));
      CPPUNIT_ASSERT(areTheSame(line1, line2));
   }

   void testGetLineCopyByAlias()
   {
      SipLine line1(FULL_LINE_URL_1);
      Url lineUri = line1.getLineUri();
      Url alias = SipLine::getLineUri(UtlString(FULL_LINE_URL_1_ALIAS));
      SipLineMgr lineMgr;
      lineMgr.addLine(line1);
      lineMgr.addLineAlias(alias, line1.getLineUri());

      SipLine line2;
      CPPUNIT_ASSERT(lineMgr.getLineCopy(alias, line2));
      CPPUNIT_ASSERT(areTheSame(line1, line2));
   }

   void testGetLineUris()
   {
      SipLine line1(FULL_LINE_URL_1);
      SipLine line2(FULL_LINE_URL_2);
      SipLineMgr lineMgr;
      lineMgr.addLine(line1);
      lineMgr.addLine(line2);

      UtlSList lineUris;
      lineMgr.getLineUris(lineUris);
      CPPUNIT_ASSERT(lineUris.entries() == 2);
   }

   void testSetStateForLine()
   {
      SipLine line1(FULL_LINE_URL_1, SipLine::LINE_STATE_UNKNOWN);
      Url lineUri = line1.getLineUri();
      line1.setState(SipLine::LINE_STATE_FAILED);
      SipLineMgr lineMgr;
      lineMgr.addLine(line1);

      SipLine line2;
      lineMgr.getLineCopy(lineUri, line2);
      CPPUNIT_ASSERT(line2.getState() == SipLine::LINE_STATE_FAILED);

      CPPUNIT_ASSERT(lineMgr.setStateForLine(lineUri, SipLine::LINE_STATE_REGISTERED));
      lineMgr.getLineCopy(lineUri, line2);
      CPPUNIT_ASSERT(line2.getState() == SipLine::LINE_STATE_REGISTERED);

   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipLineMgrTest);
