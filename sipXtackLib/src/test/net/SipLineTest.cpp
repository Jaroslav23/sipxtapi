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
#include <sipxunit/TestUtilities.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <net/SipLine.h>
#include <net/SipLineCredential.h>
#include <net/Url.h>
#include <net/SipMessage.h>
#include <CompareHelper.h>

// DEFINES
#define FULL_LINE_URL_1 "\"John Doe\"<sip:user1:password1@host1:5060;urlparm=value1?headerParam=value1>;fieldParam=value1"
#define FULL_LINE_URL_1_SIPS "\"John Doe\"<sips:user1:password1@host1:5060;urlparm=value1?headerParam=value1>;fieldParam=value1"
#define FULL_LINE_URL_2 "\"Jane Doe\"<sip:user2:password2@host2:5061;urlparm=value2?headerParam=value2>;fieldParam=value2"
#define USER_ID_1 "user1"
#define USER_ID_2 "user2"
#define IDENTITY_URI_1 "sip:user1@host1"
#define IDENTITY_URI_1_PORT "sip:user1@host1:5060"
#define IDENTITY_URI_1_SIPS "sip:user1@host1"
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

class SipLineTest : public CppUnit::TestCase
{

   CPPUNIT_TEST_SUITE(SipLineTest);
   CPPUNIT_TEST(testSipLineAssignOperator);
   CPPUNIT_TEST(testSipLineCopyConstructor);
   CPPUNIT_TEST(testSipLineContainableType);
   CPPUNIT_TEST(testSipLineContainableHash);
   CPPUNIT_TEST(testSipLineContainableCompareTo);
   CPPUNIT_TEST(testSipLineState);
   CPPUNIT_TEST(testSipLineUserId);
   CPPUNIT_TEST(testSipLineUri);
   CPPUNIT_TEST(testSipLineFullUrl);
   CPPUNIT_TEST(testSipLineRealm);
   CPPUNIT_TEST(testSipLineContact);
   CPPUNIT_TEST(testSipLineCredentials1);
   CPPUNIT_TEST(testSipLineCredentials2);
   CPPUNIT_TEST(testSipLineStaticAreLineUrisEqual);
   CPPUNIT_TEST(testSipLineStaticGetLineUri);
   CPPUNIT_TEST_SUITE_END();

private:

public:
   SipLineTest()
   {
   }

   ~SipLineTest()
   {
   }

   void setUp()
   {
   }

   void tearDown()
   {
   }

   void testSipLineAssignOperator()
   {
      SipLine line1(FULL_LINE_URL_1);
      SipLine line2(FULL_LINE_URL_2);

      line2 = line1;
      CPPUNIT_ASSERT(areTheSame(line1, line2));
   }

   void testSipLineCopyConstructor()
   {
      SipLine line1(FULL_LINE_URL_1);
      SipLine line2(line1);

      CPPUNIT_ASSERT(areTheSame(line1, line2));
   }

   void testSipLineContainableType()
   {
      SipLine line1(FULL_LINE_URL_1);

      CPPUNIT_ASSERT(areTheSame(line1.getContainableType(), "SipLine"));
   }

   void testSipLineContainableHash()
   {
      SipLine line1(FULL_LINE_URL_1);
      SipLine line2(FULL_LINE_URL_2);
      SipLine line3 = line1;

      CPPUNIT_ASSERT(!areTheSame(line1.hash(), line2.hash()));
      CPPUNIT_ASSERT(areTheSame(line1.hash(), line3.hash()));
   }

   void testSipLineContainableCompareTo()
   {
      SipLine line1(FULL_LINE_URL_1);
      SipLine line2(FULL_LINE_URL_2);

      int result1 = line1.compareTo(&line2);
      int result2 = line2.compareTo(&line1);

      CPPUNIT_ASSERT((result1 > 0 && result2 < 0) || (result1 < 0 && result2 > 0));
   }

   void testSipLineState()
   {
      SipLine line1(FULL_LINE_URL_1, SipLine::LINE_STATE_DISABLED);

      CPPUNIT_ASSERT_EQUAL(line1.getState(), SipLine::LINE_STATE_DISABLED);
      line1.setState(SipLine::LINE_STATE_REGISTERED);
      CPPUNIT_ASSERT_EQUAL(line1.getState(), SipLine::LINE_STATE_REGISTERED);
   }

