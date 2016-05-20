//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////
// Author: Dan Petrie (dpetrie AT SIPez DOT com)

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>
#include <utl/UtlHashMap.h>

#include <os/OsDefs.h>
#include <net/SipDialog.h>
#include <net/SipMessage.h>
#include <net/SipUserAgent.h>
#include <net/SipDialogMgr.h>
//#include <net/SipRefreshManager.h>
//#include <net/SipSubscribeClient.h>

// Test messages
char subscribe[]="SUBSCRIBE sip:111@example.com SIP/2.0\r\n\
From: \"Dan Petrie\"<sip:111@example.com>;tag=1612c1612\r\n\
To: \"Dan Petrie\"<sip:111@example.com>\r\n\
Call-Id: e2aab34a72a0eb18300fbec445d5d665\r\n\
Cseq: 1 SUBSCRIBE\r\n\
Contact: sip:111@10.1.2.3\r\n\
Event: message-summary\r\n\
Accept: application/simple-message-summary\r\n\
Expires: 3600\r\n\
Date: Tue, 26 Apr 2005 14:59:30 GMT\r\n\
Max-Forwards: 20\r\n\
User-Agent: Pingtel/2.2.0 (VxWorks)\r\n\
Accept-Language: en\r\n\
Supported: sip-cc, sip-cc-01, timer, replaces\r\n\
Via: SIP/2.0/UDP 10.1.2.3;branch=z9hG4bK7ce947ad9439bfeb6226852d87f5cca8\r\n\
Content-Length: 0\r\n\
\r\n";

char subscribe401[] = "SIP/2.0 401 Unauthorized\r\n\
From: \"Dan Petrie\"<sip:111@example.com>;tag=1612c1612\r\n\
To: \"Dan Petrie\"<sip:111@example.com>\r\n\
Call-Id: e2aab34a72a0eb18300fbec445d5d665\r\n\
Cseq: 1 SUBSCRIBE\r\n\
Via: SIP/2.0/UDP 10.1.2.3;branch=z9hG4bK7ce947ad9439bfeb6226852d87f5cca8\r\n\
Www-Authenticate: Digest realm=\"example.com\", nonce=\"606a7e9c58258179f966b0987a1bf38d1114527548\", opaque=\"change4\"\r\n\
Date: Tue, 26 Apr 2005 14:59:08 GMT\r\n\
Allow: INVITE, ACK, CANCEL, BYE, REFER, OPTIONS, NOTIFY, SUBSCRIBE\r\n\
User-Agent: sipX/2.8.0 (Linux)\r\n\
Accept-Language: en\r\n\
Supported: sip-cc-01, timer\r\n\
Contact: sip:10.1.2.4:5110\r\n\
\r\n";

char subscribeAuth[] = "SUBSCRIBE sip:111@example.com SIP/2.0\r\n\
From: \"Dan Petrie\"<sip:111@example.com>;tag=1612c1612\r\n\
To: \"Dan Petrie\"<sip:111@example.com>\r\n\
Call-Id: e2aab34a72a0eb18300fbec445d5d665\r\n\
Cseq: 2 SUBSCRIBE\r\n\
Contact: sip:111@10.1.2.3\r\n\
Event: message-summary\r\n\
Accept: application/simple-message-summary\r\n\
Expires: 3600\r\n\
Date: Tue, 26 Apr 2005 14:59:30 GMT\r\n\
Max-Forwards: 20\r\n\
User-Agent: Pingtel/2.2.0 (VxWorks)\r\n\
Accept-Language: en\r\n\
Supported: sip-cc, sip-cc-01, timer, replaces\r\n\
Authorization: Digest username=\"111\", realm=\"example.com\", nonce=\"606a7e9c58258179f966b0987a1bf38d1114527548\", uri=\"sip:111@example.com\", response=\"feaa478e10ee7d3ef6037746696bace6\", opaque=\"change4\"\r\n\
Via: SIP/2.0/UDP 10.1.1.177;branch=z9hG4bK64807d4040eecf1d8b0ae759505b81b0\r\n\
Content-Length: 0\r\n\
\r\n";

