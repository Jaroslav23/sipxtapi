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

// APPLICATION INCLUDES
#include <net/SipUdpServer.h>
#include <net/SipUserAgent.h>
#include <net/Url.h>
#include <os/OsDateTime.h>
#include <os/OsLock.h>
#include <os/OsNatAgentTask.h>
#include <os/OsNatDatagramSocket.h>
#include <os/HostAdapterAddress.h>
#include <os/OsTimer.h>
#include <os/OsCallback.h>
#include <os/OsPtrMsg.h>
#include <os/OsReadLock.h>
#include <os/OsWriteLock.h>
#include <os/OsTask.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlPtr.h>

#if defined(_VXWORKS)
#   include <socket.h>
#   include <resolvLib.h>
//#   include <netinet/ip.h>
#elif defined(__pingtel_on_posix__)
//#   include <netinet/tcp.h>
#   include <sys/types.h>
#   include <sys/socket.h>
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
//#define TEST_PRINT
//#define LOG_SIZE
// STATIC VARIABLE INITIALIZATIONS

class SipKeepAliveBinding
{
public:
    OsSocket*   m_pSocket ;
    UtlString   m_targetIp ;
    int         m_targetPort ;
    UtlString   m_method ;
    int         m_iKeepAlive ;
    OsTimer*    m_pTimer ;
    SipMessage* m_pSipMessage ;
    OsNatKeepaliveListener* m_pListener ;

    UtlString m_sendIp ;
    int       m_sendPort ;

    SipKeepAliveBinding(OsSocket*        pSocket,
                        const UtlString& targetIp,
                        int              targetPort,
                        const UtlString& method,
                        int              iKeepAlive,
                        OsNatKeepaliveListener* pListener)
    {
        m_pSocket = pSocket ;
        m_targetIp = targetIp ;
        m_targetPort = targetPort ;
        m_method = method ;
        m_iKeepAlive = iKeepAlive ;
        m_pTimer = NULL ;
        m_pSipMessage = NULL ;
        m_pListener = pListener ;

        m_sendIp = targetIp ; 
        m_sendPort = targetPort ;
    }

    OsNatKeepaliveEvent populateEvent(const char* szContactIp,
                                       const int   contactPort)
    {
        OsNatKeepaliveEvent event ;

        if (!m_method.compareTo("OPTIONS"))
            event.type = OS_NAT_KEEPALIVE_SIP_OPTIONS ;
        else if (!m_method.compareTo("PING"))
            event.type = OS_NAT_KEEPALIVE_SIP_PING ;        
        else
            event.type = OS_NAT_KEEPALIVE_INVALID ;

        event.remoteAddress = m_targetIp ;
        event.remotePort    = m_targetPort;
        event.keepAliveSecs = m_iKeepAlive ;
        event.mappedAddress = szContactIp ;
        event.mappedPort    = contactPort ;

        return event ;
    }

    void start(SipUdpServer* pServer) 
    {
        if (m_pTimer == NULL)
        {
            OsCallback* pCallback = new OsCallback((intptr_t) pServer, SipUdpServer::SipKeepAliveCallback) ;
            // timer manages callback deletion
            m_pTimer = new OsTimer(pCallback) ;        
        }

        // Fire off event
        if (m_pListener)            
            m_pListener->OnKeepaliveStart(populateEvent(NULL, PORT_NONE)) ;
        
        m_pTimer->periodicEvery(OsTime(0, 0), OsTime(m_iKeepAlive, 0)) ;
    }

    void fireFeedback(const char* szContactIp, int contactPort)
    {
        // Fire off event
        if (m_pListener)
            m_pListener->OnKeepaliveFeedback(populateEvent(szContactIp, contactPort)) ;
    }

    void fireFailure()
    {
        // Fire off event
        if (m_pListener)
            m_pListener->OnKeepaliveFailure(populateEvent(NULL, PORT_NONE)) ;
    }

    void stop()
    {
        if (m_pTimer)
        {
            m_pTimer->stop() ;

            // Fire off event
            if (m_pListener)                
                m_pListener->OnKeepaliveStop(populateEvent(NULL, PORT_NONE)) ;
        }
    }

    UtlBoolean matches(OsSocket*   pSocket,
                       const char* szRemoteIp,
                       const int   remotePort,
                       const char* szMethod) 
    {
        return ((pSocket == m_pSocket) &&
                (m_targetIp.compareTo(szRemoteIp) == 0) && 
                (remotePort == m_targetPort) &&
                (m_method.compareTo(szMethod) == 0)) ;
    }

