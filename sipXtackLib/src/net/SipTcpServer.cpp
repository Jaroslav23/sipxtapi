//  
// Copyright (C) 2007 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <net/SipTcpServer.h>
#include <net/SipUserAgent.h>
#include <os/OsDateTime.h>
#include <os/HostAdapterAddress.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/UtlPtr.h>
#include <net/SipServerBroker.h>
#include <net/SipServerBrokerListener.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
//#define TEST_PRINT
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipTcpServer::SipTcpServer(int port,
	  		   SipUserAgent* userAgent,
                           const char* protocolString, 
                           const char* taskName,
                           UtlBoolean bUseNextAvailablePort,
                           const char* szBindAddr) :
    SipProtocolServerBase(userAgent, protocolString, taskName)
{   
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipTcpServer::_ port = %d, taskName = '%s', bUseNextAvailablePort = %d, szBindAddr = '%s'",
                 port, taskName, bUseNextAvailablePort, szBindAddr);

   mServerPort = port ;
   mpServerBrokerListener = new SipServerBrokerListener(this);

#ifdef _DISABLE_MULTIPLE_INTERFACE_SUPPORT
   szBindAddr = "0.0.0.0" ;
#endif

    if (szBindAddr && 0 != strcmp(szBindAddr, "0.0.0.0"))
    {
        mDefaultIp = szBindAddr;
        createServerSocket(szBindAddr, mServerPort, bUseNextAvailablePort);
    }
    else
    {
        int numAddresses = MAX_IP_ADDRESSES;
        const HostAdapterAddress* adapterAddresses[MAX_IP_ADDRESSES];
        OsNetwork::getAllLocalHostIps(adapterAddresses, numAddresses);

        for (int i = 0; i < numAddresses; i++)
        {
            createServerSocket(adapterAddresses[i]->mAddress.data(),
                               mServerPort,
                               bUseNextAvailablePort);
            if (0 == i)
            {
                // use the first IP address in the array
                // for the 'default ip'
                mDefaultIp = adapterAddresses[i]->mAddress.data();
            }
            delete adapterAddresses[i];
            adapterAddresses[i] = NULL;
        }
    }

   mDefaultPort = SIP_PORT;

}

UtlBoolean SipTcpServer::startListener()
{
    UtlBoolean bRet(FALSE);
#       ifdef TEST_PRINT
        osPrintf("SIP Server binding to port %d\n", serverPort);
#       endif

    // iterate over the SipServerBroker map and call start
    UtlHashMapIterator iterator(mServerBrokers);
    UtlPtr<SipServerBroker>* pBrokerContainer = NULL;
    SipServerBroker* pBroker = NULL;
    UtlString* pKey = NULL;
    
    while((pKey = (UtlString*)iterator()))
    {
        pBrokerContainer = (UtlPtr<SipServerBroker>*) iterator.value();
        if (pBrokerContainer)
        {
            pBroker = pBrokerContainer->getValue();
            if (pBroker)
            {
                pBroker->start();
                bRet = TRUE;
            }
        }
    }
    return bRet;
}

OsStatus SipTcpServer::createServerSocket(const char* szBindAddr, int& port, const UtlBoolean& bUseNextAvailablePort)
{
    OsStatus rc = OS_FAILED;

    if (port != PORT_NONE)
    {
        OsServerSocket* pSocket = new OsServerSocket(64, port, szBindAddr);

        // If the socket is busy or unbindable and the user requested using the
        // next available port, try the next SIP_MAX_PORT_RANGE ports.
        if (pSocket && !pSocket->isOk() && bUseNextAvailablePort)
        {
            for (int i=1; i<=SIP_MAX_PORT_RANGE; i++)
            {
                delete pSocket;
                pSocket = new OsServerSocket(64, port+i);
                if (pSocket && pSocket->isOk())
                {
                    break ;
                }
            }
        }

        if (pSocket && pSocket->isOk())
        {
            port = pSocket->getLocalHostPort();

            UtlString adapterName;        
            OsNetwork::getAdapterName(adapterName, szBindAddr);

            SipContact sipContact(-1, SIP_CONTACT_LOCAL, SIP_TRANSPORT_TCP,
               szBindAddr, port, adapterName, szBindAddr);
            mSipUserAgent->addContact(sipContact);
       
            // add address and port to the maps. Socket object deletion is managed by SipServerBroker
            mServerSocketMap.insertKeyAndValue(new UtlString(szBindAddr),
                                               new UtlPtr<OsServerSocket>(pSocket, FALSE));
            mServerPortMap.insertKeyAndValue(new UtlString(szBindAddr),
                                                   new UtlInt(pSocket->getLocalHostPort()));
            mServerBrokers.insertKeyAndValue(new UtlString(szBindAddr),
                                              new UtlPtr<SipServerBroker>(new SipServerBroker((OsServerTask*)mpServerBrokerListener,
                                                                                               pSocket), FALSE));                                                   
        }

    }
    return rc;
}

// Copy constructor
SipTcpServer::SipTcpServer(const SipTcpServer& rSipTcpServer) :
    SipProtocolServerBase(NULL, SIP_TRANSPORT_TCP_STR, "SipTcpServer-%d")
{
}

// Destructor
SipTcpServer::~SipTcpServer()
{
    if (mpServerBrokerListener)
    {
        mpServerBrokerListener->requestShutdown();
        delete mpServerBrokerListener;
        mpServerBrokerListener = NULL;
    }
    waitUntilShutDown();
    {
        SipServerBroker* pBroker = NULL;
        UtlHashMapIterator iterator(mServerBrokers);
        UtlPtr<SipServerBroker>* pBrokerContainer = NULL;
        UtlString* pKey = NULL;
        
        while ((pKey = (UtlString*)iterator()))
        {
            pBrokerContainer = (UtlPtr<SipServerBroker>*)iterator.value();
            if (pBrokerContainer)
            {
                pBroker = pBrokerContainer->getValue();
                if (pBroker)
                {
                    delete pBroker;
                    pBroker = NULL;
                }
            }
        }
        mServerBrokers.destroyAll();
    }

    // socket objects are managed by SipServerBroker
    mServerSocketMap.destroyAll();
    mServerPortMap.destroyAll();
    
}

/* ============================ MANIPULATORS ============================== */

int SipTcpServer::run(void* runArgument)
{

    while (!isShuttingDown())
    {
        OsTask::delay(500); // this method really shouldn't do anything
    }

    return(0);
}

void SipTcpServer::shutdownListener()
{
    requestShutdown();
    shutdownClients();
}


OsSocket* SipTcpServer::buildClientSocket(int hostPort, const char* hostAddress, const char* localIp)
{
    // Create a socket in non-blocking mode while connecting
    OsConnectionSocket* socket = new OsConnectionSocket(hostPort, hostAddress, FALSE, localIp);
    if (socket)
    {
        socket->makeBlocking();
    }
    return(socket);
}

// Assignment operator
SipTcpServer&
SipTcpServer::operator=(const SipTcpServer& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */

// The the local server port for this server
int SipTcpServer::getServerPort() const 
{
    return mServerPort ;

}    

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