char subscribe202[] = "SIP/2.0 202 Accepted\r\n\
Expires: 3600\r\n\
From: \"Dan Petrie\"<sip:111@example.com>;tag=1612c1612\r\n\
To: \"Dan Petrie\"<sip:111@example.com>;tag=435889744\r\n\
Call-Id: e2aab34a72a0eb18300fbec445d5d665\r\n\
Cseq: 2 SUBSCRIBE\r\n\
Via: SIP/2.0/UDP 10.1.1.177;branch=z9hG4bK64807d4040eecf1d8b0ae759505b81b0\r\n\
Contact: sip:111@example.com\r\n\
Date: Tue, 26 Apr 2005 14:59:08 GMT\r\n\
Allow: INVITE, ACK, CANCEL, BYE, REFER, OPTIONS, NOTIFY, SUBSCRIBE\r\n\
User-Agent: sipX/2.8.0 (Linux)\r\n\
Accept-Language: en\r\n\
Supported: sip-cc-01, timer\r\n\
Content-Length: 0\r\n\
\r\n";

char notify[] = "NOTIFY sip:111@10.1.1.177 SIP/2.0\r\n\
Content-Type: application/simple-message-summary\r\n\
Content-Length: 50\r\n\
Event: message-summary\r\n\
Subscription-State: active;expires=3600\r\n\
From: \"Dan Petrie\"<sip:111@example.com>;tag=435889744\r\n\
To: \"Dan Petrie\"<sip:111@example.com>;tag=1612c1612\r\n\
Call-Id: e2aab34a72a0eb18300fbec445d5d665\r\n\
Cseq: 10 NOTIFY\r\n\
Contact: sip:10.1.20.3:5110\r\n\
Date: Tue, 26 Apr 2005 14:59:08 GMT\r\n\
Max-Forwards: 20\r\n\
User-Agent: sipX/2.8.0 (Linux)\r\n\
Accept-Language: en\r\n\
Supported: sip-cc-01, timer\r\n\
Via: SIP/2.0/UDP 10.1.20.3:5110;branch=z9hG4bK-1334bee34ff713f3b58e898d1a2eaf06\r\n\
\r\n\
Messages-Waiting: no\r\n\
Voice-Message: 0/0 (0/0)\r\n";

/**
 * Unittest for SipMessage
 */
class SipDialogTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(SipDialogTest);
      CPPUNIT_TEST(createSubscribeDialog);
      CPPUNIT_TEST(createDeleteDialogTest);
      CPPUNIT_TEST(isTagSwapNeededTest);
      CPPUNIT_TEST(isInitialDialogTest);
      CPPUNIT_TEST(createDialogNotifyBeforeResponseTest);
      CPPUNIT_TEST(isSameDialogServerPerspectiveTest);
      CPPUNIT_TEST(isSameDialogClientPerspectiveTest);
      CPPUNIT_TEST(isSameDialogStrictServerPerspectiveTest);
      CPPUNIT_TEST(isSameDialogStrictClientPerspectiveTest);
      CPPUNIT_TEST(isInitialDialogOfServerPerspectiveTest);
      CPPUNIT_TEST(isInitialDialogOfClientPerspectiveTest);
      CPPUNIT_TEST(subscribeNotifyServerPerspectiveTest);
      CPPUNIT_TEST(subscribeNotifyClientPerspectiveTest);
      CPPUNIT_TEST_SUITE_END();

      public:

   void createSubscribeDialog()
   {
         const char subscribeRequestString[] =
            "SUBSCRIBE sip:sipuaconfig@sipuaconfig.example.com SIP/2.0\r\n\
From: sip:10.1.1.10;Vendor=Pingtel;Model=xpressa_strongarm_vxworks;Version=2.4.0.0009;Serial=00d01e00f4f4;Mac=00d01e00f4f4;tag=17747cec9\r\n\
To: sip:sipuaconfig@sipuaconfig.example.com\r\n\
Call-Id: config-17747cec9-00d01e004e6f@10.1.1.10\r\n\
Cseq: 7 SUBSCRIBE\r\n\
Contact: sip:10.1.1.10\r\n\
Event: sip-config\r\n\
Config_allow: http\r\n\
Config_require: x-xpressa-apps, x-xpressa-device, x-xpressa-install, x-xpressa-user\r\n\
Expires: 86400\r\n\
Date: Tue, 26 Apr 2005 03:54:16 GMT\r\n\
Max-Forwards: 20\r\n\
User-Agent: Pingtel/2.4.0 (VxWorks)\r\n\
Accept-Language: en\r\n\
Supported: sip-cc, sip-cc-01, timer, replaces\r\n\
Via: SIP/2.0/UDP 10.1.1.10;branch=z9hG4bKedff6a4031b8220192be959d966159b6\r\n\
Content-Length: 0\r\n";
         const char subscribeResponseString[] = 
             "SIP/2.0 202 Accepted\r\n\
Expires: 86400\r\n\
From: sip:10.1.1.10;Vendor=Pingtel;Model=xpressa_strongarm_vxworks;Version=2.4.0.0009;Serial=00d01e004e6f;Mac=00d01e004e6f;tag=17747cec9\r\n\
To: sip:sipuaconfig@sipuaconfig.example.com;tag=1114487634asd\r\n\
Call-Id: config-17747cec9-00d01e004e6f@10.1.1.10\r\n\
Cseq: 7 SUBSCRIBE\r\n\
Via: SIP/2.0/UDP 10.1.1.10;branch=z9hG4bKedff6a4031b8220192be959d966159b6\r\n\
Date: Tue, 26 Apr 2005 03:53:55 GMT\r\n\
Contact: sip:10.1.1.10:5090\r\n\
Allow: INVITE, ACK, CANCEL, BYE, REFER, OPTIONS, SUBSCRIBE\r\n\
User-Agent: sipX/2.8.0 (Linux)\r\n\
Accept-Language: en\r\n\
Content-Length: 0\r\n\r\n";

         SipMessage subRequest(subscribeRequestString, strlen(subscribeRequestString));
         subRequest.setFromThisSide(true);
         SipMessage subResponse(subscribeResponseString, strlen(subscribeResponseString));
         subResponse.setFromThisSide(false);
         SipDialog subDialog(&subRequest);

         UtlString method;
         subDialog.getInitialMethod(method);
         ASSERT_STR_EQUAL(SIP_SUBSCRIBE_METHOD, method.data());

         UtlString dialogHandle;
         subDialog.getDialogHandle(dialogHandle);
         ASSERT_STR_EQUAL("config-17747cec9-00d01e004e6f@10.1.1.10,17747cec9,",
             dialogHandle);

         UtlString callId;
         UtlString localTag;
         UtlString remoteTag;
         SipDialog::parseHandle(dialogHandle,
                            callId,
                            localTag,
                            remoteTag);
         ASSERT_STR_EQUAL("config-17747cec9-00d01e004e6f@10.1.1.10",
             callId.data());
         ASSERT_STR_EQUAL("17747cec9", localTag.data());
         ASSERT_STR_EQUAL("", remoteTag.data());

         subDialog.getCallId(callId);
         ASSERT_STR_EQUAL("config-17747cec9-00d01e004e6f@10.1.1.10",
             callId.data());

         Url fromUri;
         UtlString fromField;
         UtlString fromTag;
         subDialog.getLocalField(fromUri);
         fromUri.toString(fromField);
         ASSERT_STR_EQUAL("<sip:10.1.1.10>;Vendor=Pingtel;Model=xpressa_strongarm_vxworks;Version=2.4.0.0009;Serial=00d01e00f4f4;Mac=00d01e00f4f4;tag=17747cec9",fromField.data());
         subDialog.getLocalTag(fromTag);
         ASSERT_STR_EQUAL("17747cec9", fromTag.data());

         Url toUri;
         UtlString toField;
         UtlString toTag;
         subDialog.getRemoteField(toUri);
         toUri.toString(toField);
         ASSERT_STR_EQUAL("<sip:sipuaconfig@sipuaconfig.example.com>",toField.data());
         subDialog.getRemoteTag(toTag);
         ASSERT_STR_EQUAL("", toTag.data());

         Url remoteContactUri;
         UtlString remoteContactString;
         subDialog.getRemoteContact(remoteContactUri);
         remoteContactUri.toString(remoteContactString);
         // Not set yet as we do not have a contact from the other side
         ASSERT_STR_EQUAL("<sip:>", remoteContactString.data());

         Url localContactUri;
         UtlString localContactString;
         subDialog.getLocalContact(localContactUri);
         localContactUri.toString(localContactString);
         ASSERT_STR_EQUAL("<sip:10.1.1.10>", localContactString.data());

         Url remoteRequestUri;
         subDialog.getRemoteRequestUri(remoteRequestUri);
         ASSERT_STR_EQUAL("sip:sipuaconfig@sipuaconfig.example.com", remoteRequestUri.toString().data());

         Url localRequestUri;
         subDialog.getLocalRequestUri(localRequestUri);
         ASSERT_STR_EQUAL("sip:", localRequestUri.toString().data());

         CPPUNIT_ASSERT_EQUAL(7, subDialog.getLastLocalCseq());
         CPPUNIT_ASSERT_EQUAL(-1, subDialog.getLastRemoteCseq());

         CPPUNIT_ASSERT(subDialog.isInitialDialog());

         CPPUNIT_ASSERT(subDialog.isInitialDialogOf("config-17747cec9-00d01e004e6f@10.1.1.10",
             "17747cec9", "foo"));
         CPPUNIT_ASSERT(subDialog.isInitialDialogOf(subRequest));
         CPPUNIT_ASSERT(subDialog.isInitialDialogOf(subResponse));


         CPPUNIT_ASSERT(subDialog.isSameDialog("config-17747cec9-00d01e004e6f@10.1.1.10",
             "17747cec9", ""));
         CPPUNIT_ASSERT(subDialog.isSameDialog(subRequest));
         CPPUNIT_ASSERT(subDialog.isInitialDialogOf(subResponse));
         CPPUNIT_ASSERT(subDialog.isSameDialog(subResponse));

         // Update to change early dialog to setup dialog
         UtlString dump;
         subDialog.toString(dump);
         subDialog.updateDialog(subResponse);
         CPPUNIT_ASSERT(!subDialog.isSameDialog(subRequest)); // after updating dialog state, request no longer matches
         UtlString updatedLocalTag;
         UtlString updatedRemoteTag;
         subDialog.getLocalTag(updatedLocalTag);
         CPPUNIT_ASSERT(updatedLocalTag.compareTo("17747cec9") == 0);
         subDialog.getRemoteTag(updatedRemoteTag);
         ASSERT_STR_EQUAL("1114487634asd", updatedRemoteTag.data());
         CPPUNIT_ASSERT(subDialog.isSameDialog(subResponse));
         CPPUNIT_ASSERT(subDialog.isInitialDialogOf("config-17747cec9-00d01e004e6f@10.1.1.10",
             "17747cec9", ""));

      };

      void isInitialDialogTest()
      {
         CPPUNIT_ASSERT(!SipDialog::isInitialDialog("call-id,localtag,remotetag"));
         CPPUNIT_ASSERT(SipDialog::isInitialDialog("call-id,localtag,"));
         CPPUNIT_ASSERT(SipDialog::isInitialDialog("call-id,,remotetag"));

         CPPUNIT_ASSERT(!SipDialog::isInitialDialog("call-id", "localtag", "remotetag"));
         CPPUNIT_ASSERT(SipDialog::isInitialDialog("call-id", NULL, "remotetag"));
         CPPUNIT_ASSERT(SipDialog::isInitialDialog("call-id", "localtag", NULL));
      }

      void isTagSwapNeededTest()
      {
         SipMessage inboundSubWithAuthRequest(subscribeAuth); // request inbound
         inboundSubWithAuthRequest.setFromThisSide(false);
         SipMessage outboundSubWithAuthRequest(subscribeAuth); // request outbound
         outboundSubWithAuthRequest.setFromThisSide(true);
         SipMessage inboundSub202Response(subscribe202); // response inbound
         inboundSub202Response.setFromThisSide(false);
         SipMessage outboundSub202Response(subscribe202); // response outbound
         outboundSub202Response.setFromThisSide(true);

         CPPUNIT_ASSERT(SipDialog::isTagSwapNeeded(inboundSubWithAuthRequest));
         CPPUNIT_ASSERT(!SipDialog::isTagSwapNeeded(outboundSubWithAuthRequest));
         CPPUNIT_ASSERT(!SipDialog::isTagSwapNeeded(inboundSub202Response));
         CPPUNIT_ASSERT(SipDialog::isTagSwapNeeded(outboundSub202Response));
      }

      void createDeleteDialogTest()
      {
         SipDialogMgr dialogMgr;
         SipMessage subWithAuthRequest(subscribeAuth);
         subWithAuthRequest.setFromThisSide(false);
         CPPUNIT_ASSERT(dialogMgr.createDialog(subWithAuthRequest, FALSE));
         CPPUNIT_ASSERT(dialogMgr.countDialogs() == 1);

         UtlString earlyDialogHandle;
         SipDialog::getDialogHandle(subWithAuthRequest, earlyDialogHandle);
         dialogMgr.deleteDialog(earlyDialogHandle);
         CPPUNIT_ASSERT(dialogMgr.countDialogs() == 0);
      }

      void createDialogNotifyBeforeResponseTest()
      {
         SipMessage subWithAuthRequest(subscribeAuth);
         subWithAuthRequest.setFromThisSide(true);
         SipMessage sub202Response(subscribe202);
         sub202Response.setFromThisSide(false);
         SipMessage notifyRequest(notify);
         notifyRequest.setFromThisSide(true);
         SipDialog sipDialog(&subWithAuthRequest);
         CPPUNIT_ASSERT(sipDialog.isInitialDialog());
         sipDialog.updateDialog(notifyRequest); // first give it notify
         CPPUNIT_ASSERT(sipDialog.isEstablishedDialog()); // we must be established dialog now
         sipDialog.updateDialog(sub202Response);
         CPPUNIT_ASSERT(sipDialog.isEstablishedDialog()); // we must be still established dialog
      }

      void isSameDialogStrictServerPerspectiveTest()
      {
         SipMessage subWithAuthRequest(subscribeAuth);
         subWithAuthRequest.setFromThisSide(false);
         SipMessage sub202Response(subscribe202);
         sub202Response.setFromThisSide(true);
         SipMessage notifyRequest(notify);
         notifyRequest.setFromThisSide(true);

         SipDialog sipDialog(&subWithAuthRequest);
         // all must succeed
         CPPUNIT_ASSERT(sipDialog.isSameDialog(subWithAuthRequest, TRUE));
         CPPUNIT_ASSERT(!sipDialog.isSameDialog(sub202Response, TRUE));
         CPPUNIT_ASSERT(!sipDialog.isSameDialog(notifyRequest, TRUE));
         sipDialog.updateDialog(sub202Response); // this will establish dialog
         CPPUNIT_ASSERT(!sipDialog.isSameDialog(subWithAuthRequest, TRUE));
         CPPUNIT_ASSERT(sipDialog.isSameDialog(sub202Response, TRUE));
         CPPUNIT_ASSERT(sipDialog.isSameDialog(notifyRequest, TRUE));
         sipDialog.updateDialog(notifyRequest); // dialog is already established
         CPPUNIT_ASSERT(!sipDialog.isSameDialog(subWithAuthRequest, TRUE));
         CPPUNIT_ASSERT(sipDialog.isSameDialog(sub202Response, TRUE));
         CPPUNIT_ASSERT(sipDialog.isSameDialog(notifyRequest, TRUE));
      }

      void isSameDialogStrictClientPerspectiveTest()
      {
         SipMessage subWithAuthRequest(subscribeAuth);
         subWithAuthRequest.setFromThisSide(true);
         SipMessage sub202Response(subscribe202);
         sub202Response.setFromThisSide(false);
         SipMessage notifyRequest(notify);
         notifyRequest.setFromThisSide(false);

         SipDialog sipDialog(&subWithAuthRequest);
         CPPUNIT_ASSERT(sipDialog.isSameDialog(subWithAuthRequest, TRUE));
         CPPUNIT_ASSERT(!sipDialog.isSameDialog(sub202Response, TRUE));
         CPPUNIT_ASSERT(!sipDialog.isSameDialog(notifyRequest, TRUE));
         sipDialog.updateDialog(sub202Response); // this will establish dialog
         CPPUNIT_ASSERT(!sipDialog.isSameDialog(subWithAuthRequest, TRUE));
         CPPUNIT_ASSERT(sipDialog.isSameDialog(sub202Response, TRUE));
         CPPUNIT_ASSERT(sipDialog.isSameDialog(notifyRequest, TRUE));
         sipDialog.updateDialog(notifyRequest); // dialog is already established
         CPPUNIT_ASSERT(!sipDialog.isSameDialog(subWithAuthRequest, TRUE)); 
         CPPUNIT_ASSERT(sipDialog.isSameDialog(sub202Response, TRUE));
         CPPUNIT_ASSERT(sipDialog.isSameDialog(notifyRequest, TRUE));
      }

      void isSameDialogServerPerspectiveTest()
      {
         SipMessage subWithAuthRequest(subscribeAuth);
         subWithAuthRequest.setFromThisSide(false);
         SipMessage sub202Response(subscribe202);
         sub202Response.setFromThisSide(true);
         SipMessage notifyRequest(notify);
         notifyRequest.setFromThisSide(true);

         SipDialog sipDialog(&subWithAuthRequest);
         // all must succeed
         CPPUNIT_ASSERT(sipDialog.isSameDialog(subWithAuthRequest));
         CPPUNIT_ASSERT(sipDialog.isSameDialog(sub202Response));
         CPPUNIT_ASSERT(sipDialog.isSameDialog(notifyRequest));
         sipDialog.updateDialog(sub202Response); // this will establish dialog
         CPPUNIT_ASSERT(!sipDialog.isSameDialog(subWithAuthRequest)); // original message will no longer match
         CPPUNIT_ASSERT(sipDialog.isSameDialog(sub202Response));
         CPPUNIT_ASSERT(sipDialog.isSameDialog(notifyRequest));
         sipDialog.updateDialog(notifyRequest); // dialog is already established
         CPPUNIT_ASSERT(!sipDialog.isSameDialog(subWithAuthRequest)); // original message will no longer match
         CPPUNIT_ASSERT(sipDialog.isSameDialog(sub202Response));
         CPPUNIT_ASSERT(sipDialog.isSameDialog(notifyRequest));
      }

      void isSameDialogClientPerspectiveTest()
      {
         SipMessage subWithAuthRequest(subscribeAuth);
         subWithAuthRequest.setFromThisSide(true);
         SipMessage sub202Response(subscribe202);
         sub202Response.setFromThisSide(false);
         SipMessage notifyRequest(notify);
         notifyRequest.setFromThisSide(false);

         SipDialog sipDialog(&subWithAuthRequest);
         CPPUNIT_ASSERT(sipDialog.isSameDialog(subWithAuthRequest));
         CPPUNIT_ASSERT(sipDialog.isSameDialog(sub202Response));
         CPPUNIT_ASSERT(sipDialog.isSameDialog(notifyRequest));
         sipDialog.updateDialog(sub202Response); // this will establish dialog
         CPPUNIT_ASSERT(!sipDialog.isSameDialog(subWithAuthRequest)); // original message will no longer match
         CPPUNIT_ASSERT(sipDialog.isSameDialog(sub202Response));
         CPPUNIT_ASSERT(sipDialog.isSameDialog(notifyRequest));
         sipDialog.updateDialog(notifyRequest); // dialog is already established
         CPPUNIT_ASSERT(!sipDialog.isSameDialog(subWithAuthRequest)); // original message will no longer match
         CPPUNIT_ASSERT(sipDialog.isSameDialog(sub202Response));
         CPPUNIT_ASSERT(sipDialog.isSameDialog(notifyRequest));
      }

      void isInitialDialogOfServerPerspectiveTest()
      {
         SipMessage subWithAuthRequest(subscribeAuth);
         subWithAuthRequest.setFromThisSide(false);
         SipMessage sub202Response(subscribe202);
         sub202Response.setFromThisSide(true);
         SipMessage notifyRequest(notify);
         notifyRequest.setFromThisSide(true);

         SipDialog sipDialog(&subWithAuthRequest);
         // all must succeed
         CPPUNIT_ASSERT(sipDialog.isInitialDialogOf(subWithAuthRequest));
         CPPUNIT_ASSERT(sipDialog.isInitialDialogOf(sub202Response));
         CPPUNIT_ASSERT(sipDialog.isInitialDialogOf(notifyRequest));
         sipDialog.updateDialog(sub202Response); // this will establish dialog
         CPPUNIT_ASSERT(sipDialog.isInitialDialogOf(subWithAuthRequest)); // original message must still match
         CPPUNIT_ASSERT(sipDialog.isInitialDialogOf(sub202Response));
         CPPUNIT_ASSERT(sipDialog.isInitialDialogOf(notifyRequest));
         sipDialog.updateDialog(notifyRequest); // dialog is already established
         CPPUNIT_ASSERT(sipDialog.isInitialDialogOf(subWithAuthRequest)); // original message must still match
         CPPUNIT_ASSERT(sipDialog.isInitialDialogOf(sub202Response));
         CPPUNIT_ASSERT(sipDialog.isInitialDialogOf(notifyRequest));
      }

      void isInitialDialogOfClientPerspectiveTest()
      {
         SipMessage subWithAuthRequest(subscribeAuth);
         subWithAuthRequest.setFromThisSide(true);
         SipMessage sub202Response(subscribe202);
         sub202Response.setFromThisSide(false);
         SipMessage notifyRequest(notify);
         notifyRequest.setFromThisSide(false);

         SipDialog sipDialog(&subWithAuthRequest);
         CPPUNIT_ASSERT(sipDialog.isInitialDialogOf(subWithAuthRequest));
         CPPUNIT_ASSERT(sipDialog.isInitialDialogOf(sub202Response));
         CPPUNIT_ASSERT(sipDialog.isInitialDialogOf(notifyRequest));
         sipDialog.updateDialog(sub202Response); // this will establish dialog
         CPPUNIT_ASSERT(sipDialog.isInitialDialogOf(subWithAuthRequest)); // original message must still match
         CPPUNIT_ASSERT(sipDialog.isInitialDialogOf(sub202Response));
         CPPUNIT_ASSERT(sipDialog.isInitialDialogOf(notifyRequest));
         sipDialog.updateDialog(notifyRequest); // dialog is already established
         CPPUNIT_ASSERT(sipDialog.isInitialDialogOf(subWithAuthRequest)); // original message must still match
         CPPUNIT_ASSERT(sipDialog.isInitialDialogOf(sub202Response));
         CPPUNIT_ASSERT(sipDialog.isInitialDialogOf(notifyRequest));
      }

      void subscribeNotifyServerPerspectiveTest()
      {
        // This first set of tests assumes a server prespective
         SipDialogMgr dialogMgr;
         SipMessage subRequest(subscribe);
         subRequest.setFromThisSide(false);
         SipMessage sub401Response(subscribe401);
         sub401Response.setFromThisSide(true);
         SipMessage subWithAuthRequest(subscribeAuth);
         subWithAuthRequest.setFromThisSide(false);
         SipMessage sub202Response(subscribe202);
         sub202Response.setFromThisSide(true);
         SipMessage notifyRequest(notify);
         notifyRequest.setFromThisSide(true);
         CPPUNIT_ASSERT(dialogMgr.createDialog(subWithAuthRequest, FALSE));

         UtlString earlyDialogHandle;
         SipDialog::getDialogHandle(subRequest, earlyDialogHandle);

         UtlString dummyDialog;
         CPPUNIT_ASSERT(!dialogMgr.getEstablishedDialogHandleFor(earlyDialogHandle,
             dummyDialog));

         UtlString establishedDialogHandle;
         dialogMgr.updateDialog(sub202Response);
         CPPUNIT_ASSERT(dialogMgr.getEstablishedDialogHandleFor(earlyDialogHandle,
             establishedDialogHandle));

         CPPUNIT_ASSERT(dialogMgr.countDialogs() == 1);
         dialogMgr.updateDialog(notifyRequest);
         SipMessage nextNotifyToSend;
         dialogMgr.setNextLocalTransactionInfo(nextNotifyToSend, 
                                               SIP_NOTIFY_METHOD,
                                               establishedDialogHandle);
         int cseq;
         UtlString notifyMethod;
         nextNotifyToSend.getCSeqField(&cseq, &notifyMethod);
         CPPUNIT_ASSERT(cseq == 11);
         ASSERT_STR_EQUAL(notifyMethod.data(), SIP_NOTIFY_METHOD);
         UtlString notifyTo;
         UtlString notifyFrom;
         nextNotifyToSend.getToField(&notifyTo);
         nextNotifyToSend.getFromField(&notifyFrom);
         UtlString subTo;
         UtlString subFrom;
         sub202Response.getToField(&subTo);
         sub202Response.getFromField(&subFrom);
         ASSERT_STR_EQUAL(subTo, notifyFrom);
         ASSERT_STR_EQUAL(subFrom, notifyTo);
         UtlString nextNotifyCallId;
         nextNotifyToSend.getCallIdField(&nextNotifyCallId);
         UtlString subCallId;
         subRequest.getCallIdField(&subCallId);
         ASSERT_STR_EQUAL(nextNotifyCallId, subCallId);

         dialogMgr.deleteDialog(earlyDialogHandle);
         CPPUNIT_ASSERT(dialogMgr.countDialogs() == 0);
      }

      void subscribeNotifyClientPerspectiveTest()
      {
         // This first set of tests assumes a client prespective
         SipDialogMgr dialogMgr;
         SipMessage subRequest(subscribe);
         subRequest.setFromThisSide(true);
         SipMessage sub401Response(subscribe401);
         sub401Response.setFromThisSide(false);
         SipMessage subWithAuthRequest(subscribeAuth);
         subWithAuthRequest.setFromThisSide(true);
         SipMessage sub202Response(subscribe202);
         sub202Response.setFromThisSide(false);
         SipMessage notifyRequest(notify);
         notifyRequest.setFromThisSide(false);

         // The following simulate the client side of a SUBSCRIBE dialog
         CPPUNIT_ASSERT(dialogMgr.createDialog(subRequest, TRUE));
         UtlString earlyDialogHandle;
         SipDialog::getDialogHandle(subRequest, earlyDialogHandle);

         CPPUNIT_ASSERT(dialogMgr.countDialogs() == 1);
         CPPUNIT_ASSERT(dialogMgr.updateDialog(sub401Response));
         CPPUNIT_ASSERT(dialogMgr.initialDialogExists(earlyDialogHandle));
         CPPUNIT_ASSERT(dialogMgr.updateDialog(subWithAuthRequest));
         UtlString establishedDialogHandle;
         CPPUNIT_ASSERT(dialogMgr.updateDialog(sub202Response));
         CPPUNIT_ASSERT(dialogMgr.getEstablishedDialogHandleFor(earlyDialogHandle,
            establishedDialogHandle));
         CPPUNIT_ASSERT(dialogMgr.dialogExists(establishedDialogHandle));
         SipMessage nextSubRequest;
         // Build a new SUBSCRIBE to send (e.g. a refresh)
         dialogMgr.setNextLocalTransactionInfo(nextSubRequest,
            SIP_SUBSCRIBE_METHOD,
            establishedDialogHandle);
         UtlString nextSubTo;
         UtlString nextSubFrom;
         nextSubRequest.getToField(&nextSubTo);
         nextSubRequest.getFromField(&nextSubFrom);
         UtlString subTo;
         UtlString subFrom;
         sub202Response.getToField(&subTo);
         sub202Response.getFromField(&subFrom);
         ASSERT_STR_EQUAL(subTo, nextSubTo);
         ASSERT_STR_EQUAL(subFrom, nextSubFrom);
         int nextSubCseq;
         UtlString subMethod;
         nextSubRequest.getCSeqField(&nextSubCseq, &subMethod);
         ASSERT_STR_EQUAL(subMethod.data(), SIP_SUBSCRIBE_METHOD);
         CPPUNIT_ASSERT(nextSubCseq == 3);

         // Already exists, should not create another dialog
         CPPUNIT_ASSERT(!dialogMgr.createDialog(nextSubRequest, TRUE));
      }

};

CPPUNIT_TEST_SUITE_REGISTRATION(SipDialogTest);