    ~SipKeepAliveBinding()
    {
        stop() ;
        delete m_pTimer ;
        delete m_pSipMessage ;
    }
} ;



/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipUdpServer::SipUdpServer(int port,
                           SipUserAgent* userAgent,
                           int udpReadBufferSize,
                           UtlBoolean bUseNextAvailablePort,
                           const char* szBoundIp)
: SipProtocolServerBase(userAgent, "UDP", "SipUdpServer-%d")
, mSipCallIdGenerator("k")
, mStunRefreshSecs(28)
, mStunPort(PORT_NONE)
, mKeepAliveMutex(OsRWMutex::Q_FIFO)
, mMapLock(OsMutex::Q_FIFO)   
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipUdpServer::_ port = %d, bUseNextAvailablePort = %d, szBoundIp = '%s'",
                  port, bUseNextAvailablePort, szBoundIp);

#ifdef _DISABLE_MULTIPLE_INTERFACE_SUPPORT
    szBoundIp = "0.0.0.0" ;
#endif

    if (szBoundIp && 0 != strcmp(szBoundIp, "0.0.0.0"))
    {
        mDefaultIp = szBoundIp;
        int serverSocketPort = port;
        createServerSocket(szBoundIp, serverSocketPort, bUseNextAvailablePort, udpReadBufferSize);
    }
    else
    {
        int numAddresses = MAX_IP_ADDRESSES;
        const HostAdapterAddress* adapterAddresses[MAX_IP_ADDRESSES];
        OsNetwork::getAllLocalHostIps(adapterAddresses, numAddresses);

        for (int i = 0; i < numAddresses; i++)
        {
            int serverSocketPort = port;
            
            createServerSocket(adapterAddresses[i]->mAddress.data(),
                               serverSocketPort,
                               bUseNextAvailablePort,
                               udpReadBufferSize);
            if (0 == i)
            {
                // use the first IP address in the array
                // for the 'default ip'
                mDefaultIp = adapterAddresses[i]->mAddress.data();
            }
            delete adapterAddresses[i];   
        }
    }
}


// Destructor
SipUdpServer::~SipUdpServer()
{
    waitUntilShutDown();
    
    SipClient* pServer = NULL;
    UtlHashMapIterator iterator(mServers);
    UtlVoidPtr* pServerContainer = NULL;
    UtlString* pKey = NULL;
    
    while ((pKey = (UtlString*)iterator()))
    {
        pServerContainer = (UtlVoidPtr*)iterator.value();
        if (pServerContainer)
        {
            pServer = (SipClient*)pServerContainer->getValue();
            pServer->requestShutdown();
            delete pServer;
        }
    }

    {
       /*OsNatDatagramSocket* pSocket = NULL;
       UtlHashMapIterator iterator(mServerSocketMap);
       UtlPtr<OsNatDatagramSocket>* pSocketContainer = NULL;
       UtlString* pKey = NULL;

       while (pKey = (UtlString*)iterator())
       {
          pSocketContainer = (UtlPtr<OsNatDatagramSocket>*)iterator.value();
          if (pSocketContainer)
          {
             pSocket = pSocketContainer->getValue();
             if (pSocket)
             {
                delete pSocket;
                pSocket = NULL;
             }
          }
       }*/
       mServerSocketMap.destroyAll();
    }

    mServers.destroyAll();
    mServerPortMap.destroyAll();    
}

/* ============================ MANIPULATORS ============================== */

