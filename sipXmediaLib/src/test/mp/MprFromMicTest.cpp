//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#include <mp/MpMediaTask.h>
#include <mp/MpFlowGraphBase.h>
#include <mp/MpTestResource.h>
#include <mp/MpMisc.h>
#include <mp/MprFromMic.h>
#include <mp/MpBufferMsg.h>

#include "mp/MpGenericResourceTest.h"

///  Unit test for MprFromMic
class MprFromMicTest : public MpGenericResourceTest
{
    CPPUNIT_TEST_SUB_SUITE(MprFromMicTest, MpGenericResourceTest);
    /*CPPUNIT_TEST(testCreators);
    CPPUNIT_TEST(testDisabled);*/
    CPPUNIT_TEST(testEnabledEmptyQueue);
    CPPUNIT_TEST_SUITE_END();

/// Length of message queue used to communicate with MprToSpkr
#define MSG_Q_LEN            1

public:

   void testCreators()
   {
       MprFromMic*       pFromMic = NULL;
       OsStatus          res;

       // when we have a flow graph that contains resources and links,
       // verify that destroying the flow graph also gets rid of the resources
       // and links.
       pFromMic = new MprFromMic("MprFromMic",
                                 TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC);

       res = mpFlowGraph->addResource(*pFromMic);
       CPPUNIT_ASSERT(res == OS_SUCCESS);
   }

   void testDisabled()
   {
       MprFromMic*       pFromMic   = NULL;
       OsStatus          res;

       pFromMic = new MprFromMic("MprFromMic",
                                 TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC);
       CPPUNIT_ASSERT(pFromMic != NULL);

       setupFramework(pFromMic);

       // TESTCASE 1:
       // pFromMic disabled, there are no buffers on the input 0.
       CPPUNIT_ASSERT(mpSourceResource->disable());
       CPPUNIT_ASSERT(pFromMic->disable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // We did not generated any buffers
       CPPUNIT_ASSERT(  !mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                     && !mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid());

       // TESTCASE 2:
       // pFromMic disabled, there are buffers on the input 0.
       CPPUNIT_ASSERT(mpSourceResource->enable());
       CPPUNIT_ASSERT(pFromMic->disable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Buffer should be passed through
       CPPUNIT_ASSERT(  mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                     && (  mpSourceResource->mLastDoProcessArgs.outBufs[0]
                        == mpSinkResource->mLastDoProcessArgs.inBufs[0]));

       // Stop flowgraph
       haltFramework();
   }

   void testEnabledEmptyQueue()
   {
       MprFromMic*       pFromMic   = NULL;
       MpAudioBufPtr     pBuf;
       OsStatus          res;

       pFromMic = new MprFromMic("MprFromMic",
                                 TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC);
       CPPUNIT_ASSERT(pFromMic != NULL);

       setupFramework(pFromMic);

       // TESTCASE 1:
       // pFromMic enabled, there are no buffers on the input 0, message queue
       // is empty.
       CPPUNIT_ASSERT(mpSourceResource->disable());
       CPPUNIT_ASSERT(pFromMic->enable());

       UtlBoolean isInBufValid = mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid();
       CPPUNIT_ASSERT(!isInBufValid);

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Mic must have sent some buffer
       isInBufValid = mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid();
       CPPUNIT_ASSERT(isInBufValid);

       // Stop flowgraph
       haltFramework();
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(MprFromMicTest);
