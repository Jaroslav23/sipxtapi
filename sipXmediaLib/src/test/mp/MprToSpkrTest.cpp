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
#include <mp/MprToSpkr.h>
#include <mp/MpBufferMsg.h>

#include "mp/MpGenericResourceTest.h"

///  Unit test for MprToSpkr
class MprToSpkrTest : public MpGenericResourceTest
{
    CPPUNIT_TEST_SUB_SUITE(MprToSpkrTest, MpGenericResourceTest);
    CPPUNIT_TEST(testCreators);
    CPPUNIT_TEST(testDisabled);
    CPPUNIT_TEST(testEnabledNoQueue);
    CPPUNIT_TEST(testEnabledNoData);
    CPPUNIT_TEST(testEnabledWithData);
    CPPUNIT_TEST(testEnabledFullQueue);
    CPPUNIT_TEST_SUITE_END();

/// Length of message queue used to communicate with MprToSpkr
#define MSG_Q_LEN            1

public:

   void testCreators()
   {
       MprToSpkr*        pToSpkr    = NULL;
       OsStatus          res;

       // when we have a flow graph that contains resources and links,
       // verify that destroying the flow graph also gets rid of the resources
       // and links.
       pToSpkr = new MprToSpkr("MprToSpkr",
                               TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC, NULL);

       res = mpFlowGraph->addResource(*pToSpkr);
       CPPUNIT_ASSERT(res == OS_SUCCESS);
   }