OsStatus SipUdpServer::createServerSocket(const char* szBoundIp,
                                          int& port,
                                          const UtlBoolean& bUseNextAvailablePort, 
                                          int udpReadBufferSize)
{
    OsStatus rc = OS_FAILED;
    OsNatDatagramSocket* pSocket =
      new OsNatDatagramSocket(0, NULL, port, szBoundIp, NULL);
   
    if (pSocket)
    {
        // If the socket is busy or unbindable and the user requested using the
        // next available port, try the next SIP_MAX_PORT_RANGE ports.
        if (bUseNextAvailablePort & portIsValid(port) && pSocket && !pSocket->isOk())
        {
            for (int i=1; i<=SIP_MAX_PORT_RANGE; i++)
            {
                delete pSocket ;
                pSocket = new OsNatDatagramSocket(0, NULL, port+i, szBoundIp, NULL);
                if (pSocket->isOk())
                {
                    break ;
                }
            }
        }
    }
    
    if (pSocket)
    {     
        pSocket->enableTransparentReads(false);  
        port = pSocket->getLocalHostPort();

        UtlString adapterName;        
        OsNetwork::getAdapterName(adapterName, szBoundIp);

        SipContact sipContact(-1, SIP_CONTACT_LOCAL, SIP_TRANSPORT_UDP,
           szBoundIp, port, adapterName, szBoundIp);
        mSipUserAgent->addContact(sipContact);
   
        // add address and port to the maps
        mServerSocketMap.insertKeyAndValue(new UtlString(szBoundIp),
                                            new UtlPtr<OsNatDatagramSocket>(pSocket));
        port = pSocket->getLocalHostPort() ;
        mServerPortMap.insertKeyAndValue(new UtlString(szBoundIp), new UtlInt(port));


        int sockbufsize = 0;
        int size = sizeof(int);
            getsockopt(pSocket->getSocketDescriptor(),
                    SOL_SOCKET,
                    SO_RCVBUF,
                    (char*)&sockbufsize,
        #if defined(__pingtel_on_posix__)
                    (socklen_t*) // caste
        #endif
                    &size);
        #ifdef LOG_SIZE
        OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipUdpServer::SipUdpServer UDP buffer size: %d size: %d\n",
                    sockbufsize, size);
        #endif /* LOG_SIZE */

        if(udpReadBufferSize > 0)
        {
            setsockopt(pSocket->getSocketDescriptor(),
            SOL_SOCKET,
            SO_RCVBUF,
            (char*)&udpReadBufferSize,
            sizeof(int));

            getsockopt(pSocket->getSocketDescriptor(),
                SOL_SOCKET,
                SO_RCVBUF,
                (char*)&sockbufsize,
        #if defined(__pingtel_on_posix__)
                (socklen_t*) // caste
        #endif
                &size);
        #ifdef LOG_SIZE
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipUdpServer::SipUdpServer reset UDP buffer size: %d size: %d\n",
                        sockbufsize, size);
        #endif /* LOG_SIZE */
        }
    }
    return rc;
}


int SipUdpServer::run(void* runArg)
            {


    return 0 ;
}


void SipUdpServer::enableStun(const char* szStunServer,
                              int iStunPort,
                              const char* szLocalIp, 
                              int refreshPeriodInSecs, 
                              OsMsgQ* pNotificationQueue) 
{
    OsLock lock(mMapLock);
    // Store settings
    mStunPort = iStunPort;
    mStunRefreshSecs = refreshPeriodInSecs;
    if (szStunServer)
    {
        mStunServer = szStunServer;
    }
    else
    {
        mStunServer.clear();
    }
    
    UtlBoolean bStunAll = FALSE;
    UtlHashMapIterator iterator(mServerSocketMap);
    UtlString* pMapKey = NULL;
    UtlString sIpToStun;
    
    if (szLocalIp)
    {
       sIpToStun.append(szLocalIp);
    }
    if (sIpToStun.isNull() || sIpToStun.compareTo("0.0.0.0") == 0)
    {
        bStunAll = true;
        // if no ip specified, start on the first one
        pMapKey = dynamic_cast<UtlString*>(iterator());
        if (pMapKey)
        {
           sIpToStun = *pMapKey;
        }
    }
    
    while (!sIpToStun.isNull())
    {
        const UtlPtr<OsNatDatagramSocket>* pSocketContainer = (UtlPtr<OsNatDatagramSocket>*)mServerSocketMap.findValue(&sIpToStun);
        
        if (pSocketContainer)
        {
           OsNatDatagramSocket* pSocket = pSocketContainer->getValue();
           if (pSocket)
           {
              pSocket->disableStun();
              // Update server client
              if (!mStunServer.isNull()) 
              {
                 pSocket->setStunNotificationQueue(pNotificationQueue);
                 pSocket->enableStun(mStunServer, mStunPort, refreshPeriodInSecs, 0, false);
              }
           }
        }
        if (bStunAll)
        {
            // get the next address to stun
            pMapKey = dynamic_cast<UtlString*>(iterator());
            if (pMapKey)
            {
                sIpToStun = *pMapKey;
            }
            else
            {
                break; // no more server sockets
            }
        }
        else
        {
            break;
        }
    } // end while
}

