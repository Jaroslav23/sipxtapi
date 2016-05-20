//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef sipXtapiTestAudio_h__
#define sipXtapiTestAudio_h__

#include <cppunit/extensions/HelperMacros.h>
#include "sipXtapiTest.h"

class sipXtapiTestAudio : public CppUnit::TestFixture
{
   CPPUNIT_TEST_SUITE(sipXtapiTestAudio);

#if TEST_AUDIO /* [ */     
   CPPUNIT_TEST(testGainAPI);
   CPPUNIT_TEST(testMuteAPI);
   CPPUNIT_TEST(testVolumeAPI);
   CPPUNIT_TEST(testAudioSettings);   // requires voiceengine
#endif /* TEST_AUDIO ] */


   CPPUNIT_TEST_SUITE_END();

public:
   sipXtapiTestAudio();

   // set up test
   void setUp();
   // destroy any objects left after test
   void tearDown();

   void testGainAPI();
   void testMuteAPI();
   void testVolumeAPI();
   void testAudioSettings();

private:
   // always use one more than we need
   SIPX_INST m_hInst1;
   SIPX_INST m_hInst2;
};

#endif // sipXtapiTestAudio_h__