   void testDisabled()
   {
       MprToSpkr*        pToSpkr    = NULL;
       OsStatus          res;

       pToSpkr = new MprToSpkr("MprToSpkr",
                               TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC, NULL);
       CPPUNIT_ASSERT(pToSpkr != NULL);

       setupFramework(pToSpkr);

       // TESTCASE 1:
       // pToSpkr disabled, there are no buffers on the input 0.
       CPPUNIT_ASSERT(mpSourceResource->disable());
       CPPUNIT_ASSERT(pToSpkr->disable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // We did not generated any buffers
       CPPUNIT_ASSERT(  !mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                     && !mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid());

       // TESTCASE 2:
       // pToSpkr disabled, there are buffers on the input 0.
       CPPUNIT_ASSERT(mpSourceResource->enable());
       CPPUNIT_ASSERT(pToSpkr->disable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Buffer should be passed through
       CPPUNIT_ASSERT(  mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                     && (mpSourceResource->mLastDoProcessArgs.outBufs[0] ==
                         mpSinkResource->mLastDoProcessArgs.inBufs[0])
                     );

       // Stop flowgraph
       haltFramework();
   }

   void testEnabledNoQueue()
   {
       MprToSpkr*       pToSpkr    = NULL;
       OsStatus         res;

       pToSpkr = new MprToSpkr("MprToSpkr",
                               TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC, NULL);
       CPPUNIT_ASSERT(pToSpkr != NULL);

       setupFramework(pToSpkr);

       // pToSpkr enabled, there are no buffers on the input 0.
       CPPUNIT_ASSERT(mpSourceResource->disable());
       CPPUNIT_ASSERT(pToSpkr->enable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // We did not generated any buffers
       CPPUNIT_ASSERT(  !mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                     && !mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid());

       // pToSpkr enabled, there are buffers on the input 0.
       CPPUNIT_ASSERT(mpSourceResource->enable());
       CPPUNIT_ASSERT(pToSpkr->enable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Buffer should be passed through
       CPPUNIT_ASSERT(  mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                     && (mpSourceResource->mLastDoProcessArgs.outBufs[0] ==
                         mpSinkResource->mLastDoProcessArgs.inBufs[0])
                     );

       // Stop flowgraph
       haltFramework();
   }

   void testEnabledNoData()
   {
       MprToSpkr*        pToSpkr    = NULL;
       OsMsgQ*           pSpkQ      = NULL;
       OsMsgQ*           pEchoQ     = NULL;
       MpAudioBufPtr     pBuf;
       OsStatus          res;

       // Create message queues to get data from MprToSpkr
       pSpkQ = new OsMsgQ(MSG_Q_LEN);
       CPPUNIT_ASSERT(pSpkQ != NULL);
       pEchoQ = new OsMsgQ(MSG_Q_LEN);
       CPPUNIT_ASSERT(pEchoQ != NULL);

       pToSpkr = new MprToSpkr("MprToSpkr",
                               TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC, pEchoQ);
       CPPUNIT_ASSERT(pToSpkr != NULL);

       setupFramework(pToSpkr);

       // pToSpkr enabled, there are no buffers on the input 0, message queue
       // is empty.
       CPPUNIT_ASSERT(mpSourceResource->disable());
       CPPUNIT_ASSERT(pToSpkr->enable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // No buffers processed
       CPPUNIT_ASSERT(  !mpSourceResource->mLastDoProcessArgs.outBufs[0].isValid()
                     && !mpSinkResource->mLastDoProcessArgs.inBufs[0].isValid()
                     && pSpkQ->isEmpty()
                     && pEchoQ->isEmpty()
                     );

       // Stop flowgraph
       haltFramework();

       // Free message queues
       delete pSpkQ;
       delete pEchoQ;
   }

   void testEnabledWithData()
   {
       MprToSpkr*        pToSpkr    = NULL;
       OsMsgQ*           pEchoQ     = NULL;
       MpBufferMsg*      pEchoMsg   = NULL;
       MpBufPtr          pBuf;
       OsStatus          res;

       // Create message queues to get data from MprToSpkr
       pEchoQ = new OsMsgQ(MSG_Q_LEN);
       CPPUNIT_ASSERT(pEchoQ != NULL);

       pToSpkr = new MprToSpkr("MprToSpkr",
                               TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC, pEchoQ);
       CPPUNIT_ASSERT(pToSpkr != NULL);

       setupFramework(pToSpkr);

       // pToSpkr enabled, there are buffers on the input 0, message queue
       // is not full.
       CPPUNIT_ASSERT(mpSourceResource->enable());
       CPPUNIT_ASSERT(pToSpkr->enable());

       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Get messages from the queues (wait for 1 second)
       res = pEchoQ->receive((OsMsg*&)pEchoMsg, OsTime(1000));
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Store output buffer for convenience
       pBuf = mpSourceResource->mLastDoProcessArgs.outBufs[0];

       MpBufPtr echoBuf = pEchoMsg->getBuffer();
       CPPUNIT_ASSERT(echoBuf.isValid()); // echo buffer is a clone of output buffer
       MpAudioBufPtr pEchoQBuf = echoBuf;
       MpAudioBufPtr pSourceBuf = pBuf;
       CPPUNIT_ASSERT(pSourceBuf->compareSamples(pEchoQBuf) == 0); // samples must be equal by content

       // Buffer is sent to queues and to output
       CPPUNIT_ASSERT(mpSinkResource->mLastDoProcessArgs.inBufs[0] == pBuf);

       // Free received message and stored buffer
       pEchoMsg->releaseMsg();
       pBuf.release();

       // Stop flowgraph
       haltFramework();

       // Free message queue
       delete pEchoQ;
   }

   void testEnabledFullQueue()
   {
       MprToSpkr*        pToSpkr    = NULL;
       OsMsgQ*           pEchoQ     = NULL;
       MpBufferMsg*      pEchoMsg   = NULL;
       MpBufPtr          pBuf[2];
       OsStatus          res;

       // Create message queues to get data from MprToSpkr
       pEchoQ = new OsMsgQ(MSG_Q_LEN);
       CPPUNIT_ASSERT(pEchoQ != NULL);

       pToSpkr = new MprToSpkr("MprToSpkr",
                               TEST_SAMPLES_PER_FRAME, TEST_SAMPLES_PER_SEC, pEchoQ);
       CPPUNIT_ASSERT(pToSpkr != NULL);

       setupFramework(pToSpkr);

       // pToSpkr enabled, there are buffers on the input 0
       CPPUNIT_ASSERT(mpSourceResource->enable());
       CPPUNIT_ASSERT(pToSpkr->enable());

       // This will fill the queues
       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Store first output buffer..
       pBuf[0] = mpSourceResource->mLastDoProcessArgs.outBufs[0];

       // Buffer is sent to queue and to output
       CPPUNIT_ASSERT(mpSinkResource->mLastDoProcessArgs.inBufs[0] == pBuf[0]);

       // Queues are full. 
       res = mpFlowGraph->processNextFrame();
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Get messages from the queue
       res = pEchoQ->receive((OsMsg*&)pEchoMsg, OsTime(1000));
       CPPUNIT_ASSERT(res == OS_SUCCESS);

       // Store output buffer for convenience
       pBuf[1] = mpSourceResource->mLastDoProcessArgs.outBufs[0];

       MpBufPtr echoBuf = pEchoMsg->getBuffer();
       // Buffer is failed to send to queues, but appears on the output
       CPPUNIT_ASSERT(echoBuf.isValid()); // echo buffer is a clone of output buffer
       MpAudioBufPtr pEchoQBuf = echoBuf;
       MpAudioBufPtr pSourceBuf = pBuf[0];
       CPPUNIT_ASSERT(pSourceBuf->compareSamples(pEchoQBuf) == 0); // samples must be equal by content

       CPPUNIT_ASSERT(mpSinkResource->mLastDoProcessArgs.inBufs[0] == pBuf[1]);

       // Free received message and stored buffer
       pEchoMsg->releaseMsg();
       pBuf[0].release();
       pBuf[1].release();

       // Stop flowgraph
       haltFramework();

       // Free message queue
       delete pEchoQ;
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(MprToSpkrTest);