UtlBoolean SipUdpServer::startListener()
{
#       ifdef TEST_PRINT
   osPrintf("SIP Server binding to port %d\n", serverPort);
#       endif

   UtlHashMapIterator iter(mServerSocketMap);
   UtlPtr<OsNatDatagramSocket>* pSocketContainer = NULL;
   UtlString* pKey;
   while ((pKey =(UtlString*)iter()))
   {
      OsNatDatagramSocket* pSocket = NULL;
      SipClient* pServer = NULL;
      UtlVoidPtr* pServerContainer = NULL;

      UtlString localIp = *pKey;
      pSocketContainer = (UtlPtr<OsNatDatagramSocket>*)iter.value();

      if (pSocketContainer)
      {    
         pSocket = pSocketContainer->getValue();
      }

      pServerContainer = (UtlVoidPtr*)mServers.findValue(&localIp);
      if (!pServerContainer)
      {
         pServer = new SipClient(pSocket);

         // This used to be done at the end of this else statement
         // however there is a race and the userAgent must be set before
         // starting this client.  I think the race occurs if there is
         // immediately an incoming message on the socket.
         if(mSipUserAgent)
         {
            if (pServer)
            {
               pServer->setUserAgent(mSipUserAgent);
            }
         }
         this->mServers.insertKeyAndValue(new UtlString(localIp), new UtlVoidPtr((void*)pServer));
         pServer->start();
      }
      else
      {
         pServer = (SipClient*) pServerContainer->getValue();
         if(mSipUserAgent)
         {
            if (pServer)
            {
               pServer->setUserAgent(mSipUserAgent);
            }
         }
      }
   }
   return(TRUE);
}

void SipUdpServer::shutdownListener()
{
    OsLock lock(mMapLock);
    SipClient* pServer = NULL;
    UtlHashMapIterator iterator(mServers);
    UtlVoidPtr* pServerContainer = NULL;
    UtlString* pKey = NULL;
    
    while ((pKey = (UtlString*)iterator()))
    {
        pServerContainer = (UtlVoidPtr*) iterator.value();
        pServer = (SipClient*)pServerContainer->getValue();
        if (pServer)
        {
            pServer->requestShutdown();
        }
    }
}


UtlBoolean SipUdpServer::sendTo(const SipMessage& message,
                               const char* address,
                               int port,
                               const char* szLocalSipIp)
{
    UtlBoolean sendOk;
    UtlVoidPtr* pServerContainer = NULL;
    SipClient* pServer = NULL;
    
    if (szLocalSipIp)
    {
        UtlString localKey(szLocalSipIp);
        pServerContainer = (UtlVoidPtr*)this->mServers.findValue(&localKey);
        if (pServerContainer)
        {
            pServer = (SipClient*) pServerContainer->getValue();
        }
    }
    else
    {
        // no local sip IP specified, so, use the default one
        UtlString defaultKey(mDefaultIp);
       
        pServerContainer = (UtlVoidPtr*) mServers.findValue(&defaultKey);
        if (pServerContainer)
        {
            pServer = (SipClient*) pServerContainer->getValue();
        }
    }
    
    if (pServer)
    {
        sendOk = pServer->sendTo(message, address, port);
    }
    else
    {
        sendOk = false;
    }
    return(sendOk);
}


OsSocket* SipUdpServer::buildClientSocket(int hostPort, const char* hostAddress, const char* localIp)
{
    OsNatDatagramSocket* pSocket = NULL;
    UtlPtr<OsNatDatagramSocket>* pSocketContainer = NULL;

    assert(localIp != NULL);
    UtlString sLocalIp(localIp);
     
    pSocketContainer = (UtlPtr<OsNatDatagramSocket>*)mServerSocketMap.findValue(&sLocalIp);
    assert(pSocketContainer);
     
    pSocket = pSocketContainer->getValue();
    assert(pSocket);

    return pSocket ;
}


