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

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include "sipXtapiTest.h"
#include "EventValidator.h"
#include "callbacks.h"

extern SIPX_INST g_hInst1;
extern SIPX_INST g_hInst2;
extern SIPX_INST g_hInst3;

void sipXtapiTestSuite::testPublishAndSubscribeCall() 
{
   testPublishAndSubscribe(true, "testPublishAndSubscribeCall");
}
void sipXtapiTestSuite::testPublishAndSubscribeConfig() 
{
   testPublishAndSubscribe(false, "testPublishAndSubscribeConfig");
}

void sipXtapiTestSuite::testPublishAndSubscribe(bool bCallContext,
                                                const char* szTestName) 
{
   for (int iStressFactor = 0; iStressFactor<STRESS_FACTOR; iStressFactor++)
   {
      printf("\n%s (%2d of %2d)", szTestName, iStressFactor+1, STRESS_FACTOR);

      EventValidator validatorPublish("testPublishAndSubscribe.publish");
      EventValidator validatorSubscribe1("testPublishAndSubscribe.subscribe1");
      EventValidator validatorSubscribe2("testPublishAndSubscribe.subscribe2");
      SIPX_LINE hLine = 0;
      SIPX_LINE hLine2 = 0;
      SIPX_LINE hLine3 = 0;
      SIPX_RESULT rc;
      SIPX_PUB hPub_coffee  = 0;
      SIPX_PUB hPub_lunch   = 0;
      SIPX_SUB hSub1_coffee = 0;
      SIPX_SUB hSub1_lunch  = 0;
      SIPX_SUB hSub2_coffee = 0;
      SIPX_SUB hSub2_lunch  = 0;
      SIPX_CONTACT_ID contactId = CONTACT_LOCAL;

      validatorPublish.reset();
      validatorPublish.ignoreEventCategory(EVENT_CATEGORY_MEDIA);

      validatorSubscribe1.reset();
      validatorSubscribe1.ignoreEventCategory(EVENT_CATEGORY_MEDIA);
      validatorSubscribe2.reset();
      validatorSubscribe2.ignoreEventCategory(EVENT_CATEGORY_MEDIA);

      sipxEventListenerAdd(g_hInst1, UniversalEventValidatorCallback, &validatorPublish);
      sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorSubscribe1);
      sipxEventListenerAdd(g_hInst3, UniversalEventValidatorCallback, &validatorSubscribe2);

      rc = sipxLineAdd(g_hInst1, "sip:foo@127.0.0.1:8000", &hLine);
      CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

      rc = sipxLineAdd(g_hInst2, "sip:bar@127.0.0.1:9100", &hLine2);
      CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

      rc = sipxLineAdd(g_hInst3, "sip:bar@127.0.0.1:10000", &hLine3);
      CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

      validatorPublish.waitForLineEvent(hLine, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL);        
      validatorSubscribe1.waitForLineEvent(hLine2, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL);        
      validatorSubscribe2.waitForLineEvent(hLine3, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL);        

      UtlString publisherUrl1("foo@127.0.0.1:8000");

      rc = sipxPublisherCreate(g_hInst1, &hPub_coffee, publisherUrl1.data(), "coffee", "application/coffeeStuff", "java ready", 10);
      CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

      rc = sipxPublisherCreate(g_hInst1, &hPub_lunch, publisherUrl1.data(), "lunch", "application/lunchStuff", "order up", 8);
      CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

      if (bCallContext)
      {
         SIPX_CALL hCall1;
         SIPX_LINE hCall2;
         SIPX_CALL hTemp;

         /*
         * Setup Auto-answer call back
         */    

         resetAutoAnswerCallback();
         rc = sipxEventListenerAdd(g_hInst1, AutoAnswerCallback, NULL);
         assert(rc == SIPX_RESULT_SUCCESS);


         // setup and verify call 1

         /*
         * Create Call
         */
         rc = sipxCallCreate(g_hInst2, hLine2, &hCall1);
         CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS);
         bool bRC = validatorSubscribe1.waitForCallEvent(hLine2, hCall1, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL, true);
         CPPUNIT_ASSERT(bRC);

         /*
         * Initiate Call
         */ 
         rc = sipxCallConnect(hCall1, publisherUrl1.data(), SIPX_FOCUS_MANUAL);
         CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS);

         /*
         * Validate events (listener auto-answers)
         */
         // Calling Side
         bRC = validatorSubscribe1.waitForCallEvent(hLine2, hCall1, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, true);
         CPPUNIT_ASSERT(bRC);
         bRC = validatorSubscribe1.waitForCallEvent(hLine2, hCall1, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL, true);
         CPPUNIT_ASSERT(bRC);
         bRC = validatorSubscribe1.waitForCallEvent(hLine2, hCall1, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, true);
         CPPUNIT_ASSERT(bRC);

         sipxCallHold(g_hAutoAnswerCallbackCall, true);

         bRC = validatorSubscribe1.waitForCallEvent(hLine2, hCall1, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL, false);
         CPPUNIT_ASSERT(bRC);

         bRC = !validatorSubscribe1.validateNoWaitingEvent();
         CPPUNIT_ASSERT(bRC);


         // setup and veryify Call 2
         /*
         * Create Call
         */
         OsTask::delay(CALL_DELAY);  // wait for autoanswercallback events to fire

         resetAutoAnswerCallback();

         rc = sipxCallCreate(g_hInst3, hLine3, &hCall2);
         CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS);
         bRC = validatorSubscribe2.waitForCallEvent(hLine3, hCall2, CALLSTATE_DIALTONE, CALLSTATE_CAUSE_NORMAL, false);
         CPPUNIT_ASSERT(bRC);

         /*
         * Initiate Call
         */ 
         rc = sipxCallConnect(hCall2, publisherUrl1.data(), SIPX_FOCUS_MANUAL);
         CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS);

         /*
         * Validate events (listener auto-answers)
         */
         // Calling Side
         bRC = validatorSubscribe2.waitForCallEvent(hLine3, hCall2, CALLSTATE_REMOTE_OFFERING, CALLSTATE_CAUSE_NORMAL, false);
         CPPUNIT_ASSERT(bRC);
         bRC = validatorSubscribe2.waitForCallEvent(hLine3, hCall2, CALLSTATE_REMOTE_ALERTING, CALLSTATE_CAUSE_NORMAL, false);
         CPPUNIT_ASSERT(bRC);
         bRC = validatorSubscribe2.waitForCallEvent(hLine3, hCall2, CALLSTATE_CONNECTED, CALLSTATE_CAUSE_NORMAL, false);
         CPPUNIT_ASSERT(bRC);

         sipxCallHold(g_hAutoAnswerCallbackCall, true);

         bRC = validatorSubscribe2.waitForCallEvent(hLine3, hCall2, CALLSTATE_REMOTE_HELD, CALLSTATE_CAUSE_NORMAL, false);
         CPPUNIT_ASSERT(bRC);

         // ok, now we have a publisher set up, and two calls hCall1, hCall2, have called into
         // the publisher
         validatorSubscribe1.reset();
         validatorSubscribe1.ignoreEventCategory(EVENT_CATEGORY_MEDIA);
         validatorSubscribe2.reset();
         validatorSubscribe2.ignoreEventCategory(EVENT_CATEGORY_MEDIA);

         // hCall1 subscribes to the coffee publisher
         rc = sipxCallSubscribe(hCall1, "coffee", "application/coffeeStuff", &hSub1_coffee, false);
         CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
         validatorSubscribe1.waitForSubStatusEvent(SIPX_SUBSCRIPTION_ACTIVE, SUBSCRIPTION_CAUSE_NORMAL, true);

         // hCall1 receives the initial "java ready" content
         SIPX_NOTIFY_INFO matchingNotify;
         matchingNotify.hSub = hSub1_coffee;
         matchingNotify.nContentLength = 10;
         matchingNotify.szContentType = "application/coffeeStuff";
         matchingNotify.pContent = "java ready";
         validatorSubscribe1.waitForNotifyEvent(&matchingNotify, true);

         // hCall2 subscribes to the coffee publisher
         rc = sipxCallSubscribe(hCall2, "coffee", "application/coffeeStuff", &hSub2_coffee, false);
         CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
         validatorSubscribe2.waitForSubStatusEvent(SIPX_SUBSCRIPTION_ACTIVE, SUBSCRIPTION_CAUSE_NORMAL, true);

         // hCall2 receives the initial "java ready" content
         matchingNotify.hSub = hSub2_coffee;
         validatorSubscribe2.waitForNotifyEvent(&matchingNotify, true);

         // hCall1 subscribes to the lunch publisher
         rc = sipxCallSubscribe(hCall1, "lunch", "application/lunchStuff", &hSub1_lunch, false);
         CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
         validatorSubscribe1.waitForSubStatusEvent(SIPX_SUBSCRIPTION_ACTIVE, SUBSCRIPTION_CAUSE_NORMAL, true);

         // hCall2 subscribes to the lunch publisher
         rc = sipxCallSubscribe(hCall2, "lunch", "application/lunchStuff", &hSub2_lunch, false);
         CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
         validatorSubscribe2.waitForSubStatusEvent(SIPX_SUBSCRIPTION_ACTIVE, SUBSCRIPTION_CAUSE_NORMAL, true);

         // hCall1 receives the initial "order up" content
         matchingNotify.hSub = hSub1_lunch;
         matchingNotify.nContentLength = 8;
         matchingNotify.szContentType = "application/lunchStuff";
         matchingNotify.pContent = "order up";
         validatorSubscribe1.waitForNotifyEvent(&matchingNotify, true);

         // hCall2 receives the initial "order up content
         matchingNotify.hSub = hSub2_lunch;
         validatorSubscribe2.waitForNotifyEvent(&matchingNotify, true);

         // start percolating
         char szPercolatingContent[256];
         for (int i = 0; i < 20; i++)
         {
            sprintf(szPercolatingContent, "Percolating %2.2d", i);
            sipxPublisherUpdate(hPub_coffee, "application/coffeeStuff", szPercolatingContent, 14);

            // hCall1 receives percolation content from the coffee publisher
            matchingNotify.hSub = hSub1_coffee;
            matchingNotify.nContentLength = 14;
            matchingNotify.szContentType = "application/coffeeStuff";
            matchingNotify.pContent = szPercolatingContent;
            validatorSubscribe1.waitForNotifyEvent(&matchingNotify, true);

            // hCall2 receives percolation content from the coffee publisher
            matchingNotify.hSub = hSub2_coffee;
            validatorSubscribe2.waitForNotifyEvent(&matchingNotify, true);
         }

         // Destroy the coffee Publisher, with "out of order" content
         rc = sipxPublisherDestroy(hPub_coffee, "application/coffeeStuff", "out of order", 12);
         CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

         // hCall1 receives the final "out of order" content from the coffee publisher
         matchingNotify.hSub = hSub1_coffee;
         matchingNotify.nContentLength = 12;
         matchingNotify.szContentType = "application/coffeeStuff";
         matchingNotify.pContent = "out of order";
         validatorSubscribe1.waitForNotifyEvent(&matchingNotify, true);

         // hCall1 Unsubscribes from the coffee publisher
         rc = sipxCallUnsubscribe(hSub1_coffee);
         CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
         validatorSubscribe1.waitForSubStatusEvent(SIPX_SUBSCRIPTION_EXPIRED, SUBSCRIPTION_CAUSE_NORMAL, true);

         // hCall2 receives the final "out of order" content from the coffee publisher
         matchingNotify.hSub = hSub2_coffee;
         validatorSubscribe2.waitForNotifyEvent(&matchingNotify, true);
         // hCall2 Unsubscribes from the coffee publisher
         rc = sipxCallUnsubscribe(hSub2_coffee);
         CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
         validatorSubscribe2.waitForSubStatusEvent(SIPX_SUBSCRIPTION_EXPIRED, SUBSCRIPTION_CAUSE_NORMAL, true);

         // Destroy the lunch Publisher, with "check please" content
         rc = sipxPublisherDestroy(hPub_lunch, "application/lunchStuff", "check please", 12);
         CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

         // hCall1 receives the final "check please" content from the lunch publisher
         matchingNotify.hSub = hSub1_lunch;
         matchingNotify.nContentLength = 12;
         matchingNotify.szContentType = "application/lunchStuff";
         matchingNotify.pContent = "check please";
         validatorSubscribe1.waitForNotifyEvent(&matchingNotify, true);


         // hCall1 unsubscribes from the lunch publisher
         rc = sipxConfigUnsubscribe(hSub1_lunch);
         CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
         validatorSubscribe1.waitForSubStatusEvent(SIPX_SUBSCRIPTION_EXPIRED, SUBSCRIPTION_CAUSE_NORMAL, true);

         // hCall2 receives the final "check please" content from the lunch publisher
         matchingNotify.hSub = hSub2_lunch;
         validatorSubscribe2.waitForNotifyEvent(&matchingNotify, true);

         // hCall2 unsubscribes from the lunch publisher
         rc = sipxConfigUnsubscribe(hSub2_lunch);
         CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
         validatorSubscribe2.waitForSubStatusEvent(SIPX_SUBSCRIPTION_EXPIRED, SUBSCRIPTION_CAUSE_NORMAL, true);

         hTemp = hCall2;
         sipxCallDestroy(&hCall2);
         // validatorSubscribe2.waitForCallEvent(hLine3, hTemp, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL, false);
         validatorSubscribe2.waitForCallEvent(hLine3, hTemp, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, false);
         validatorSubscribe2.waitForCallEvent(hLine3, hTemp, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false);

         hTemp = hCall1;
         sipxCallDestroy(&hCall1);
         //validatorSubscribe1.waitForCallEvent(hLine2, hTemp, CALLSTATE_HELD, CALLSTATE_CAUSE_NORMAL, false);
         validatorSubscribe1.waitForCallEvent(hLine2, hTemp, CALLSTATE_DISCONNECTED, CALLSTATE_CAUSE_NORMAL, false);
         validatorSubscribe1.waitForCallEvent(hLine2, hTemp, CALLSTATE_DESTROYED, CALLSTATE_CAUSE_NORMAL, false);

         OsTask::delay(CALL_DELAY);  // wait for any pending events

         rc = sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorSubscribe1);
         CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS);
         rc = sipxEventListenerRemove(g_hInst3, UniversalEventValidatorCallback, &validatorSubscribe2);
         CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS);
         rc = sipxEventListenerRemove(g_hInst1, AutoAnswerCallback, NULL);
         CPPUNIT_ASSERT(rc == SIPX_RESULT_SUCCESS);
      }
      else
      {
         // Line2 subscribes to the coffee publisher
         rc = sipxConfigSubscribe(g_hInst2, hLine2, publisherUrl1.data(), "coffee", "application/coffeeStuff", contactId, &hSub1_coffee);
         CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
         validatorSubscribe1.waitForSubStatusEvent(SIPX_SUBSCRIPTION_ACTIVE, SUBSCRIPTION_CAUSE_NORMAL, true);

         // Line2 receives the initial "java ready" content
         SIPX_NOTIFY_INFO matchingNotify;
         matchingNotify.hSub = hSub1_coffee;
         matchingNotify.nContentLength = 10;
         matchingNotify.szContentType = "application/coffeeStuff";
         matchingNotify.pContent = "java ready";
         validatorSubscribe1.waitForNotifyEvent(&matchingNotify, true);

         // Line3 subscribes to the coffee publisher
         rc = sipxConfigSubscribe(g_hInst3, hLine3, publisherUrl1.data(), "coffee", "application/coffeeStuff", contactId, &hSub2_coffee);
         CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
         validatorSubscribe2.waitForSubStatusEvent(SIPX_SUBSCRIPTION_ACTIVE, SUBSCRIPTION_CAUSE_NORMAL, true);

         // Line3 receives the initial "java ready" content
         matchingNotify.hSub = hSub2_coffee;
         validatorSubscribe2.waitForNotifyEvent(&matchingNotify, true);

         // Line2 subscribes to the lunch publisher
         rc = sipxConfigSubscribe(g_hInst2, hLine2, publisherUrl1.data(), "lunch", "application/lunchStuff", contactId, &hSub1_lunch);
         CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
         validatorSubscribe1.waitForSubStatusEvent(SIPX_SUBSCRIPTION_ACTIVE, SUBSCRIPTION_CAUSE_NORMAL, true);

         // Line3 subscribes to the lunch publisher
         rc = sipxConfigSubscribe(g_hInst3, hLine3, publisherUrl1.data(), "lunch", "application/lunchStuff", contactId, &hSub2_lunch);
         CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
         validatorSubscribe2.waitForSubStatusEvent(SIPX_SUBSCRIPTION_ACTIVE, SUBSCRIPTION_CAUSE_NORMAL, true);

         // Line2 receives the initial "order up" content
         matchingNotify.hSub = hSub1_lunch;
         matchingNotify.nContentLength = 8;
         matchingNotify.szContentType = "application/lunchStuff";
         matchingNotify.pContent = "order up";
         validatorSubscribe1.waitForNotifyEvent(&matchingNotify, true);

         // Line3 receives the initial "order up content
         matchingNotify.hSub = hSub2_lunch;
         validatorSubscribe2.waitForNotifyEvent(&matchingNotify, true);

         // start percolating
         char szPercolatingContent[256];
         for (int i = 0; i < 20; i++)
         {
            sprintf(szPercolatingContent, "Percolating %2.2d", i);
            sipxPublisherUpdate(hPub_coffee, "application/coffeeStuff", szPercolatingContent, 14);

            // hCall1 receives percolation content from the coffee publisher
            matchingNotify.hSub = hSub1_coffee;
            matchingNotify.nContentLength = 14;
            matchingNotify.szContentType = "application/coffeeStuff";
            matchingNotify.pContent = szPercolatingContent;
            validatorSubscribe1.waitForNotifyEvent(&matchingNotify, true);

            // hCall2 receives percolation content from the coffee publisher
            matchingNotify.hSub = hSub2_coffee;
            validatorSubscribe2.waitForNotifyEvent(&matchingNotify, true);
         }

         // Destroy the coffee Publisher, with "out of order" content
         rc = sipxPublisherDestroy(hPub_coffee, "application/coffeeStuff", "out of order", 12);
         CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

         // Line2 receives the final "out of order" content from the coffee publisher
         matchingNotify.hSub = hSub1_coffee;
         matchingNotify.nContentLength = 12;
         matchingNotify.szContentType = "application/coffeeStuff";
         matchingNotify.pContent = "out of order";
         validatorSubscribe1.waitForNotifyEvent(&matchingNotify, true);

         // Line2 Unsubscribes from the coffee publisher
         rc = sipxConfigUnsubscribe(hSub1_coffee);
         CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
         validatorSubscribe1.waitForSubStatusEvent(SIPX_SUBSCRIPTION_EXPIRED, SUBSCRIPTION_CAUSE_NORMAL, true);

         // Line3 receives the final "out of order" content from the coffee publisher
         matchingNotify.hSub = hSub2_coffee;
         validatorSubscribe2.waitForNotifyEvent(&matchingNotify, true);
         // Line3 Unsubscribes from the coffee publisher
         rc = sipxConfigUnsubscribe(hSub2_coffee);
         CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
         validatorSubscribe2.waitForSubStatusEvent(SIPX_SUBSCRIPTION_EXPIRED, SUBSCRIPTION_CAUSE_NORMAL, true);

         // Destroy the lunch Publisher, with "check please" content
         rc = sipxPublisherDestroy(hPub_lunch, "application/lunchStuff", "check please", 12);
         CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);


         // Line2 receives the final "check please" content from the lunch publisher
         matchingNotify.hSub = hSub1_lunch;
         matchingNotify.nContentLength = 12;
         matchingNotify.szContentType = "application/lunchStuff";
         matchingNotify.pContent = "check please";
         validatorSubscribe1.waitForNotifyEvent(&matchingNotify, true);


         // Line2 unsubscribes from the lunch publisher
         rc = sipxConfigUnsubscribe(hSub1_lunch);
         CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
         validatorSubscribe1.waitForSubStatusEvent(SIPX_SUBSCRIPTION_EXPIRED, SUBSCRIPTION_CAUSE_NORMAL, true);

         // LiNE3 receives the final "check please" content from the lunch publisher
         matchingNotify.hSub = hSub2_lunch;
         validatorSubscribe2.waitForNotifyEvent(&matchingNotify, true);

         // Line3 unsubscribes from the lunch publisher
         rc = sipxConfigUnsubscribe(hSub2_lunch);
         CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
         validatorSubscribe2.waitForSubStatusEvent(SIPX_SUBSCRIPTION_EXPIRED, SUBSCRIPTION_CAUSE_NORMAL, true);
      }
      rc = sipxLineRemove(hLine);
      CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

      rc = sipxLineRemove(hLine2);
      CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

      rc = sipxLineRemove(hLine3);
      CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

      sipxEventListenerRemove(g_hInst1, UniversalEventValidatorCallback, &validatorPublish);
      sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorSubscribe1);
      sipxEventListenerRemove(g_hInst3, UniversalEventValidatorCallback, &validatorSubscribe2);

   }

   OsTask::delay(TEST_DELAY);    
   checkForLeaks();
}