   void testSipLineUserId()
   {
      SipLine line1(FULL_LINE_URL_1);

      CPPUNIT_ASSERT(areTheSame(line1.getUserId(), USER_ID_1));
   }

   void testSipLineUri()
   {
      SipLine line1(FULL_LINE_URL_1);

      CPPUNIT_ASSERT(areTheSame(line1.getLineUri(), Url(IDENTITY_URI_1, TRUE)));
   }

   void testSipLineFullUrl()
   {
      SipLine line1(FULL_LINE_URL_1);

      CPPUNIT_ASSERT(areTheSame(line1.getFullLineUrl(), SipLine::buildFullLineUrl(Url(FULL_LINE_URL_1))));
   }

   void testSipLineRealm()
   {
      SipLine line1(FULL_LINE_URL_1);

      line1.addCredential("realm", "username", "password", "type");
      CPPUNIT_ASSERT(line1.realmExists("realm"));
   }

   void testSipLineContact()
   {
      SipLine line1(FULL_LINE_URL_1);
      Url correctContact(FULL_LINE_URL_1);
      correctContact.setPassword(NULL);
      correctContact.setPath(NULL);
      correctContact.removeFieldParameters();
      correctContact.includeAngleBrackets();

      UtlString str1 = correctContact.toString();
      UtlString str2 = line1.getPreferredContactUri().toString();

      CPPUNIT_ASSERT(areTheSame(line1.getPreferredContactUri(), correctContact));
   }

   void testSipLineCredentials1()
   {
      SipLine line1(FULL_LINE_URL_1);

      line1.addCredential("realm1", "username1", "password", "type");
      line1.addCredential("realm2", "username2", "password", "type");

      CPPUNIT_ASSERT(line1.getNumOfCredentials() == 2);
   }

   void testSipLineCredentials2()
   {
      SipLine line1(FULL_LINE_URL_1);
      SipLineCredential cred(CREDENTIAL_REALM, CREDENTIAL_USERID, CREDENTIAL_PASSWORD, CREDENTIAL_TYPE);
      CPPUNIT_ASSERT(line1.addCredential(cred));
      CPPUNIT_ASSERT(!line1.addCredential(cred));
      CPPUNIT_ASSERT(line1.addCredential(CREDENTIAL_REALM2, CREDENTIAL_USERID2, CREDENTIAL_PASSWORD2, CREDENTIAL_TYPE2));

      SipLineCredential res;
      CPPUNIT_ASSERT(line1.getCredential(CREDENTIAL_TYPE, CREDENTIAL_REALM, res));
      CPPUNIT_ASSERT(areTheSame(cred.getUserId(), res.getUserId()));
      CPPUNIT_ASSERT(areTheSame(cred.getRealm(), res.getRealm()));
      CPPUNIT_ASSERT(areTheSame(cred.getPasswordToken(), res.getPasswordToken()));
      CPPUNIT_ASSERT(areTheSame(cred.getType(), res.getType()));
   }

   void testSipLineStaticAreLineUrisEqual()
   {
      CPPUNIT_ASSERT(SipLine::areLineUrisEqual(FULL_LINE_URL_1, IDENTITY_URI_1));
      CPPUNIT_ASSERT(SipLine::areLineUrisEqual(FULL_LINE_URL_1, IDENTITY_URI_1_PORT));

      // we consider sips/sip lines to be equal, so that we can use the same authentication password
      // and route calls to the same line
      CPPUNIT_ASSERT(SipLine::areLineUrisEqual(FULL_LINE_URL_1_SIPS, IDENTITY_URI_1_SIPS));
      CPPUNIT_ASSERT(SipLine::areLineUrisEqual(FULL_LINE_URL_1, FULL_LINE_URL_1_SIPS));
   }

   void testSipLineStaticGetLineUri()
   {
      Url lineUri = SipLine::getLineUri(UtlString(FULL_LINE_URL_1));
      Url correctLineUri(IDENTITY_URI_1, TRUE);
      CPPUNIT_ASSERT(areTheSame(lineUri, correctLineUri)); // they must be equal
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipLineTest);