UtlBoolean SipUdpServer::addCrLfKeepAlive(const char* szLocalIp,
                                          const char* szRemoteIp,
                                          const int   remotePort,
                                          const int   keepAliveSecs,
                                          OsNatKeepaliveListener* pListener) 
{
    OsLock lock(mMapLock);
    UtlHashMapIterator iterator(mServerSocketMap);
    UtlString* pKey = NULL;
    OsNatDatagramSocket* pSocket = NULL;
    UtlBoolean bSuccess = false ;

    while (pKey = (UtlString*) iterator())
    {
        if (    (pKey->compareTo(szLocalIp) == 0) || 
                (strcmp(szLocalIp, "0.0.0.0") == 0) || 
                (strlen(szLocalIp) == 0)    )
        {
            UtlPtr<OsNatDatagramSocket>* pValue = (UtlPtr<OsNatDatagramSocket>*) mServerSocketMap.findValue(pKey) ;
            if (pValue)
            {
                pSocket = pValue->getValue();
                if (pSocket)
                {
                    if (pSocket->addCrLfKeepAlive(szRemoteIp, remotePort, 
                            keepAliveSecs, pListener))
                    {
                        bSuccess = true ;
                    }
                }
            }
        }
    }

    return bSuccess ;
}


UtlBoolean SipUdpServer::removeCrLfKeepAlive(const char* szLocalIp,
                                             const char* szRemoteIp,
                                             const int   remotePort) 
{
    OsLock lock(mMapLock);
    UtlHashMapIterator iterator(mServerSocketMap);
    UtlString* pKey = NULL;
    OsNatDatagramSocket* pSocket = NULL;
    UtlBoolean bSuccess = false ;

    while (pKey = (UtlString*) iterator())
    {
        if (    (pKey->compareTo(szLocalIp) == 0) || 
                (strcmp(szLocalIp, "0.0.0.0") == 0) || 
                (strlen(szLocalIp) == 0)    )
        {
            UtlPtr<OsNatDatagramSocket>* pValue = (UtlPtr<OsNatDatagramSocket>*) mServerSocketMap.findValue(pKey) ;
            if (pValue)
            {
                pSocket = pValue->getValue();
                if (pSocket)
                {
                    if (pSocket->removeCrLfKeepAlive(szRemoteIp, remotePort))
                    {
                        bSuccess = true ;
                    }
                }
            }
        }
    }

    return bSuccess ;
}

UtlBoolean SipUdpServer::addStunKeepAlive(const char* szLocalIp,
                                          const char* szRemoteIp,
                                          const int   remotePort,
                                          const int   keepAliveSecs,
                                          OsNatKeepaliveListener* pListener) 
{
    OsLock lock(mMapLock);
    UtlHashMapIterator iterator(mServerSocketMap);
    UtlString* pKey = NULL;
    OsNatDatagramSocket* pSocket = NULL;
    UtlBoolean bSuccess = false ;

    while (pKey = (UtlString*) iterator())
    {
        if (    (pKey->compareTo(szLocalIp) == 0) || 
                (strcmp(szLocalIp, "0.0.0.0") == 0) || 
                (strlen(szLocalIp) == 0)    )
        {
            UtlPtr<OsNatDatagramSocket>* pValue = (UtlPtr<OsNatDatagramSocket>*) mServerSocketMap.findValue(pKey) ;
            if (pValue)
            {
                pSocket = pValue->getValue();
                if (pSocket)
                {
                    if (pSocket->addStunKeepAlive(szRemoteIp, remotePort, 
                            keepAliveSecs, pListener))
                    {
                        bSuccess = true ;
                    }
                }
            }
        }
    }

    return bSuccess ;
 }


UtlBoolean SipUdpServer::removeStunKeepAlive(const char* szLocalIp,
                                             const char* szRemoteIp,
                                             const int   remotePort)
{
    OsLock lock(mMapLock);
    UtlHashMapIterator iterator(mServerSocketMap);
    UtlString* pKey = NULL;
    OsNatDatagramSocket* pSocket = NULL;
    UtlBoolean bSuccess = false ;

    while (pKey = (UtlString*) iterator())
    {
        if (    (pKey->compareTo(szLocalIp) == 0) || 
                (strcmp(szLocalIp, "0.0.0.0") == 0) || 
                (strlen(szLocalIp) == 0)    )
        {
            UtlPtr<OsNatDatagramSocket>* pValue = (UtlPtr<OsNatDatagramSocket>*) mServerSocketMap.findValue(pKey) ;
            if (pValue)
            {
                pSocket = pValue->getValue();
                if (pSocket)
                {
                    if (pSocket->removeStunKeepAlive(szRemoteIp, remotePort))
                    {
                        bSuccess = true ;
                    }
                }
            }
        }
    }

    return bSuccess ;
}

