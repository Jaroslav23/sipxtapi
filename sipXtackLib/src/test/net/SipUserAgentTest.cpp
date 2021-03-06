//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// Author: Scott Zuk
//         szuk AT telusplanet DOT net
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <os/OsTimerTask.h>
#include <os/OsProcess.h>
#include <os/OsNatAgentTask.h>
#include <net/SipMessage.h>
#include <net/SipUserAgent.h>
#include <net/SipLineMgr.h>
#include <net/SipRefreshMgr.h>

#define SHUTDOWN_TEST_ITERATIONS 3

/**
 * Unittest for SipUserAgent
 */
class SipUserAgentTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(SipUserAgentTest);
      CPPUNIT_TEST(testShutdownBlocking);
      CPPUNIT_TEST(testShutdownNonBlocking);
      CPPUNIT_TEST_SUITE_END();

public:

   // OsProcess doesn't provide any thread info so this method returns
   // the number of threads running under the process given by PID.
   // FIXME: Only implemented for linux, always returns 1 otherwise.
   int getNumThreads( int PID )
   {
       int numThreads = 1;

#ifdef __linux__
       // /proc parsing stolen from OsProcessIteratorLinux.cpp
       OsStatus retval = OS_FAILED;
       char pidString[20];
       SNPRINTF(pidString, sizeof(pidString), "%d", PID);

       OsPath fullProcName = "/proc/";
       fullProcName += pidString;
       fullProcName += "/status";
       OsFileLinux procFile(fullProcName);
       if (procFile.open(OsFile::READ_ONLY) == OS_SUCCESS)
       {
           long len = 5000; //since the length is always 0 for these files, lets try to read 5k
           char *buffer = new char[len+1];
           if (buffer)
           {
               unsigned long bytesRead;
               procFile.read((void *)buffer,(unsigned long)len,bytesRead);

               if (bytesRead)
               {
                   procFile.close();
                   //null-terminate the string
                   buffer[bytesRead] = 0;
                   //now parse the info we need
                   char *ptr = strtok(buffer,"\n");
                   while(ptr)
                   {
                       if (memcmp(ptr,"Threads:",8) == 0)
                       {
                           numThreads = atoi(ptr+8);
                       }

                       ptr = strtok(NULL,"\n");
                   }

                   //say we are successful
                   retval = OS_SUCCESS;
               }
               else
                   osPrintf("Couldn't read bytes in readProcFile\n");

               delete [] buffer;
           }

           procFile.close();
       }
#endif
       return numThreads;
   }

   void testShutdownBlocking()
   {
      int myPID = OsProcess::getCurrentPID();
      int startingThreads;

      // Stop TimerTask and NatAgentTask before counting threads.
      // Some tests do not bother stopping them, so they may come started.
      OsTimerTask::destroyTimerTask();
      OsNatAgentTask::releaseInstance();

      // Count number of threads now.
      startingThreads = getNumThreads(myPID);

      // Simple invite message from siptest/src/siptest/invite.txt
      const char* SimpleMessage = 
          "INVITE sip:1@192.168.0.6 SIP/2.0\r\n"
          "Route: <sip:foo@192.168.0.4:5064;lr>\r\n"
          "From: <sip:888@10.1.1.144;user=phone>;tag=bbb\r\n"
          "To: <sip:3000@192.168.0.3:3000;user=phone>\r\n"
          "Call-Id: 8\r\n"
          "Cseq: 1 INVITE\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage testMsg( SimpleMessage, strlen( SimpleMessage ) );

      for(int i = 0; i < SHUTDOWN_TEST_ITERATIONS; ++i)
      {
         // Limit life time of lineMgr and refreshMgr. They should be freed
         // before releasing OsNatAgentTask instance, or we will crash.
         {
            SipRefreshMgr refreshMgr(NULL);
            SipLineMgr    lineMgr(&refreshMgr);
            lineMgr.startLineMgr();

            SipUserAgent sipUA( 5090
                              ,5090
                              ,5091
                              ,"127.0.0.1"     // bind IP address
                              ,NULL     // default defaultUser
                              ,NULL     // default sipProxyServers
                              ,NULL     // default sipDirectoryServers
                              ,NULL     // default authenticationScheme
                              ,NULL     // default authenicateRealm
                              ,NULL     // default authenticateDb
                              ,NULL     // default authorizeUserIds
                              ,NULL     // default authorizePasswords
                              ,&lineMgr
                              );

            sipUA.start();
            refreshMgr.setSipUserAgent(&sipUA);

            sipUA.send(testMsg);

            // Wait long enough for some stack timeouts/retansmits to occur
            OsTask::delay(10000); // 10 seconds

            // Shut down the tasks in reverse order.
            refreshMgr.requestShutdown();
            sipUA.shutdown(TRUE);
            lineMgr.requestShutdown();

            CPPUNIT_ASSERT(sipUA.isShutdownDone());
         }

         // Stop TimerTask and NatAgentTask again before counting threads.
         // They were started while testing.
         OsTimerTask::destroyTimerTask();
         OsNatAgentTask::releaseInstance();

         // Test to see that all the threads created by the above operations
         // get properly shut down.
         // Since the threads do not shut down synchronously with the above
         // calls, we have to wait before we know they will be cleared.
         OsTask::delay(1000);   // 1 second
         int numThreads = getNumThreads(myPID);
         CPPUNIT_ASSERT_EQUAL(startingThreads,numThreads);
      }
   };

   void testShutdownNonBlocking()
   {
      int myPID = OsProcess::getCurrentPID();
      int startingThreads;

      // Stop TimerTask and NatAgentTask before counting threads.
      // Some tests do not bother stopping them, so they may come started.
      OsTimerTask::destroyTimerTask();
      OsNatAgentTask::releaseInstance();

      // Count number of threads now.
      startingThreads = getNumThreads(myPID);

      // Simple invite message from siptest/src/siptest/invite.txt
      const char* SimpleMessage = 
          "INVITE sip:1@192.168.0.6 SIP/2.0\r\n"
          "Route: <sip:foo@192.168.0.4:5064;lr>\r\n"
          "From: <sip:888@10.1.1.144;user=phone>;tag=bbb\r\n"
          "To: <sip:3000@192.168.0.3:3000;user=phone>\r\n"
          "Call-Id: 8\r\n"
          "Cseq: 1 INVITE\r\n"
          "Content-Length: 0\r\n"
          "\r\n";

      SipMessage testMsg( SimpleMessage, strlen( SimpleMessage ) );

      for(int i = 0; i < SHUTDOWN_TEST_ITERATIONS; ++i)
      {
         // Limit life time of lineMgr and refreshMgr. They should be freed
         // before releasing OsNatAgentTask instance, or we will crash.
         {
            SipRefreshMgr refreshMgr(NULL);
            SipLineMgr lineMgr(&refreshMgr);

            lineMgr.startLineMgr();

            SipUserAgent sipUA( 5090
                              ,5090
                              ,5091
                              ,"127.0.0.1"     // bind IP address
                              ,NULL     // default defaultUser
                              ,NULL     // default sipProxyServers
                              ,NULL     // default sipDirectoryServers
                              ,NULL     // default authenticationScheme
                              ,NULL     // default authenicateRealm
                              ,NULL     // default authenticateDb
                              ,NULL     // default authorizeUserIds
                              ,NULL     // default authorizePasswords
                              ,&lineMgr
                              );

            sipUA.start();
            refreshMgr.setSipUserAgent(&sipUA);

            sipUA.send(testMsg);

            // Wait long enough for some stack timeouts/retransmits to occur
            OsTask::delay(10000); // 10 seconds

            sipUA.shutdown(FALSE);
            lineMgr.requestShutdown();
            refreshMgr.requestShutdown();

            while(!sipUA.isShutdownDone())
            {
               ;
            }
            CPPUNIT_ASSERT(sipUA.isShutdownDone());
         }

         // Stop TimerTask and NatAgentTask again before counting threads.
         // They were started while testing.
         OsTimerTask::destroyTimerTask();
         OsNatAgentTask::releaseInstance();

         // Test to see that all the threads created by the above operations
         // get properly shut down.
         // Since the threads do not shut down synchronously with the above
         // calls, we have to wait before we know they will be cleared.
         OsTask::delay(1000);   // 1 second
         int numThreads = getNumThreads(myPID);
         CPPUNIT_ASSERT_EQUAL(startingThreads,numThreads);
      }
   };

};

CPPUNIT_TEST_SUITE_REGISTRATION(SipUserAgentTest);
