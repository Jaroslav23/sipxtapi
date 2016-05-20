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
#include <net/SipLineCredential.h>
#include <sipxunit/TestUtilities.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <CompareHelper.h>

// DEFINES
#define FULL_LINE_URL_1 "\"John Doe\"<sip:user1:password1@host1:5060;urlparm=value1?headerParam=value1>;fieldParam=value1"
#define USER_ID_1 "user1"
#define IDENTITY_URI_1 "sip:user1@host1"

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

class SipLineCredentialTest : public CppUnit::TestCase
{

   CPPUNIT_TEST_SUITE(SipLineCredentialTest);
   CPPUNIT_TEST(testAddRemove);
   CPPUNIT_TEST(testMd5);
   CPPUNIT_TEST_SUITE_END();

private:

public:
   SipLineCredentialTest()
   {
   }

   ~SipLineCredentialTest()
   {
   }

   void setUp()
   {
   }

   void tearDown()
   {
   }

   void testAddRemove()
   {
      SipLineCredential cred(CREDENTIAL_REALM, CREDENTIAL_USERID, CREDENTIAL_PASSWORD, CREDENTIAL_TYPE);
      CPPUNIT_ASSERT(areTheSame(cred.getUserId(), CREDENTIAL_USERID));
      CPPUNIT_ASSERT(areTheSame(cred.getRealm(), CREDENTIAL_REALM));
      CPPUNIT_ASSERT(areTheSame(cred.getPasswordToken(), CREDENTIAL_PASSWORD));
      CPPUNIT_ASSERT(areTheSame(cred.getType(), CREDENTIAL_TYPE));
   }

   void testMd5()
   {
      SipLineCredential cred(CREDENTIAL_REALM, CREDENTIAL_USERID, CREDENTIAL_PASSWORD, CREDENTIAL_TYPE);

      UtlString md5 = cred.getPasswordMD5Digest(CREDENTIAL_REALM2);
      UtlString correctMd5;
      HttpMessage::buildMd5UserPasswordDigest(CREDENTIAL_USERID,
         CREDENTIAL_REALM2,
         CREDENTIAL_PASSWORD,
         correctMd5);

      CPPUNIT_ASSERT(areTheSame(md5, correctMd5));
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(SipLineCredentialTest);