UtlBoolean SipUdpServer::addSipKeepAlive(const char* szLocalIp,
                                         const char* szRemoteIp,
                                         const int   remotePort,
                                         const char* szMethod,
                                         const int   keepAliveSecs,
                                         OsNatKeepaliveListener* pListener)
{
    UtlHashMapIterator iterator(mServerSocketMap);
    UtlString* pKey = NULL;
    OsNatDatagramSocket* pSocket = NULL;
    UtlBoolean bSuccess = false ;
        

    while (pKey = (UtlString*) iterator())
    {
        if (    (pKey->compareTo(szLocalIp) == 0) || 
                (strcmp(szLocalIp, "0.0.0.0") == 0) || 
                (strlen(szLocalIp) == 0)    )
        {
            UtlPtr<OsNatDatagramSocket>* pValue = (UtlPtr<OsNatDatagramSocket>*) mServerSocketMap.findValue(pKey) ;
            if (pValue)
            {
                pSocket = pValue->getValue();
                if (pSocket)
                {
                    SipKeepAliveBinding* pBinding = NULL ;                    
                    mKeepAliveMutex.acquireWrite() ;
                    pBinding = (SipKeepAliveBinding*) findKeepAliveBinding(
                            pSocket, szRemoteIp, remotePort, szMethod) ;

                    if (pBinding == NULL)
                    {
                        pBinding = new SipKeepAliveBinding(pSocket,
                                szRemoteIp, remotePort, szMethod, keepAliveSecs,
                                pListener) ;                        

                        bSuccess = addKeepAliveBinding(pBinding) ;
                    }
                    else
                    {
                        // If we found a binding, stop it to avoid multiple
                        // timers.
                        pBinding->stop() ;
                    }
                    mKeepAliveMutex.releaseWrite() ;

                    if (bSuccess && pBinding)
                    {
                        OsNatAgentTask::getInstance()->addExternalBinding(
                                NULL, szRemoteIp, remotePort, "", 0) ;

                        pBinding->start(this) ;
                    }
                }
            }
        }
    }

    return bSuccess ;
}

UtlBoolean SipUdpServer::removeSipKeepAlive(const char* szLocalIp,
                                            const char* szRemoteIp,
                                            const int   remotePort,
                                            const char* szMethod) 
{
    UtlHashMapIterator iterator(mServerSocketMap);
    UtlString* pKey = NULL;
    OsNatDatagramSocket* pSocket = NULL;
    UtlBoolean bSuccess = false ;

    while (pKey = (UtlString*) iterator())
    {
        if (    (pKey->compareTo(szLocalIp) == 0) || 
                (strcmp(szLocalIp, "0.0.0.0") == 0) || 
                (strlen(szLocalIp) == 0)    )
        {
            UtlPtr<OsNatDatagramSocket>* pValue = (UtlPtr<OsNatDatagramSocket>*) mServerSocketMap.findValue(pKey) ;
            if (pValue)
            {
                pSocket = pValue->getValue();
                if (pSocket)
                {
                    SipKeepAliveBinding* pBinding = NULL ;                    
                    mKeepAliveMutex.acquireWrite() ;
                    pBinding = (SipKeepAliveBinding*) findKeepAliveBinding(
                            pSocket, szRemoteIp, remotePort, szMethod) ;
                    if (pBinding)
                    {
                        bSuccess = removeKeepAliveBinding(pBinding) ;
                    }
                    mKeepAliveMutex.releaseWrite() ;

                    // Stop/Delete binding will stop timer
                    delete pBinding ;
                }
            }
        }
    }

    return bSuccess ;
}

void SipUdpServer::updateSipKeepAlive(const char* szLocalIp,
                                      const char* szMethod,
                                      const char* szRemoteIp,
                                      const int   remotePort,
                                      const char* szContactIp,
                                      const int   contactPort) 
{
    UtlHashMapIterator iterator(mServerSocketMap);
    UtlString* pKey = NULL;
    OsNatDatagramSocket* pSocket = NULL;

    while (pKey = (UtlString*) iterator())
    {
        if (    (pKey->compareTo(szLocalIp) == 0) || 
                (strcmp(szLocalIp, "0.0.0.0") == 0) || 
                (strlen(szLocalIp) == 0)    )
        {
            UtlPtr<OsNatDatagramSocket>* pValue = (UtlPtr<OsNatDatagramSocket>*) mServerSocketMap.findValue(pKey) ;
            if (pValue)
            {
                pSocket = pValue->getValue();
                if (pSocket)
                {
                    SipKeepAliveBinding* pBinding = NULL ;                    
                    mKeepAliveMutex.acquireRead() ;
                    pBinding = (SipKeepAliveBinding*) findKeepAliveBinding(
                            pSocket, szRemoteIp, remotePort, szMethod) ;
                    if (pBinding)
                    {
                        pBinding->fireFeedback(szContactIp, contactPort) ;                        
                    }
                    mKeepAliveMutex.releaseRead() ;                                        
                }
            }
        }
    }
}


