//
// Copyright (C) 2006 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
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

#include <net/SipUserAgent.h>
#include <cp/CpTestSupport.h>
#include <net/SipMessage.h>
#include <net/SipLineMgr.h>
#include <net/SipRefreshMgr.h>
#include <mi/CpMediaInterfaceFactoryFactory.h>

#if defined _WIN32 && !defined WINCE
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

_CrtMemState MemStateBegin;
_CrtMemState MemStateEnd;
_CrtMemState MemStateDiff;
#endif 

#define BROKEN_INITTEST

#define NUM_OF_RUNS 10

/**
 * Unittest for CallManager
 */
class CallManangerTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(CallManangerTest);

    CPPUNIT_TEST(testSimpleTeardown);
    CPPUNIT_TEST_SUITE_END();

public:
    void testSimpleTeardown()
    {
#if defined _WIN32 && !defined WINCE
        _CrtMemCheckpoint(&MemStateBegin);
#endif
        int i;
        for (i=0; i<NUM_OF_RUNS; ++i)
        {
/*            pCallManager->start();           
            pCallManager->requestShutdown();
            delete pCallManager;*/
        }
        
        for (i=0; i<NUM_OF_RUNS; ++i)
        {
//            sipxDestroyMediaFactoryFactory() ;
        }
            
#if defined _WIN32 && !defined WINCE
        _CrtMemCheckpoint(&MemStateEnd);
        if (_CrtMemDifference(&MemStateDiff, &MemStateBegin, &MemStateEnd))
        {
            _CrtMemDumpStatistics(&MemStateDiff);
        }
#endif
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CallManangerTest);