void sipXtapiTestSuite::testPublishAndSubscribeConfigLong() 
{
   for (int iStressFactor = 0; iStressFactor < STRESS_FACTOR; iStressFactor++)
   {
      printf("\ntestPublishAndSubscribeConfigLong (%2d of %2d)", iStressFactor+1, STRESS_FACTOR);

      EventValidator validatorPublish("testPublishAndSubscribeConfigLong.publish");
      EventValidator validatorSubscribe("testPublishAndSubscribeConfigLong.subscribe1");
      SIPX_LINE hLine1 = 0;
      SIPX_LINE hLine2 = 0;
      SIPX_RESULT rc;
      SIPX_PUB hPub_coffee = 0;
      SIPX_SUB hSub_coffee = 0;
      SIPX_CONTACT_ID contactId = CONTACT_LOCAL;
      int subscriptionPeriod = 40;

      validatorPublish.reset();
      validatorPublish.ignoreEventCategory(EVENT_CATEGORY_MEDIA);

      validatorSubscribe.reset();
      validatorSubscribe.ignoreEventCategory(EVENT_CATEGORY_MEDIA);

      sipxEventListenerAdd(g_hInst1, UniversalEventValidatorCallback, &validatorPublish);
      sipxEventListenerAdd(g_hInst2, UniversalEventValidatorCallback, &validatorSubscribe);

      rc = sipxLineAdd(g_hInst1, "sip:foo@127.0.0.1:8000", &hLine1);
      CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

      rc = sipxLineAdd(g_hInst2, "sip:bar@127.0.0.1:9100", &hLine2);
      CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

      validatorPublish.waitForLineEvent(hLine1, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL);        
      validatorSubscribe.waitForLineEvent(hLine2, LINESTATE_PROVISIONED, LINESTATE_PROVISIONED_NORMAL);        

      UtlString publisherUrl("foo@127.0.0.1:8000");
      UtlString publisherEventType("coffee");
      UtlString publisherContentType("application/coffeeStuff");
      UtlString publisherInitialContent("java ready");

      rc = sipxPublisherCreate(g_hInst1, &hPub_coffee, publisherUrl, publisherEventType,
         publisherContentType, publisherInitialContent, publisherInitialContent.length());
      CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

      // Line2 subscribes to the coffee publisher
      rc = sipxConfigSubscribe(g_hInst2, hLine2, publisherUrl.data(), publisherEventType,
         publisherContentType, contactId, &hSub_coffee, subscriptionPeriod);
      CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
      validatorSubscribe.waitForSubStatusEvent(SIPX_SUBSCRIPTION_ACTIVE, SUBSCRIPTION_CAUSE_NORMAL, true);

      // Line2 receives the initial "java ready" content
      SIPX_NOTIFY_INFO matchingNotify;
      matchingNotify.hSub = hSub_coffee;
      matchingNotify.szContentType = publisherContentType;
      matchingNotify.pContent = publisherInitialContent;
      matchingNotify.nContentLength = publisherInitialContent.length();
      validatorSubscribe.waitForNotifyEvent(&matchingNotify, true);

      for (int loopNum = 0; loopNum < 3; loopNum++)
      {
         printf("\nIteration %i, Waiting for 1 refresh", loopNum);
         OsTask::delay(subscriptionPeriod*1000*1.2);// we cannot verify refresh event

         // start percolating
         char szPercolatingContent[256];
         for (int i = 0; i < 20; i++)
         {
            sprintf(szPercolatingContent, "Percolating %2.2d", i);
            printf("\nSending NOTIFY %s", szPercolatingContent);
            sipxPublisherUpdate(hPub_coffee, publisherContentType, szPercolatingContent, strlen(szPercolatingContent));

            // hCall1 receives percolation content from the coffee publisher
            matchingNotify.hSub = hSub_coffee;
            matchingNotify.szContentType = publisherContentType;
            matchingNotify.pContent = szPercolatingContent;
            matchingNotify.nContentLength = strlen(szPercolatingContent);
            validatorSubscribe.waitForNotifyEvent(&matchingNotify, true);
            OsTask::delay(100); // not needed, only used to simulate real world
         }
      }

      // Destroy the coffee Publisher, with "out of order" content
      UtlString publisherFinalContent("out of order");
      rc = sipxPublisherDestroy(hPub_coffee, publisherContentType, publisherFinalContent, publisherFinalContent.length());
      CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

      // Line2 receives the final "out of order" content from the coffee publisher
      matchingNotify.hSub = hSub_coffee;
      matchingNotify.szContentType = publisherContentType;
      matchingNotify.pContent = publisherFinalContent;
      matchingNotify.nContentLength = publisherFinalContent.length();
      validatorSubscribe.waitForNotifyEvent(&matchingNotify, true);

      // Line2 Unsubscribes from the coffee publisher
      rc = sipxConfigUnsubscribe(hSub_coffee);
      CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);
      validatorSubscribe.waitForSubStatusEvent(SIPX_SUBSCRIPTION_EXPIRED, SUBSCRIPTION_CAUSE_NORMAL, true);

      rc = sipxLineRemove(hLine1);
      CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

      rc = sipxLineRemove(hLine2);
      CPPUNIT_ASSERT(SIPX_RESULT_SUCCESS == rc);

      sipxEventListenerRemove(g_hInst1, UniversalEventValidatorCallback, &validatorPublish);
      sipxEventListenerRemove(g_hInst2, UniversalEventValidatorCallback, &validatorSubscribe);
   }

   OsTask::delay(TEST_DELAY);    
   checkForLeaks();
}