/* ============================ ACCESSORS ================================= */

void SipUdpServer::printStatus()
{
    OsLock lock(mMapLock);
    SipClient* pServer = NULL;
    UtlHashMapIterator iterator(mServers);
    UtlVoidPtr* pServerContainer = NULL;
    UtlString* pKey = NULL;
    
    while ((pKey = (UtlString*)iterator()))
    {
        pServerContainer = (UtlVoidPtr*) iterator.value();
        if (pServerContainer)
        {
            pServer = (SipClient*)pServerContainer->getValue();
        }
        if (pServer)
        {
            UtlString clientNames;
            long clientTouchedTime = pServer->getLastTouchedTime();
            UtlBoolean clientOk = pServer->isOk();
            pServer->getClientNames(clientNames);
            osPrintf("UDP server %p last used: %ld ok: %d names: \n%s \n",
                this, clientTouchedTime, clientOk, clientNames.data());

            SipProtocolServerBase::printStatus();
        }
    }
}

int SipUdpServer::getServerPort(const char* szLocalIp) 
{
    int port = PORT_NONE;

    char szLocalIpForPortLookup[256];
    memset((void*)szLocalIpForPortLookup, 0, sizeof(szLocalIpForPortLookup));
    
    if (NULL == szLocalIp)
    {
        strcpy(szLocalIpForPortLookup, mDefaultIp);
    }
    else
    {
        strcpy(szLocalIpForPortLookup, szLocalIp);
    }
    
    UtlString localIpKey(szLocalIpForPortLookup);
    
    UtlInt* pUtlPort;
    pUtlPort = (UtlInt*)this->mServerPortMap.findValue(&localIpKey);
    if (pUtlPort)
    {
       port = pUtlPort->getValue();
    }
    
    return port ;
}


UtlBoolean SipUdpServer::getStunAddress(UtlString* pIpAddress, int* pPort,
                                        const char* szLocalIp) 
{
    UtlBoolean bRet = false;
    OsNatDatagramSocket* pSocket = NULL;
    UtlPtr<OsNatDatagramSocket>* pSocketContainer = NULL;

    if (szLocalIp)
    {
        UtlString localIpKey(szLocalIp);
       
        pSocketContainer = (UtlPtr<OsNatDatagramSocket>*)this->mServerSocketMap.findValue(&localIpKey);
        if (pSocketContainer)
        {
            pSocket = pSocketContainer->getValue();
        }
    }
    else
    {
        // just use the default Socket in our collection
        UtlString defaultIpKey(mDefaultIp);
       
        pSocketContainer = (UtlPtr<OsNatDatagramSocket>*)mServerSocketMap.findValue(&defaultIpKey);
        if (pSocketContainer != NULL )
        {
            pSocket = pSocketContainer->getValue();
        }
    }
    
    if (pSocket)
    {
        bRet =  pSocket->getMappedIp(pIpAddress, pPort) ;
    }
    return bRet;
}


UtlBoolean SipUdpServer::addKeepAliveBinding(void* pBinding) 
{
    UtlVoidPtr key(pBinding) ;
    UtlBoolean bSuccess = false ;    

    if (mSipKeepAliveBindings.find(&key) == NULL)
    {
        if (mSipKeepAliveBindings.insert(new UtlVoidPtr(pBinding)) != NULL)
        {
            bSuccess = true ;
        }
    }

    return bSuccess ;
}


UtlBoolean SipUdpServer::removeKeepAliveBinding(void* pBinding) 
{
    UtlVoidPtr key(pBinding) ;
    UtlBoolean bSuccess ;    

    bSuccess = mSipKeepAliveBindings.destroy(&key) ;

    return bSuccess ;
}


void* SipUdpServer::findKeepAliveBinding(OsTimer* pTimer) 
{
    void* pRC = NULL ;

    UtlSListIterator itor(mSipKeepAliveBindings) ;
    while (UtlContainable* pCont = itor())
    {
        SipKeepAliveBinding* pBinding = (SipKeepAliveBinding*) 
                ((UtlVoidPtr*) pCont)->getValue() ;
        if (pBinding->m_pTimer == pTimer)
        {
            pRC = pBinding ;
            break ;
        }
    }

    return pRC ;
}

