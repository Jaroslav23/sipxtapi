//  
// Copyright (C) 2007 Jaroslav Libak
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>
#include <mp/MpAudioDriverFactory.h>
#include <mp/MpAudioDriverBase.h>

class MpAudioDriverFactoryTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(MpAudioDriverFactoryTest);
   CPPUNIT_TEST(testCreateAudioDriver);
   CPPUNIT_TEST(testGetDriverNameVersion);
   CPPUNIT_TEST_SUITE_END();

public:

   void setUp()
   {
      // nothing to be done
   }

   void tearDown()
   {
      // nothing to be done
   }

   void testCreateAudioDriver()
   {
      // create portaudio driver
      MpAudioDriverBase* gooddriver = MpAudioDriverFactory::createAudioDriver(MpAudioDriverFactory::AUDIO_DRIVER_PORTAUDIO);
      CPPUNIT_ASSERT(gooddriver);
      MpAudioDriverBase* gooddriver2 = MpAudioDriverFactory::createAudioDriver(MpAudioDriverFactory::AUDIO_DRIVER_PORTAUDIO);
      // we only allow creation of single instance, to prevent problems with deletion
      CPPUNIT_ASSERT(!gooddriver2);
      gooddriver->release();

      MpAudioDriverBase* baddriver = MpAudioDriverFactory::createAudioDriver(MpAudioDriverFactory::AUDIO_DRIVER_LAST);
      CPPUNIT_ASSERT(!baddriver);
   }

   void testGetDriverNameVersion()
   {
      UtlString goodname = MpAudioDriverFactory::getDriverNameVersion(MpAudioDriverFactory::AUDIO_DRIVER_PORTAUDIO);
      printf("Portaudio name and version is: %s\n", goodname.data());
      CPPUNIT_ASSERT(goodname.index("error", 0, UtlString::ignoreCase) == UTL_NOT_FOUND);

      UtlString badname = MpAudioDriverFactory::getDriverNameVersion(MpAudioDriverFactory::AUDIO_DRIVER_LAST);
      CPPUNIT_ASSERT(badname.index("error", 0, UtlString::ignoreCase) != UTL_NOT_FOUND);
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(MpAudioDriverFactoryTest);
