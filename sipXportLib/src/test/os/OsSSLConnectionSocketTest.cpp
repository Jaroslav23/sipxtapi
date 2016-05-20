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

#ifdef HAVE_SSL

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <string.h>
#include <stdlib.h>
#include <cstdarg>

#include <utl/UtlInt.h>
#include <utl/UtlString.h>
#include <os/OsTask.h>
#include <os/OsSSL.h>
#include <os/OsSocket.h>
#include <os/OsSSLServerSocket.h>
#include <os/OsSSLConnectionSocket.h>
#include <sipxunit/TestUtilities.h>

#define TEST_SERVER_SOCKET_PORT 15842
#define SOCKET_TIMEOUT 5000
#define SERVER_HELLO "Hello SSL socket"
#define CLIENT_HELLO "Hello SSL client"

using namespace std;

/**
 * Helper thread for accepting socket and sending some data into it.
 */
class TestServerBroker  : public OsTask
{
public:
   TestServerBroker(OsServerSocket* pSocket) : m_pSocket(pSocket)
   {
      start();
   }

   virtual ~TestServerBroker()
   {
      requestShutdown();
      waitUntilShutDown();
      if (m_pSocket)
      {
         m_pSocket->close();
         delete m_pSocket;
         m_pSocket = NULL;
      }
   }

   virtual int run(void* pArg)
   {
      OsConnectionSocket* clientSocket = NULL;

      while(m_pSocket && !isShuttingDown() && m_pSocket->isOk())
      {
         char buffer[300];
         memset(buffer, 0, sizeof(buffer));

         clientSocket = m_pSocket->accept();
         clientSocket->read(buffer, sizeof(buffer), SOCKET_TIMEOUT); // blocking read

         if (strcmp(buffer, SERVER_HELLO) == 0)
         {
            memset(buffer, 0, sizeof(buffer));
            strcpy(buffer, CLIENT_HELLO);
            clientSocket->write(buffer, strlen(buffer));
         }

         // close and delete socket
         clientSocket->close();
         delete clientSocket;
         clientSocket = NULL;
      }

      return 0;
   }
private:
   OsServerSocket* m_pSocket;
};

class OsSSLConnectionSocketTest : public CppUnit::TestCase
{

   CPPUNIT_TEST_SUITE(OsSSLConnectionSocketTest);
   CPPUNIT_TEST(testSSLConnection);
   CPPUNIT_TEST_SUITE_END();

private:

public:
   OsSSLConnectionSocketTest()
   {
   }

   void setUp()
   {
      // init SSL
      OsSSL::setCrtVerificationPolicy(OsSSL::SSL_ALWAYS_ACCEPT);
      OsSSL::getInstance();
   }

   void tearDown()
   {
   }

   ~OsSSLConnectionSocketTest()
   {
   }

   void testSSLConnection() 
   {
      OsSSLServerSocket* serverSocket = new OsSSLServerSocket(1, TEST_SERVER_SOCKET_PORT);
      TestServerBroker* serverBroker = new TestServerBroker(serverSocket);
      OsSSLConnectionSocket* clientSocket = new OsSSLConnectionSocket(TEST_SERVER_SOCKET_PORT, "localhost");
      clientSocket->makeBlocking();

      CPPUNIT_ASSERT(clientSocket->isConnected());
      CPPUNIT_ASSERT(clientSocket->isReadyToWrite(SOCKET_TIMEOUT));

      // send server hello
      const char* request = SERVER_HELLO;
      int res = clientSocket->write(request, strlen(request), 0);
      
      // read data from socket
      char readBuffer[300];
      memset(readBuffer, 0, sizeof(readBuffer));

      res = clientSocket->read(readBuffer, sizeof(readBuffer), SOCKET_TIMEOUT);
      if (strcmp(readBuffer, CLIENT_HELLO) != 0)
      {
         // expected message not receieved
         CPPUNIT_FAIL("Expected message was not received on SSL socket.");
      }

      // close socket, delete broker
      delete serverBroker;
      serverBroker = NULL;
      clientSocket->close();
      delete clientSocket;
      clientSocket = NULL;
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(OsSSLConnectionSocketTest);

#endif