void* SipUdpServer::findKeepAliveBinding(OsSocket*   pSocket,
                                         const char* szRemoteIp,
                                         const int   remotePort,
                                         const char* szMethod) 
{
    void* pRC = NULL ;

    UtlSListIterator itor(mSipKeepAliveBindings) ;
    while (UtlContainable* pCont = itor())
    {
        SipKeepAliveBinding* pBinding = (SipKeepAliveBinding*) 
                ((UtlVoidPtr*) pCont)->getValue() ;

        if (pBinding->matches(pSocket, szRemoteIp, remotePort, szMethod))
        {
            pRC = pBinding ;
            break ;
        }
    }

    return pRC ;
}



void SipUdpServer::sendSipKeepAlive(OsTimer* pTimer) 
{    
    assert(pTimer) ;

    if (pTimer)
    {
        SipKeepAliveBinding* pBinding ;

        OsReadLock lock(mKeepAliveMutex) ;
        if (pBinding = (SipKeepAliveBinding*) 
                findKeepAliveBinding(pTimer))
        {
            if (pBinding->m_pSipMessage == NULL)
            {
                pBinding->m_pSipMessage = new SipMessage() ;

                // Setup From Field
                UtlString from;
                mSipUserAgent->getDefaultContactUri(&from);
                char fromTag[80];
                SNPRINTF(fromTag, sizeof(fromTag), ";tag=%d%d", rand(), rand());
                from.append(fromTag);

                // Setup To Field
                Url toUrl(NULL, TRUE);
                toUrl.setScheme(Url::SipUrlScheme) ;
                toUrl.setHostAddress(pBinding->m_targetIp) ;
                if (pBinding->m_targetPort != SIP_PORT)
                {
                    toUrl.setHostPort(pBinding->m_targetPort) ;
                }
                UtlString to = toUrl.toString();


                // Resolve the raw address from a DNS SRV, A record
                // to an IP address
                server_t* dnsSrvRecords = SipSrvLookup::servers(
                        pBinding->m_targetIp, "sip", OsSocket::UDP, 
                        pBinding->m_targetPort, mDefaultIp);

                if(dnsSrvRecords[0].isValidServerT())
                {
                    // Note: we are not doing any failover here as that is
                    // a little tricky with the NAT stuff.  We cannot change
                    // addresses with every transaction as we may get different
                    // ports and addresses every time we send a ping.  For now
                    // we do one DNS SRV lookup at the begining of time and
                    // stick to that result.
                    dnsSrvRecords[0].getIpAddressFromServerT(pBinding->m_sendIp);
                    pBinding->m_sendPort = dnsSrvRecords[0].getPortFromServerT();

                    // If the ping URL or DNS SRV did not specify a port
                    // bind it to the default port.
                    if (!portIsValid(pBinding->m_sendPort))
                    {
                        pBinding->m_sendPort = SIP_PORT;
                    }
                }
                else
                {
                    // Log failure and kill timer
                }

                delete[] dnsSrvRecords;

                // Generate CallId
                UtlString callId(mSipCallIdGenerator.getNewCallId());

                pBinding->m_pSipMessage->setRequestData(pBinding->m_method, 
                        to, from, to, callId, rand() % 32768);

                // Set User Agent header
                mSipUserAgent->setUserAgentHeader(*pBinding->m_pSipMessage) ;
            }
            
            // Update Via                           
            if (!mSipUserAgent->sendSymmetricUdp(
                    *pBinding->m_pSipMessage, pBinding->m_sendIp, 
                    pBinding->m_sendPort))
            {
                pBinding->fireFailure() ;
            }
            pBinding->m_pSipMessage->incrementCSeqNumber() ;
        }
    }
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

void SipUdpServer::SipKeepAliveCallback(const int userData, 
                                        const int eventData)
{  
    SipUdpServer* pUdpServer = (SipUdpServer*) userData ;
    OsTimer* pTimer = (OsTimer*) eventData ;

    assert(pUdpServer) ;
    assert(pTimer) ;

    if (pUdpServer && pTimer)
    {
        OsPtrMsg msg(OsMsg::PHONE_APP, SipUserAgent::KEEPALIVE_MESSAGE, pUdpServer, pTimer) ;
        pUdpServer->mSipUserAgent->postMessage(msg) ;
    }     
}

