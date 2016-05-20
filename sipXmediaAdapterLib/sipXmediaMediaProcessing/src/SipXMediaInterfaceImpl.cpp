// 
// Copyright (C) 2005-2007 SIPez LLC.
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

// Author: Dan Petrie (dpetrie AT SIPez DOT com)
 
// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <utl/UtlDListIterator.h>
#include <utl/UtlHashMap.h>
#include <os/OsDatagramSocket.h>
#include <os/OsNatDatagramSocket.h>
#include <os/OsMulticastSocket.h>
#include <os/HostAdapterAddress.h>
#include <os/OsNetwork.h>
#include "include/SipXMediaInterfaceImpl.h"
#include "mi/CpMediaInterfaceFactory.h"
#include <mp/MpMediaTask.h>
#include <mp/MpCallFlowGraph.h>
#include <mp/dtmflib.h>
#include <sdp/SdpCodec.h>

#if defined(_VXWORKS)
#   include <socket.h>
#   include <resolvLib.h>
#   include <netinet/ip.h>
#elif defined(__pingtel_on_posix__)
#   include <netinet/in.h>
#   include <netinet/tcp.h>
#   include <sys/types.h>
#   include <sys/socket.h>
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define MINIMUM_DTMF_LENGTH 60
#define MAX_RTP_PORTS 1000

//#define TEST_PRINT

// STATIC VARIABLE INITIALIZATIONS

class SipXMediaConnection : public UtlInt
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    SipXMediaConnection(int connectionId = -1)
    : UtlInt(connectionId)
    , mRtpSendHostAddress()
    , mDestinationSet(FALSE)
    , mIsMulticast(FALSE)
    , mpRtpAudioSocket(NULL)
    , mpRtcpAudioSocket(NULL)
    , mRtpAudioSendHostPort(0)
    , mRtcpAudioSendHostPort(0)
    , mRtpAudioReceivePort(0)
    , mRtcpAudioReceivePort(0)
    , mRtpAudioSending(FALSE)
    , mRtpAudioReceiving(FALSE)
    , mpAudioCodec(NULL)
    , mpSdpCodecList(NULL)
    , mContactType(SIP_CONTACT_AUTO)
    , mbAlternateDestinations(FALSE)
    {
    };

    virtual ~SipXMediaConnection()
    {
        if(mpRtpAudioSocket)
        {

#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG, 
                "~SipXMediaConnection deleting RTP socket: %p descriptor: %d",
                mpRtpAudioSocket, mpRtpAudioSocket->getSocketDescriptor());
#endif
            delete mpRtpAudioSocket;
            mpRtpAudioSocket = NULL;
        }

        if(mpRtcpAudioSocket)
        {
#ifdef TEST_PRINT
            OsSysLog::add(FAC_CP, PRI_DEBUG, 
                "~SipXMediaConnection deleting RTCP socket: %p descriptor: %d",
                mpRtcpAudioSocket, mpRtcpAudioSocket->getSocketDescriptor());
#endif
            delete mpRtcpAudioSocket;
            mpRtcpAudioSocket = NULL;
        }

        if(mpSdpCodecList)
        {
            OsSysLog::add(FAC_CP, PRI_DEBUG, 
                "~SipXMediaConnection deleting SdpCodecList %p",
                mpSdpCodecList);
            delete mpSdpCodecList;
            mpSdpCodecList = NULL;
        }

        if (mpAudioCodec)
        {
            delete mpAudioCodec;
            mpAudioCodec = NULL; 
        }              
    }

    UtlString mRtpSendHostAddress;
    UtlBoolean mDestinationSet;
    UtlBoolean mIsMulticast;
    OsDatagramSocket* mpRtpAudioSocket;
    OsDatagramSocket* mpRtcpAudioSocket;
    int mRtpAudioSendHostPort;
    int mRtcpAudioSendHostPort;
    int mRtpAudioReceivePort;
    int mRtcpAudioReceivePort;
    UtlBoolean mRtpAudioSending;
    UtlBoolean mRtpAudioReceiving;
    SdpCodec* mpAudioCodec;
    SdpCodecList* mpSdpCodecList; /// list of SDP codecs used for SDP negotiation, but not up to date when RTP is flowing
    SIP_CONTACT_TYPE mContactType ;
    UtlString mLocalIPAddress; ///< local bind IP address
    UtlBoolean mbAlternateDestinations ;
};

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipXMediaInterfaceImpl::SipXMediaInterfaceImpl(CpMediaInterfaceFactory* pFactoryImpl,
											            OsMsgQ* pInterfaceNotificationQueue,///< queue for sending interface notifications
                                             const SdpCodecList* pCodecList,///< list of SdpCodec instances
                                             const char* publicIPAddress,///< ignored
                                             const char* localIPAddress,///< local bind IP address
                                             const char* locale,///< locale for tone generator
                                             int expeditedIpTos,
                                             const char* szStunServer,
                                             int iStunPort,
                                             int iStunKeepAlivePeriodSecs,
                                             const char* szTurnServer,
                                             int iTurnPort,
                                             const char* szTurnUsername,
                                             const char* szTurnPassword,
                                             int iTurnKeepAlivePeriodSecs,
                                             UtlBoolean bEnableICE)
: CpMediaInterface(pFactoryImpl)
, m_pInterfaceNotificationQueue(pInterfaceNotificationQueue)
{
   OsSysLog::add(FAC_CP, PRI_DEBUG, "SipXMediaInterfaceImpl::SipXMediaInterfaceImpl creating a new CpMediaInterface %p",
                 this);

   mpFlowGraph = new MpCallFlowGraph(locale, pInterfaceNotificationQueue);
   OsSysLog::add(FAC_CP, PRI_DEBUG, "SipXMediaInterfaceImpl::SipXMediaInterfaceImpl creating a new MpCallFlowGraph %p",
                 mpFlowGraph);
   
   mStunServer = szStunServer ;
   mStunPort = iStunPort ;
   mStunRefreshPeriodSecs = iStunKeepAlivePeriodSecs ;
   mTurnServer = szTurnServer ;
   mTurnPort = iTurnPort ;
   mTurnRefreshPeriodSecs = iTurnKeepAlivePeriodSecs ;
   mTurnUsername = szTurnUsername ;
   mTurnPassword = szTurnPassword ;
   mEnableIce = bEnableICE ;

   if(localIPAddress && *localIPAddress)
   {
       mRtpReceiveHostAddress = localIPAddress;
       mLocalIPAddress = localIPAddress;
   }
   else
   {
       OsSocket::getHostIp(&mLocalIPAddress);
       mRtpReceiveHostAddress = mLocalIPAddress;
   }

   if(pCodecList && pCodecList->getCodecCount() > 0)
   {
       mSdpCodecList.addCodecs(*pCodecList);
       // Assign any unset payload types
       mSdpCodecList.bindPayloadIds();
   }
   else
   {
      if (pFactoryImpl)
      {
         pFactoryImpl->buildAllCodecList(mSdpCodecList);
      }
   }

   mExpeditedIpTos = expeditedIpTos;
}


// Destructor
SipXMediaInterfaceImpl::~SipXMediaInterfaceImpl()
{
   OsSysLog::add(FAC_CP, PRI_DEBUG, "SipXMediaInterfaceImpl::~SipXMediaInterfaceImpl deleting the CpMediaInterface %p",
                 this);

    SipXMediaConnection* mediaConnection = NULL;
    while ((mediaConnection = (SipXMediaConnection*) mMediaConnections.get()))
    {
        doDeleteConnection(mediaConnection);
        delete mediaConnection;
        mediaConnection = NULL;
    }

    if(mpFlowGraph)
    {
       // Free up the resources used by tone generation ASAP
       stopTone();

        // Stop the net in/out stuff before the sockets are deleted
        //mpMediaFlowGraph->stopReceiveRtp();
        //mpMediaFlowGraph->stopSendRtp();

        MpMediaTask* mediaTask = MpMediaTask::getMediaTask();

        // take focus away from the flow graph if it is focus
        if(mpFlowGraph == (MpCallFlowGraph*) mediaTask->getFocus())
        {
            mediaTask->setFocus(NULL);
        }

        OsSysLog::add(FAC_CP, PRI_DEBUG, "SipXMediaInterfaceImpl::~SipXMediaInterfaceImpl deleting the MpCallFlowGraph %p",
                      mpFlowGraph);
        delete mpFlowGraph;
        mpFlowGraph = NULL;
    }
}

/**
 * public interface for destroying this media interface
 */ 
void SipXMediaInterfaceImpl::release()
{
   delete this;
}

/* ============================ MANIPULATORS ============================== */

OsStatus SipXMediaInterfaceImpl::createConnection(int& connectionId,
                                                 const char* szLocalAddress,
                                                 int localPort,
                                                 void* videoWindowHandle, 
                                                 void* const pSecurityAttributes,
                                                 OsMsgQ* pConnectionNotificationQueue,
                                                 const RtpTransportOptions rtpTransportOptions)
{
   OsStatus retValue = OS_SUCCESS;
   SipXMediaConnection* mediaConnection=NULL;

   connectionId = mpFlowGraph->createConnection(pConnectionNotificationQueue);
   if (connectionId == -1)
   {
      return OS_LIMIT_REACHED;
   }


   mediaConnection = new SipXMediaConnection(connectionId);
   OsSysLog::add(FAC_CP, PRI_DEBUG,
                 "SipXMediaInterfaceImpl::createConnection "
                 "creating a new connection %d (%p)",
                 connectionId, mediaConnection);
   mMediaConnections.append(mediaConnection);

   // Set Local address
   if (szLocalAddress && strlen(szLocalAddress))
   {
      mediaConnection->mLocalIPAddress = szLocalAddress;
   }
   else
   {
      mediaConnection->mLocalIPAddress = mLocalIPAddress;
   }

   mediaConnection->mIsMulticast = OsSocket::isMcastAddr(mediaConnection->mLocalIPAddress);
   if (mediaConnection->mIsMulticast)
   {
      mediaConnection->mContactType = SIP_CONTACT_LOCAL;
   }

   // Create the sockets for audio stream
   retValue = createRtpSocketPair(mediaConnection->mLocalIPAddress, localPort,
                                  mediaConnection->mContactType,
                                  mediaConnection->mpRtpAudioSocket, mediaConnection->mpRtcpAudioSocket);
   if (retValue != OS_SUCCESS)
   {
       return retValue;
   }

   // Start the audio packet pump
   mpFlowGraph->startReceiveRtp(SdpCodecList(), // pass empty list
                                *mediaConnection->mpRtpAudioSocket,
                                *mediaConnection->mpRtcpAudioSocket,
                                connectionId);

   // Store audio stream settings
   mediaConnection->mRtpAudioReceivePort = mediaConnection->mpRtpAudioSocket->getLocalHostPort() ;
   mediaConnection->mRtcpAudioReceivePort = mediaConnection->mpRtcpAudioSocket->getLocalHostPort() ;

   OsSysLog::add(FAC_CP, PRI_DEBUG, 
            "SipXMediaInterfaceImpl::createConnection creating a new RTP socket: %p descriptor: %d",
            mediaConnection->mpRtpAudioSocket, mediaConnection->mpRtpAudioSocket->getSocketDescriptor());
   OsSysLog::add(FAC_CP, PRI_DEBUG, 
            "SipXMediaInterfaceImpl::createConnection creating a new RTCP socket: %p descriptor: %d",
            mediaConnection->mpRtcpAudioSocket, mediaConnection->mpRtcpAudioSocket->getSocketDescriptor());

   // Set codec factory
   mediaConnection->mpSdpCodecList = new SdpCodecList(mSdpCodecList);
   mediaConnection->mpSdpCodecList->bindPayloadIds();
   OsSysLog::add(FAC_CP, PRI_DEBUG, 
            "SipXMediaInterfaceImpl::createConnection creating a new SdpCodecList %p",
            mediaConnection->mpSdpCodecList);

    return retValue;
}

void SipXMediaInterfaceImpl::setInterfaceNotificationQueue(OsMsgQ* pInterfaceNotificationQueue)
{
	if (!m_pInterfaceNotificationQueue && mpFlowGraph)
	{
		m_pInterfaceNotificationQueue = pInterfaceNotificationQueue;
      mpFlowGraph->setInterfaceNotificationQueue(pInterfaceNotificationQueue);
	}	
}


OsStatus SipXMediaInterfaceImpl::getCapabilities(int connectionId,
                                                UtlString& rtpHostAddress,
                                                int& rtpAudioPort,
                                                int& rtcpAudioPort,
                                                int& rtpVideoPort,
                                                int& rtcpVideoPort,
                                                SdpCodecList& supportedCodecs,
                                                SdpSrtpParameters& srtpParams,
                                                int& videoBandwidth,
                                                int& videoFramerate)
{
    OsStatus rc = OS_FAILED ;
    SipXMediaConnection* pMediaConn = getMediaConnection(connectionId);
    rtpAudioPort = 0 ;
    rtcpAudioPort = 0 ;
    rtpVideoPort = 0 ;
    rtcpVideoPort = 0 ; 
    videoBandwidth = 0 ;

    if (pMediaConn)
    {
        // Audio RTP
        if (pMediaConn->mpRtpAudioSocket)
        {
            // The "rtpHostAddress" is used for the first RTP stream -- 
            // others are ignored.  They *SHOULD* be the same as the first.  
            // Possible exceptions: STUN worked for the first, but not the
            // others.  Not sure how to handle/recover from that case.
            if (pMediaConn->mContactType == SIP_CONTACT_RELAY)
            {
                assert(!pMediaConn->mIsMulticast);
                if (!((OsNatDatagramSocket*)pMediaConn->mpRtpAudioSocket)->
                                            getRelayIp(&rtpHostAddress, &rtpAudioPort))
                {
                    rtpAudioPort = pMediaConn->mRtpAudioReceivePort ;
                    rtpHostAddress = mRtpReceiveHostAddress ;
                }

            }
            else if (pMediaConn->mContactType == SIP_CONTACT_AUTO || pMediaConn->mContactType == SIP_CONTACT_NAT_MAPPED)
            {
                assert(!pMediaConn->mIsMulticast);
                if (!pMediaConn->mpRtpAudioSocket->getMappedIp(&rtpHostAddress, &rtpAudioPort))
                {
                    rtpAudioPort = pMediaConn->mRtpAudioReceivePort ;
                    rtpHostAddress = mRtpReceiveHostAddress ;
                }
            }
            else if (pMediaConn->mContactType == SIP_CONTACT_LOCAL)
            {
                 rtpHostAddress = pMediaConn->mpRtpAudioSocket->getLocalIp();
                 rtpAudioPort = pMediaConn->mpRtpAudioSocket->getLocalHostPort();
                 if (rtpAudioPort <= 0)
                 {
                     rtpAudioPort = pMediaConn->mRtpAudioReceivePort ;
                     rtpHostAddress = mRtpReceiveHostAddress ;
                 }
            }
            else
            {
              assert(0);
            }               
        }

        // Audio RTCP
        if (pMediaConn->mpRtcpAudioSocket)
        {
            if (pMediaConn->mContactType == SIP_CONTACT_RELAY)
            {
                UtlString tempHostAddress;
                assert(!pMediaConn->mIsMulticast);
                if (!((OsNatDatagramSocket*)pMediaConn->mpRtcpAudioSocket)->
                                            getRelayIp(&tempHostAddress, &rtcpAudioPort))
                {
                    rtcpAudioPort = pMediaConn->mRtcpAudioReceivePort ;
                }
                else
                {
                    // External address should match that of Audio RTP
                    assert(tempHostAddress.compareTo(rtpHostAddress) == 0) ;
                }
            }
            else if (pMediaConn->mContactType == SIP_CONTACT_AUTO || pMediaConn->mContactType == SIP_CONTACT_NAT_MAPPED)
            {
                UtlString tempHostAddress;
                assert(!pMediaConn->mIsMulticast);
                if (!pMediaConn->mpRtcpAudioSocket->getMappedIp(&tempHostAddress, &rtcpAudioPort))
                {
                    rtcpAudioPort = pMediaConn->mRtcpAudioReceivePort ;
                }
                else
                {
                    // External address should match that of Audio RTP
                    assert(tempHostAddress.compareTo(rtpHostAddress) == 0) ;
                }
            }
            else if (pMediaConn->mContactType == SIP_CONTACT_LOCAL)
            {
                rtcpAudioPort = pMediaConn->mpRtcpAudioSocket->getLocalHostPort();
                if (rtcpAudioPort <= 0)
                {
                    rtcpAudioPort = pMediaConn->mRtcpAudioReceivePort ;
                }
            }                
            else
            {
                assert(0);
            }
        }

        supportedCodecs.clearCodecs();
        UtlSList codecList;
        pMediaConn->mpSdpCodecList->getCodecs(codecList);
        supportedCodecs.addCodecs(codecList);
        supportedCodecs.bindPayloadIds();   

        // Setup SRTP parameters here
        memset((void*)&srtpParams, 0, sizeof(SdpSrtpParameters));

        rc = OS_SUCCESS ;
    }

    return rc ;
}


OsStatus SipXMediaInterfaceImpl::getCapabilitiesEx(int connectionId, 
                                                  int nMaxAddresses,
                                                  UtlString rtpHostAddresses[], 
                                                  int rtpAudioPorts[],
                                                  int rtcpAudioPorts[],
                                                  int rtpVideoPorts[],
                                                  int rtcpVideoPorts[],
                                                  RTP_TRANSPORT transportTypes[],
                                                  int& nActualAddresses,
                                                  SdpCodecList& supportedCodecs,
                                                  SdpSrtpParameters& srtpParameters,
                                                  int& videoBandwidth,
                                                  int& videoFramerate)
{   
    OsStatus rc = OS_FAILED ;
    SipXMediaConnection* pMediaConn = getMediaConnection(connectionId);
    nActualAddresses = 0 ;

    // Clear input rtpAudioPorts, rtcpAudioPorts, rtpVideoPorts, rtcpVideoPorts
    // and transportTypes arrays. Do not suppose them to be cleaned by caller.
    memset(rtpAudioPorts, 0, nMaxAddresses*sizeof(int));
    memset(rtcpAudioPorts, 0, nMaxAddresses*sizeof(int));
    memset(rtpVideoPorts, 0, nMaxAddresses*sizeof(int));
    memset(rtcpVideoPorts, 0, nMaxAddresses*sizeof(int));
    for (int i = 0; i < nMaxAddresses; i++)
    {
        transportTypes[i] = RTP_TRANSPORT_UNKNOWN;
    }

    if (pMediaConn)
    {        
        switch (pMediaConn->mContactType)
        {
            case SIP_CONTACT_LOCAL:
                addLocalContacts(connectionId, nMaxAddresses, rtpHostAddresses,
                        rtpAudioPorts, rtcpAudioPorts, rtpVideoPorts, 
                        rtcpVideoPorts, nActualAddresses) ;
                addNatedContacts(connectionId, nMaxAddresses, rtpHostAddresses,
                        rtpAudioPorts, rtcpAudioPorts, rtpVideoPorts, 
                        rtcpVideoPorts, nActualAddresses) ;
                addRelayContacts(connectionId, nMaxAddresses, rtpHostAddresses,
                        rtpAudioPorts, rtcpAudioPorts, rtpVideoPorts, 
                        rtcpVideoPorts, nActualAddresses) ;
                break ;
            case SIP_CONTACT_RELAY:
                addRelayContacts(connectionId, nMaxAddresses, rtpHostAddresses,
                        rtpAudioPorts, rtcpAudioPorts, rtpVideoPorts, 
                        rtcpVideoPorts, nActualAddresses) ;
                addLocalContacts(connectionId, nMaxAddresses, rtpHostAddresses,
                        rtpAudioPorts, rtcpAudioPorts, rtpVideoPorts, 
                        rtcpVideoPorts, nActualAddresses) ;
                addNatedContacts(connectionId, nMaxAddresses, rtpHostAddresses,
                        rtpAudioPorts, rtcpAudioPorts, rtpVideoPorts, 
                        rtcpVideoPorts, nActualAddresses) ;
                break ;
            default:
                addNatedContacts(connectionId, nMaxAddresses, rtpHostAddresses,
                        rtpAudioPorts, rtcpAudioPorts, rtpVideoPorts, 
                        rtcpVideoPorts, nActualAddresses) ;
                addLocalContacts(connectionId, nMaxAddresses, rtpHostAddresses,
                        rtpAudioPorts, rtcpAudioPorts, rtpVideoPorts, 
                        rtcpVideoPorts, nActualAddresses) ;
                addRelayContacts(connectionId, nMaxAddresses, rtpHostAddresses,
                        rtpAudioPorts, rtcpAudioPorts, rtpVideoPorts, 
                        rtcpVideoPorts, nActualAddresses) ;
                break ;


        }

        supportedCodecs.clearCodecs();
        supportedCodecs.addCodecs(*pMediaConn->mpSdpCodecList);
        supportedCodecs.bindPayloadIds();

        memset((void*)&srtpParameters, 0, sizeof(SdpSrtpParameters));
        if (nActualAddresses > 0)
        {
            rc = OS_SUCCESS ;
        }

        // TODO: Need to get real transport types
        for (int i=0; i<nActualAddresses; i++)
        {
            transportTypes[i] = RTP_TRANSPORT_UDP ;
        }
    }

    return rc ;
}

OsStatus
SipXMediaInterfaceImpl::setMediaNotificationsEnabled(bool enabled, 
                                                    const UtlString& resourceName)
{
   return mpFlowGraph ? 
      mpFlowGraph->setNotificationsEnabled(enabled, resourceName) :
      OS_FAILED;
}

SipXMediaConnection* SipXMediaInterfaceImpl::getMediaConnection(int connectionId)
{
   UtlInt matchConnectionId(connectionId);
   return((SipXMediaConnection*) mMediaConnections.find(&matchConnectionId));
}

OsStatus SipXMediaInterfaceImpl::setConnectionDestination(int connectionId,
                                                         const char* remoteRtpHostAddress,
                                                         int remoteAudioRtpPort,
                                                         int remoteAudioRtcpPort,
                                                         int remoteVideoRtpPort,
                                                         int remoteVideoRtcpPort)
{
    OsStatus returnCode = OS_NOT_FOUND;
    SipXMediaConnection* pMediaConnection = getMediaConnection(connectionId);

    if(pMediaConnection && remoteRtpHostAddress && *remoteRtpHostAddress)
    {
        /*
         * Common Setup
         */
        pMediaConnection->mDestinationSet = TRUE;
        pMediaConnection->mRtpSendHostAddress = remoteRtpHostAddress ;

        /*
         * Audio Setup
         */
        pMediaConnection->mRtpAudioSendHostPort = remoteAudioRtpPort;
        pMediaConnection->mRtcpAudioSendHostPort = remoteAudioRtcpPort;

        if(pMediaConnection->mpRtpAudioSocket)
        {
            if (!pMediaConnection->mIsMulticast)
            {
                OsNatDatagramSocket *pSocket = (OsNatDatagramSocket*)pMediaConnection->mpRtpAudioSocket;
                pSocket->readyDestination(remoteRtpHostAddress, remoteAudioRtpPort) ;
                pSocket->applyDestinationAddress(remoteRtpHostAddress, remoteAudioRtpPort, !mStunServer.isNull(), mStunRefreshPeriodSecs, !mTurnServer.isNull()) ;
            }
            else
            {
                pMediaConnection->mpRtpAudioSocket->doConnect(remoteAudioRtpPort,
                                                              remoteRtpHostAddress,
                                                              TRUE);
            }
        }

        if(pMediaConnection->mpRtcpAudioSocket && (remoteAudioRtcpPort > 0))
        {
            if (!pMediaConnection->mIsMulticast)
            {
                OsNatDatagramSocket *pSocket = (OsNatDatagramSocket*)pMediaConnection->mpRtcpAudioSocket;
                pSocket->readyDestination(remoteRtpHostAddress, remoteAudioRtcpPort) ;
                pSocket->applyDestinationAddress(remoteRtpHostAddress, remoteAudioRtcpPort, !mStunServer.isNull(), mStunRefreshPeriodSecs, !mTurnServer.isNull()) ;
            }
            else
            {
                pMediaConnection->mpRtcpAudioSocket->doConnect(remoteAudioRtpPort,
                                                               remoteRtpHostAddress,
                                                               TRUE);
            }
        }
        else
        {
            pMediaConnection->mRtcpAudioSendHostPort = 0 ;
        }

        /*
         * Video Setup
         */
#ifdef VIDEO
        if (pMediaConnection->mpRtpVideoSocket)
        {
            pMediaConnection->mRtpVideoSendHostPort = remoteVideoRtpPort ;                   
            if (!pMediaConnection->mIsMulticast)
            {
                OsNatDatagramSocket *pRtpSocket = (OsNatDatagramSocket*)pMediaConnection->mpRtpVideoSocket;
                pRtpSocket->readyDestination(remoteRtpHostAddress, remoteVideoRtpPort) ;
                pRtpSocket->applyDestinationAddress(remoteRtpHostAddress, remoteVideoRtpPort, !mStunServer.isNull(), mStunRefreshPeriodSecs, !mTurnServer.isNull()) ;
            }
            else
            {
                pMediaConnection->mpRtcpAudioSocket->doConnect(remoteAudioRtpPort,
                                                               remoteRtpHostAddress,
                                                               TRUE);
            }

            if(pMediaConnection->mpRtcpVideoSocket && (remoteVideoRtcpPort > 0))
            {
                pMediaConnection->mRtcpVideoSendHostPort = remoteVideoRtcpPort ;               
                if (!pMediaConnection->mIsMulticast)
                {
                   OsNatDatagramSocket *pRctpSocket = (OsNatDatagramSocket*)pMediaConnection->mpRtcpVideoSocket;
                   pRctpSocket->readyDestination(remoteRtpHostAddress, remoteVideoRtcpPort) ;
                   pRctpSocket->applyDestinationAddress(remoteRtpHostAddress, remoteVideoRtcpPort, !mStunServer.isNull(), mStunRefreshPeriodSecs, !mTurnServer.isNull()) ;
                }
                else
                {
                   pMediaConnection->mpRtcpAudioSocket->doConnect(remoteAudioRtpPort,
                                                                  remoteRtpHostAddress,
                                                                  TRUE);
                }
            }
            else
            {
                pMediaConnection->mRtcpVideoSendHostPort = 0 ;
            }
        }
        else
        {
            pMediaConnection->mRtpVideoSendHostPort = 0 ;
            pMediaConnection->mRtcpVideoSendHostPort = 0 ;
        }        
#endif

        returnCode = OS_SUCCESS;
    }

   return(returnCode);
}

OsStatus SipXMediaInterfaceImpl::addAudioRtpConnectionDestination(int         connectionId,
                                                                 int         iPriority,
                                                                 const char* candidateIp, 
                                                                 int         candidatePort) 
{
    OsStatus returnCode = OS_NOT_FOUND;

    SipXMediaConnection* mediaConnection = getMediaConnection(connectionId);
    if (mediaConnection) 
    {
        // This is not applicable to multicast sockets
        assert(!mediaConnection->mIsMulticast);
        if (mediaConnection->mIsMulticast)
        {
            return OS_FAILED;
        }

        if (    (candidateIp != NULL) && 
                (strlen(candidateIp) > 0) && 
                (strcmp(candidateIp, "0.0.0.0") != 0) &&
                portIsValid(candidatePort) && 
                (mediaConnection->mpRtpAudioSocket != NULL))
        {
            OsNatDatagramSocket *pSocket = (OsNatDatagramSocket*)mediaConnection->mpRtpAudioSocket;
            mediaConnection->mbAlternateDestinations = TRUE;
            pSocket->addAlternateDestination(candidateIp, candidatePort, iPriority);
            pSocket->readyDestination(candidateIp, candidatePort);

            returnCode = OS_SUCCESS;
        }
        else
        {
            returnCode = OS_FAILED ;
        }
    }

    return returnCode ;
}

OsStatus SipXMediaInterfaceImpl::addAudioRtcpConnectionDestination(int         connectionId,
                                                                  int         iPriority,
                                                                  const char* candidateIp, 
                                                                  int         candidatePort) 
{
    OsStatus returnCode = OS_NOT_FOUND;

    SipXMediaConnection* mediaConnection = getMediaConnection(connectionId);
    if (mediaConnection) 
    {        
        // This is not applicable to multicast sockets
        assert(!mediaConnection->mIsMulticast);
        if (mediaConnection->mIsMulticast)
        {
            return OS_FAILED;
        }

        if (    (candidateIp != NULL) && 
                (strlen(candidateIp) > 0) && 
                (strcmp(candidateIp, "0.0.0.0") != 0) &&
                portIsValid(candidatePort) && 
                (mediaConnection->mpRtcpAudioSocket != NULL))
        {
            OsNatDatagramSocket *pSocket = (OsNatDatagramSocket*)mediaConnection->mpRtcpAudioSocket;
            mediaConnection->mbAlternateDestinations = TRUE;
            pSocket->addAlternateDestination(candidateIp, candidatePort, iPriority);
            pSocket->readyDestination(candidateIp, candidatePort);

            returnCode = OS_SUCCESS;
        }
        else
        {
            returnCode = OS_FAILED ;
        }
    }

    return returnCode ;
}

OsStatus SipXMediaInterfaceImpl::addVideoRtpConnectionDestination(int         connectionId,
                                                                 int         iPriority,
                                                                 const char* candidateIp, 
                                                                 int         candidatePort) 
{
    OsStatus returnCode = OS_NOT_FOUND;
#ifdef VIDEO
    SipXMediaConnection* mediaConnection = getMediaConnection(connectionId);
    if (mediaConnection) 
    {        
        // This is not applicable to multicast sockets
        assert(!mediaConnection->mIsMulticast);
        if (mediaConnection->mIsMulticast)
        {
            return OS_FAILED;
        }

        if (    (candidateIp != NULL) && 
                (strlen(candidateIp) > 0) && 
                (strcmp(candidateIp, "0.0.0.0") != 0) &&
                portIsValid(candidatePort) && 
                (mediaConnection->mpRtpVideoSocket != NULL))
        {
            OsNatDatagramSocket *pSocket = (OsNatDatagramSocket*)mediaConnection->mpRtpVideoSocket;
            mediaConnection->mbAlternateDestinations = TRUE;
            pSocket->addAlternateDestination(candidateIp, candidatePort, iPriority);
            pSocket->readyDestination(candidateIp, candidatePort);

            returnCode = OS_SUCCESS;
        }
        else
        {
            returnCode = OS_FAILED ;
        }
    }
#endif
    return returnCode ;    
}

OsStatus SipXMediaInterfaceImpl::addVideoRtcpConnectionDestination(int         connectionId,
                                                                  int         iPriority,
                                                                  const char* candidateIp, 
                                                                  int         candidatePort) 
{
    OsStatus returnCode = OS_NOT_FOUND;
#ifdef VIDEO
    SipXMediaConnection* mediaConnection = getMediaConnection(connectionId);
    if (mediaConnection) 
    {        
        // This is not applicable to multicast sockets
        assert(!mediaConnection->mIsMulticast);
        if (mediaConnection->mIsMulticast)
        {
            return OS_FAILED;
        }

        if (    (candidateIp != NULL) && 
                (strlen(candidateIp) > 0) && 
                (strcmp(candidateIp, "0.0.0.0") != 0) &&
                portIsValid(candidatePort) && 
                (mediaConnection->mpRtcpVideoSocket != NULL))
        {
            OsNatDatagramSocket *pSocket = (OsNatDatagramSocket*)mediaConnection->mpRtcpVideoSocket;
            mediaConnection->mbAlternateDestinations = TRUE;
            pSocket->addAlternateDestination(candidateIp, candidatePort, iPriority);
            pSocket->readyDestination(candidateIp, candidatePort);

            returnCode = OS_SUCCESS;
        }
        else
        {
            returnCode = OS_FAILED ;
        }
    }
#endif
    return returnCode ;    
}


OsStatus SipXMediaInterfaceImpl::startRtpSend(int connectionId,
                                             const SdpCodecList& sdpCodecList)
{
   // need to set default payload types in get capabilities
   SdpCodec* audioCodec = NULL;
   SdpCodec* dtmfCodec = NULL;
   OsStatus returnCode = OS_NOT_FOUND;
   SipXMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if (mediaConnection == NULL)
      return returnCode;
   // find DTMF & primary audio codec
   UtlSList utlCodecList;
   sdpCodecList.getCodecs(utlCodecList);
   SdpCodec* pCodec = NULL;
   UtlSListIterator itor(utlCodecList);
   while (itor())
   {
      pCodec = dynamic_cast<SdpCodec*>(itor.item());
      if (pCodec)
      {
         UtlString codecMediaType;
         pCodec->getMediaType(codecMediaType);
         if (pCodec->getValue() == SdpCodec::SDP_CODEC_TONES)
         {
            dtmfCodec = pCodec;
         }
         else if (codecMediaType.compareTo(MIME_TYPE_AUDIO, UtlString::ignoreCase) == 0 && audioCodec == NULL)
         {
            audioCodec = pCodec;
         }
      }
   }

   // If we haven't set a destination and we have set alternate destinations
   if (!mediaConnection->mDestinationSet && mediaConnection->mbAlternateDestinations)
   {
      applyAlternateDestinations(connectionId) ;
   }

   if (mpFlowGraph)
   {
       // Store the primary codec for cost calculations later
       if (mediaConnection->mpAudioCodec != NULL)
       {
           delete mediaConnection->mpAudioCodec ;
           mediaConnection->mpAudioCodec = NULL ;
       }
       if (audioCodec != NULL)
       {
           mediaConnection->mpAudioCodec = new SdpCodec();
           *mediaConnection->mpAudioCodec = *audioCodec ;
       }

      if (!mediaConnection->mRtpSendHostAddress.isNull() && mediaConnection->mRtpSendHostAddress.compareTo("0.0.0.0"))
      {
         // This is the new interface for parallel codecs
         mpFlowGraph->startSendRtp(*(mediaConnection->mpRtpAudioSocket),
                                   *(mediaConnection->mpRtcpAudioSocket),
                                   connectionId,
                                   audioCodec,
                                   dtmfCodec);

         mediaConnection->mRtpAudioSending = TRUE;
         returnCode = OS_SUCCESS;
      }
   }
   return returnCode;
}


OsStatus SipXMediaInterfaceImpl::startRtpReceive(int connectionId,
                                                const SdpCodecList& sdpCodecList)
{
   OsStatus returnCode = OS_NOT_FOUND;

   SipXMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if (mediaConnection == NULL)
      return OS_NOT_FOUND;

   UtlSList utlCodecList;
   sdpCodecList.getCodecs(utlCodecList);

   if (mpFlowGraph)
   {
      mpFlowGraph->startReceiveRtp(sdpCodecList,
           *(mediaConnection->mpRtpAudioSocket), *(mediaConnection->mpRtcpAudioSocket),
           connectionId);
      mediaConnection->mRtpAudioReceiving = TRUE;

      returnCode = OS_SUCCESS;
   }
   return returnCode;
}

OsStatus SipXMediaInterfaceImpl::stopRtpSend(int connectionId)
{
   OsStatus returnCode = OS_NOT_FOUND;
   SipXMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if (mpFlowGraph && mediaConnection &&
       mediaConnection->mRtpAudioSending)
   {
      mpFlowGraph->stopSendRtp(connectionId);
      mediaConnection->mRtpAudioSending = FALSE;
      returnCode = OS_SUCCESS;
   }
   return(returnCode);
}

OsStatus SipXMediaInterfaceImpl::stopRtpReceive(int connectionId)
{
   OsStatus returnCode = OS_NOT_FOUND;
   SipXMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if (mpFlowGraph && mediaConnection &&
       mediaConnection->mRtpAudioReceiving)
   {
      mpFlowGraph->stopReceiveRtp(connectionId);
      mediaConnection->mRtpAudioReceiving = FALSE;
      returnCode = OS_SUCCESS;
   }
   return returnCode;
}

OsStatus SipXMediaInterfaceImpl::deleteConnection(int connectionId)
{
   OsStatus returnCode = OS_NOT_FOUND;
   SipXMediaConnection* mediaConnection =
       getMediaConnection(connectionId);

   UtlInt matchConnectionId(connectionId);
   mMediaConnections.remove(&matchConnectionId) ;

   returnCode = doDeleteConnection(mediaConnection);

   delete mediaConnection ;

   return(returnCode);
}

OsStatus SipXMediaInterfaceImpl::doDeleteConnection(SipXMediaConnection* mediaConnection)
{
   OsStatus returnCode = OS_NOT_FOUND;

   if(mediaConnection == NULL)
   {
      OsSysLog::add(FAC_CP, PRI_DEBUG, 
                  "SipXMediaInterfaceImpl::doDeleteConnection mediaConnection is NULL!");
      return OS_NOT_FOUND;
   }

   OsSysLog::add(FAC_CP, PRI_DEBUG, "SipXMediaInterfaceImpl::deleteConnection deleting the connection %p",
      mediaConnection);

   returnCode = OS_SUCCESS;
   mediaConnection->mDestinationSet = FALSE;

   returnCode = stopRtpSend(mediaConnection->getValue());
   returnCode = stopRtpReceive(mediaConnection->getValue());

   if(mediaConnection->getValue() >= 0)
   {
      mpFlowGraph->deleteConnection(mediaConnection->getValue());
      mediaConnection->setValue(-1);
      mpFlowGraph->synchronize();
   }

   mpFactoryImpl->releaseRtpPort(mediaConnection->mRtpAudioReceivePort) ;

   if(mediaConnection->mpRtpAudioSocket)
   {
      delete mediaConnection->mpRtpAudioSocket;
      mediaConnection->mpRtpAudioSocket = NULL;
   }
   if(mediaConnection->mpRtcpAudioSocket)
   {
      delete mediaConnection->mpRtcpAudioSocket;
      mediaConnection->mpRtcpAudioSocket = NULL;
   }

   return(returnCode);
}


OsStatus SipXMediaInterfaceImpl::playAudio(const char* url,
                                          UtlBoolean repeat,
                                          UtlBoolean local,
                                          UtlBoolean remote,
                                          UtlBoolean mixWithMic,
                                          int downScaling,
                                          void* pCookie)
{
    OsStatus returnCode = OS_NOT_FOUND;
    UtlString urlString;
    if(url) urlString.append(url);
    size_t fileIndex = urlString.index("file://");
    if(fileIndex == 0) urlString.remove(0, 6);

    if(mpFlowGraph && !urlString.isNull())
    {
         int toneOptions=0;

         if (local)
         {
            toneOptions |= MpCallFlowGraph::TONE_TO_SPKR;
         }                  
         
         if(remote)
         {
            toneOptions |= MpCallFlowGraph::TONE_TO_NET;
         }

        // Start playing the audio file
        returnCode = mpFlowGraph->playFile(urlString.data(), repeat, toneOptions, pCookie);
    }

    if(returnCode != OS_SUCCESS)
    {
        osPrintf("Cannot play audio file: %s\n", urlString.data());
    }

    return(returnCode);
}

OsStatus SipXMediaInterfaceImpl::playBuffer(void* buf,
                                           size_t bufSize,
                                           int type, 
                                           UtlBoolean repeat,
                                           UtlBoolean local,
                                           UtlBoolean remote,
                                           UtlBoolean mixWithMic,
                                           int downScaling,
                                           void* pCookie)
{
    OsStatus returnCode = OS_NOT_FOUND;
    if(mpFlowGraph && buf)
    {
         int toneOptions=0;

         if (local)
         {
            toneOptions |= MpCallFlowGraph::TONE_TO_SPKR;
         }                  
         
         if(remote)
         {
            toneOptions |= MpCallFlowGraph::TONE_TO_NET;
         }

        // Start playing the audio file
        returnCode = mpFlowGraph->playBuffer(buf, bufSize, type, repeat, toneOptions, pCookie);
    }

    if(returnCode != OS_SUCCESS)
    {
        osPrintf("Cannot play audio buffer: %10p\n", buf);
    }

    return(returnCode);
}


OsStatus SipXMediaInterfaceImpl::stopAudio()
{
    OsStatus returnCode = OS_NOT_FOUND;
    if(mpFlowGraph)
    {
        mpFlowGraph->stopFile();
        returnCode = OS_SUCCESS;
    }
    return(returnCode);
}

OsStatus SipXMediaInterfaceImpl::pausePlayback()
{
   OsStatus returnCode = OS_FAILED;

   if(mpFlowGraph)
   {
      returnCode = mpFlowGraph->pausePlayback();
   }
   return(returnCode);
}

OsStatus SipXMediaInterfaceImpl::resumePlayback()
{
   OsStatus returnCode = OS_FAILED;

   if(mpFlowGraph)
   {
      returnCode = mpFlowGraph->resumePlayback();
   }
   return(returnCode);
}

OsStatus SipXMediaInterfaceImpl::startTone(int toneId,
                                           UtlBoolean local,
                                           UtlBoolean remote,
                                           int duration)
{
   OsStatus returnCode = OS_SUCCESS;
   int toneDestination = 0 ;

   if(mpFlowGraph)
   {
      if (local)
      {
         toneDestination |= MpCallFlowGraph::TONE_TO_SPKR;
      }                  
      
      if(remote)
      {
         toneDestination |= MpCallFlowGraph::TONE_TO_NET;
      }
     
      mpFlowGraph->startTone(toneId, toneDestination, duration);
   } 

   return(returnCode);
}

OsStatus SipXMediaInterfaceImpl::stopTone()
{
   OsStatus returnCode = OS_SUCCESS;
   if(mpFlowGraph)
   {
      mpFlowGraph->stopTone();
   }

   return(returnCode);
}

OsStatus SipXMediaInterfaceImpl::muteInput(int connectionId, UtlBoolean bMute)
{
   OsStatus returnCode = OS_FAILED;
   if(mpFlowGraph)
   {
	   if(bMute)
	   {
	      return mpFlowGraph->muteInput(connectionId);
	   }
	   else
	   {
		  return mpFlowGraph->unmuteInput(connectionId);
	   }
   }

   return(returnCode);
}

OsStatus SipXMediaInterfaceImpl::giveFocus()
{
    if(mpFlowGraph)
    {
        // There should probably be a lock here
        // Set the flow graph to have the focus
        MpMediaTask* mediaTask = MpMediaTask::getMediaTask();
        mediaTask->setFocus(mpFlowGraph);
        // osPrintf("Setting focus for flow graph\n");
   }

   return OS_SUCCESS ;
}

OsStatus SipXMediaInterfaceImpl::defocus()
{
    if(mpFlowGraph)
    {
        MpMediaTask* mediaTask = MpMediaTask::getMediaTask();

        // There should probably be a lock here
        // take focus away from the flow graph if it is focus
        if(mpFlowGraph == (MpCallFlowGraph*) mediaTask->getFocus())
        {
            mediaTask->setFocus(NULL);
            // osPrintf("Setting NULL focus for flow graph\n");
        }
    }
    return OS_SUCCESS ;
}

UtlBoolean SipXMediaInterfaceImpl::hasFocus()
{
   if (mpFlowGraph)
   {
      MpMediaTask* mediaTask = MpMediaTask::getMediaTask();
      if (mediaTask)
      {
         if (mediaTask->getFocus() == mpFlowGraph)
         {
            return TRUE;
         }
      }
   }

   return FALSE;
}

OsStatus SipXMediaInterfaceImpl::recordAudio(const char* szFile)
{
   if (mpFlowGraph)
   {
      /* use new call recorder
      from now on, call recorder records both mic, speaker and local dtmf      
      we don't want raw pcm, but wav pcm, raw pcm should be passed to a callback
      meant for recording, for example for conversion to mp3 or other format */
      return mpFlowGraph->record(0, -1, szFile, 0, 0, NULL, MprRecorder::WAV_PCM_16);
   }

   return OS_FAILED;
}

OsStatus SipXMediaInterfaceImpl::stopRecording()
{
   OsStatus ret = OS_UNSPECIFIED;
   if (mpFlowGraph)
   {
#ifdef TEST_PRINT
     osPrintf("SipXMediaInterfaceImpl::stopRecording() : calling flowgraph::stoprecorders\n");
     OsSysLog::add(FAC_CP, PRI_DEBUG, "SipXMediaInterfaceImpl::stopRecording() : calling flowgraph::stoprecorders");
#endif
     mpFlowGraph->closeRecorders();
     ret = OS_SUCCESS;
   }
   
   return ret;
}

void SipXMediaInterfaceImpl::setContactType(int connectionId, SIP_CONTACT_TYPE eType, int contactId) 
{
    SipXMediaConnection* pMediaConn = getMediaConnection(connectionId);

    if (pMediaConn)
    {
        if (pMediaConn->mIsMulticast && eType == SIP_CONTACT_AUTO)
        {
            pMediaConn->mContactType = SIP_CONTACT_LOCAL;
        }
        else
        {
            // Only CONTACT_LOCAL is allowed for multicast addresses.
            assert(!pMediaConn->mIsMulticast || eType == SIP_CONTACT_LOCAL);
            pMediaConn->mContactType = eType;
        }
    }
}

OsStatus SipXMediaInterfaceImpl::setCodecList(const SdpCodecList& sdpCodecList)
{
   mSdpCodecList = sdpCodecList;
   // Assign any unset payload types
   mSdpCodecList.bindPayloadIds();

   return OS_SUCCESS;
}

OsStatus SipXMediaInterfaceImpl::getCodecList(SdpCodecList& sdpCodecList)
{
   sdpCodecList = mSdpCodecList;
   return OS_SUCCESS;
}

OsStatus SipXMediaInterfaceImpl::getCodecList(int connectionId, SdpCodecList& sdpCodecList)
{
   OsStatus rc = OS_FAILED ;
   SipXMediaConnection* pMediaConn = getMediaConnection(connectionId);
   if (pMediaConn)
   {
      sdpCodecList.clearCodecs();
      sdpCodecList.addCodecs(*pMediaConn->mpSdpCodecList);
      sdpCodecList.bindPayloadIds();

      rc = OS_SUCCESS;
   }

   return rc;
}

OsStatus SipXMediaInterfaceImpl::setConnectionFramerate(int connectionId, int framerate) 
{
    return OS_NOT_SUPPORTED ;
}


OsStatus SipXMediaInterfaceImpl::setSecurityAttributes(const void* security) 
{
    return OS_NOT_SUPPORTED ;
}


/* ============================ ACCESSORS ================================= */


OsStatus SipXMediaInterfaceImpl::setVideoQuality(int quality)
{
   return OS_SUCCESS;
}

OsStatus SipXMediaInterfaceImpl::setVideoParameters(int bitRate, int frameRate)
{
   return OS_SUCCESS;
}

OsStatus SipXMediaInterfaceImpl::getPrimaryCodec(int connectionId, 
                                                UtlString& audioCodec,
                                                UtlString& videoCodec,
                                                int* audioPayloadType,
                                                int* videoPayloadType,
                                                bool& isEncrypted)
{
    UtlString codecType;
    SipXMediaConnection* pConnection = getMediaConnection(connectionId);
    if (pConnection == NULL)
       return OS_NOT_FOUND;

    if (pConnection->mpAudioCodec != NULL)
    {
        pConnection->mpAudioCodec->getCodecName(audioCodec);
        *audioPayloadType = pConnection->mpAudioCodec->getCodecPayloadId();
    }

    videoCodec="";
    *videoPayloadType=0;

   return OS_SUCCESS;
}

OsStatus SipXMediaInterfaceImpl::getVideoQuality(int& quality)
{
   quality = 0;
   return OS_SUCCESS;
}

OsStatus SipXMediaInterfaceImpl::getVideoBitRate(int& bitRate)
{
   bitRate = 0;
   return OS_SUCCESS;
}


OsStatus SipXMediaInterfaceImpl::getVideoFrameRate(int& frameRate)
{
   frameRate = 0;
   return OS_SUCCESS;
}

/* ============================ INQUIRY =================================== */
UtlBoolean SipXMediaInterfaceImpl::isSendingRtpAudio(int connectionId)
{
   UtlBoolean sending = FALSE;
   SipXMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if(mediaConnection)
   {
       sending = mediaConnection->mRtpAudioSending;
   }
   else
   {
       osPrintf("SipXMediaInterfaceImpl::isSendingRtpAudio invalid connectionId: %d\n",
          connectionId);
   }

   return(sending);
}

UtlBoolean SipXMediaInterfaceImpl::isReceivingRtpAudio(int connectionId)
{
   UtlBoolean receiving = FALSE;
   SipXMediaConnection* mediaConnection = getMediaConnection(connectionId);

   if(mediaConnection)
   {
      receiving = mediaConnection->mRtpAudioReceiving;
   }
   else
   {
       osPrintf("SipXMediaInterfaceImpl::isReceivingRtpAudio invalid connectionId: %d\n",
          connectionId);
   }
   return(receiving);
}

UtlBoolean SipXMediaInterfaceImpl::isSendingRtpVideo(int connectionId)
{
   UtlBoolean sending = FALSE;

   return(sending);
}

UtlBoolean SipXMediaInterfaceImpl::isReceivingRtpVideo(int connectionId)
{
   UtlBoolean receiving = FALSE;

   return(receiving);
}


UtlBoolean SipXMediaInterfaceImpl::isDestinationSet(int connectionId)
{
    UtlBoolean isSet = FALSE;
    SipXMediaConnection* mediaConnection = getMediaConnection(connectionId);

    if(mediaConnection)
    {
        isSet = mediaConnection->mDestinationSet;
    }
    else
    {
       osPrintf("SipXMediaInterfaceImpl::isDestinationSet invalid connectionId: %d\n",
          connectionId);
    }
    return(isSet);
}

UtlBoolean SipXMediaInterfaceImpl::canAddParty() 
{
   // this limit is due to limit of MpCallFlowGraph using arrays
   return mMediaConnections.entries() < MAX_CONNECTIONS;
}

UtlBoolean SipXMediaInterfaceImpl::isVideoInitialized(int connectionId)
{
   return false ;
}

UtlBoolean SipXMediaInterfaceImpl::isAudioInitialized(int connectionId) 
{
    return true ;
}

UtlBoolean SipXMediaInterfaceImpl::isAudioAvailable() 
{
    return true ;
}

OsStatus SipXMediaInterfaceImpl::setVideoWindowDisplay(const void* hWnd)
{
   return OS_NOT_YET_IMPLEMENTED;
}

const void* SipXMediaInterfaceImpl::getVideoWindowDisplay()
{
   return NULL;
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */


UtlBoolean SipXMediaInterfaceImpl::getLocalAddresses(int connectionId,
                                                     int nMaxAddresses,
                                                     UtlString hostIps[],
                                                     int& rtpAudioPort,
                                                     int& rtcpAudioPort,
                                                     int& rtpVideoPort,
                                                     int& rtcpVideoPort,
                                                     int& nActualAddresses)
{
    UtlBoolean bRC = FALSE ;
    SipXMediaConnection* pMediaConn = getMediaConnection(connectionId);

    rtpAudioPort = PORT_NONE;
    rtcpAudioPort = PORT_NONE;
    rtpVideoPort = PORT_NONE;
    rtcpVideoPort = PORT_NONE;

    for (int i = 0; i < nMaxAddresses; i++)
    {
       hostIps[i].remove(0);
    }

    if (pMediaConn)
    {
        // Audio rtp port (must exist)
        if (pMediaConn->mpRtpAudioSocket)
        {
            UtlString bindIp = pMediaConn->mpRtpAudioSocket->getLocalIp(); // may be 0.0.0.0

            if (bindIp.compareTo("0.0.0.0") == 0) // bound to all
            {
               int addressCount = 32;
               const HostAdapterAddress* utlAddresses[32];
               if (OsNetwork::getAllLocalHostIps(utlAddresses, addressCount)) // changes addressCount
               {
                  // this doesn't return 0.0.0.0 or 127.0.0.1
                  for (int i = 0; (i < addressCount) && (i < nMaxAddresses); i++)
                  {
                     hostIps[i] = utlAddresses[i]->mAddress;
                     delete utlAddresses[i];
                  }
                  nActualAddresses = addressCount;
               }
               else
               {
                  return FALSE;
               }
            }
            else
            {
               // bound to single ip
               hostIps[0] = bindIp;
               nActualAddresses = 1;
            }

            rtpAudioPort = pMediaConn->mpRtpAudioSocket->getLocalHostPort();
            if (rtpAudioPort > 0)
            {
                bRC = TRUE ;
            }

            // Audio rtcp port (optional) 
            if (pMediaConn->mpRtcpAudioSocket && bRC)
            {
                rtcpAudioPort = pMediaConn->mpRtcpAudioSocket->getLocalHostPort();
            }        
        }

#ifdef VIDEO
        // Video rtp port (optional)
        if (pMediaConn->mpRtpVideoSocket && bRC)
        {
            rtpVideoPort = pMediaConn->mpRtpVideoSocket->getLocalHostPort();

            // Video rtcp port (optional)
            if (pMediaConn->mpRtcpVideoSocket)
            {
                rtcpVideoPort = pMediaConn->mpRtcpVideoSocket->getLocalHostPort();
            }
        }
#endif
    }

    return bRC ;
}

UtlBoolean SipXMediaInterfaceImpl::getNatedAddresses(int connectionId,
                                                    UtlString& hostIp,
                                                    int& rtpAudioPort,
                                                    int& rtcpAudioPort,
                                                    int& rtpVideoPort,
                                                    int& rtcpVideoPort)
{
    UtlBoolean bRC = FALSE ;
    UtlString host ;
    int port ;
    SipXMediaConnection* pMediaConn = getMediaConnection(connectionId);

    hostIp.remove(0) ;
    rtpAudioPort = PORT_NONE ;
    rtcpAudioPort = PORT_NONE ;
    rtpVideoPort = PORT_NONE ;
    rtcpVideoPort = PORT_NONE ;

    if (pMediaConn)
    {
        // Audio rtp port (must exist)
        if (pMediaConn->mpRtpAudioSocket)
        {
            if (pMediaConn->mpRtpAudioSocket->getMappedIp(&host, &port))
            {
                if (port > 0)
                {
                    hostIp = host ;
                    rtpAudioPort = port ;

                    bRC = TRUE ;
                }
            
                // Audio rtcp port (optional) 
                if (pMediaConn->mpRtcpAudioSocket && bRC)
                {
                    if (pMediaConn->mpRtcpAudioSocket->getMappedIp(&host, &port))
                    {
                        rtcpAudioPort = port ;
                        if (host.compareTo(hostIp) != 0)
                        {
                            OsSysLog::add(FAC_MP, PRI_ERR, 
                                    "Stun host IP mismatches for rtcp/audio (%s != %s)", 
                                    hostIp.data(), host.data()) ;                          
                        }
                    }
                }
            }
        }

#ifdef VIDEO
        // Video rtp port (optional)
        if (pMediaConn->mpRtpVideoSocket && bRC)
        {
            if (pMediaConn->mpRtpVideoSocket->getMappedIp(&host, &port))
            {
                rtpVideoPort = port ;
                if (host.compareTo(hostIp) != 0)
                {
                    OsSysLog::add(FAC_MP, PRI_ERR, 
                            "Stun host IP mismatches for rtp/video (%s != %s)", 
                            hostIp.data(), host.data()) ;                          
                }

                // Video rtcp port (optional)
                if (pMediaConn->mpRtcpVideoSocket)
                {
                    if (pMediaConn->mpRtcpVideoSocket->getMappedIp(&host, &port))
                    {
                        rtcpVideoPort = port ;
                        if (host.compareTo(hostIp) != 0)
                        {
                            OsSysLog::add(FAC_MP, PRI_ERR, 
                                    "Stun host IP mismatches for rtcp/video (%s != %s)", 
                                    hostIp.data(), host.data()) ;                          
                        }
                    }
                }
            }            
        }
#endif
    }

    return bRC ;
}

UtlBoolean SipXMediaInterfaceImpl::getRelayAddresses(int connectionId,
                                                    UtlString& hostIp,
                                                    int& rtpAudioPort,
                                                    int& rtcpAudioPort,
                                                    int& rtpVideoPort,
                                                    int& rtcpVideoPort)
{
    UtlBoolean bRC = FALSE ;
    UtlString host ;
    int port ;
    SipXMediaConnection* pMediaConn = getMediaConnection(connectionId);

    hostIp.remove(0) ;
    rtpAudioPort = PORT_NONE ;
    rtcpAudioPort = PORT_NONE ;
    rtpVideoPort = PORT_NONE ;
    rtcpVideoPort = PORT_NONE ;

    if (pMediaConn)
    {
        assert(!pMediaConn->mIsMulticast);
        if (pMediaConn->mIsMulticast)
        {
           return FALSE;
        }

        // Audio rtp port (must exist)
        if (pMediaConn->mpRtpAudioSocket)
        {
            if (((OsNatDatagramSocket*)pMediaConn->mpRtpAudioSocket)->getRelayIp(&host, &port))
            {
                if (port > 0)
                {
                    hostIp = host ;
                    rtpAudioPort = port ;

                    bRC = TRUE ;
                }
            
                // Audio rtcp port (optional) 
                if (pMediaConn->mpRtcpAudioSocket && bRC)
                {
                    if (((OsNatDatagramSocket*)pMediaConn->mpRtcpAudioSocket)->getRelayIp(&host, &port))
                    {
                        rtcpAudioPort = port ;
                        if (host.compareTo(hostIp) != 0)
                        {
                            OsSysLog::add(FAC_MP, PRI_ERR, 
                                    "Turn host IP mismatches for rtcp/audio (%s != %s)", 
                                    hostIp.data(), host.data()) ;                          
                        }
                    }
                }
            }
        }

#ifdef VIDEO
        // Video rtp port (optional)
        if (pMediaConn->mpRtpVideoSocket && bRC)
        {
            if ((OsNatDatagramSocket*)pMediaConn->mpRtpVideoSocket)->getRelayIp(&host, &port))
            {
                rtpVideoPort = port ;
                if (host.compareTo(hostIp) != 0)
                {
                    OsSysLog::add(FAC_MP, PRI_ERR, 
                            "Turn host IP mismatches for rtp/video (%s != %s)", 
                            hostIp.data(), host.data()) ;                          
                }

                // Video rtcp port (optional)
                if (pMediaConn->mpRtcpVideoSocket)
                {
                    if ((OsNatDatagramSocket*)pMediaConn->mpRtcpVideoSocket)->getRelayIp(&host, &port))
                    {
                        rtcpVideoPort = port ;
                        if (host.compareTo(hostIp) != 0)
                        {
                            OsSysLog::add(FAC_MP, PRI_ERR, 
                                    "Turn host IP mismatches for rtcp/video (%s != %s)", 
                                    hostIp.data(), host.data()) ;                          
                        }
                    }
                }
            }            
        }
#endif
    }

    return bRC ;
}

OsStatus SipXMediaInterfaceImpl::addLocalContacts(int connectionId, 
                                                 int nMaxAddresses,
                                                 UtlString rtpHostAddresses[], 
                                                 int rtpAudioPorts[],
                                                 int rtcpAudioPorts[],
                                                 int rtpVideoPorts[],
                                                 int rtcpVideoPorts[],
                                                 int& nActualAddresses)
{
    int rtpAudioPort = PORT_NONE ;
    int rtcpAudioPort = PORT_NONE ;
    int rtpVideoPort = PORT_NONE ;
    int rtcpVideoPort = PORT_NONE ;
    const int localIpsMaxCount = 32;
    UtlString localIps[localIpsMaxCount];
    int actualLocalIps = 0;
    OsStatus rc = OS_FAILED ;

    // Local Addresses
    if ((nActualAddresses < nMaxAddresses) && 
        getLocalAddresses(connectionId, localIpsMaxCount, localIps, rtpAudioPort, 
                          rtcpAudioPort, rtpVideoPort, rtcpVideoPort, actualLocalIps))
    {
       for (int i = 0; (i < actualLocalIps) && (i < nMaxAddresses); i++)
       {
          // Check for duplicates
          for (int k = 0; k < nActualAddresses; k++)
          {
             if ((rtpHostAddresses[k].compareTo(localIps[i]) == 0) &&
                (rtpAudioPorts[k] == rtpAudioPort) &&
                (rtcpAudioPorts[k] == rtcpAudioPort) &&
                (rtpVideoPorts[k] == rtpVideoPort) &&
                (rtcpVideoPorts[k] == rtcpVideoPort))
             {
                // duplicate
                continue;
             }
          }

          // not duplicate, add it
          rtpHostAddresses[nActualAddresses] = localIps[i];
          rtpAudioPorts[nActualAddresses] = rtpAudioPort;
          rtcpAudioPorts[nActualAddresses] = rtcpAudioPort;
          rtpVideoPorts[nActualAddresses] = rtpVideoPort;
          rtcpVideoPorts[nActualAddresses] = rtcpVideoPort;
          nActualAddresses++;

          rc = OS_SUCCESS;
       }
    }

    return rc;
}


OsStatus SipXMediaInterfaceImpl::addNatedContacts(int connectionId, 
                                                 int nMaxAddresses,
                                                 UtlString rtpHostAddresses[], 
                                                 int rtpAudioPorts[],
                                                 int rtcpAudioPorts[],
                                                 int rtpVideoPorts[],
                                                 int rtcpVideoPorts[],
                                                 int& nActualAddresses)
{
    UtlString hostIp ;
    int rtpAudioPort = PORT_NONE ;
    int rtcpAudioPort = PORT_NONE ;
    int rtpVideoPort = PORT_NONE ;
    int rtcpVideoPort = PORT_NONE ;
    OsStatus rc = OS_FAILED ;

    // NAT Addresses
    if (    (nActualAddresses < nMaxAddresses) && 
            getNatedAddresses(connectionId, hostIp, rtpAudioPort, 
            rtcpAudioPort, rtpVideoPort, rtcpVideoPort) )
    {
        bool bDuplicate = false ;
        
        // Check for duplicates
        for (int i=0; i<nActualAddresses; i++)
        {
            if (    (rtpHostAddresses[i].compareTo(hostIp) == 0) &&
                    (rtpAudioPorts[i] == rtpAudioPort) &&
                    (rtcpAudioPorts[i] == rtcpAudioPort) &&
                    (rtpVideoPorts[i] == rtpVideoPort) &&
                    (rtcpVideoPorts[i] == rtcpVideoPort))
            {
                bDuplicate = true ;
                break ;
            }
        }

        if (!bDuplicate)
        {
            rtpHostAddresses[nActualAddresses] = hostIp ;
            rtpAudioPorts[nActualAddresses] = rtpAudioPort ;
            rtcpAudioPorts[nActualAddresses] = rtcpAudioPort ;
            rtpVideoPorts[nActualAddresses] = rtpVideoPort ;
            rtcpVideoPorts[nActualAddresses] = rtcpVideoPort ;
            nActualAddresses++ ;

            rc = OS_SUCCESS ;
        }
    }
    return rc ;
}


OsStatus SipXMediaInterfaceImpl::addRelayContacts(int connectionId, 
                                                 int nMaxAddresses,
                                                 UtlString rtpHostAddresses[], 
                                                 int rtpAudioPorts[],
                                                 int rtcpAudioPorts[],
                                                 int rtpVideoPorts[],
                                                 int rtcpVideoPorts[],
                                                 int& nActualAddresses)
{
    UtlString hostIp ;
    int rtpAudioPort = PORT_NONE ;
    int rtcpAudioPort = PORT_NONE ;
    int rtpVideoPort = PORT_NONE ;
    int rtcpVideoPort = PORT_NONE ;
    OsStatus rc = OS_FAILED ;

    // Relay Addresses
    if (    (nActualAddresses < nMaxAddresses) && 
            getRelayAddresses(connectionId, hostIp, rtpAudioPort, 
            rtcpAudioPort, rtpVideoPort, rtcpVideoPort) )
    {
        bool bDuplicate = false ;
        
        // Check for duplicates
        for (int i=0; i<nActualAddresses; i++)
        {
            if (    (rtpHostAddresses[i].compareTo(hostIp) == 0) &&
                    (rtpAudioPorts[i] == rtpAudioPort) &&
                    (rtcpAudioPorts[i] == rtcpAudioPort) &&
                    (rtpVideoPorts[i] == rtpVideoPort) &&
                    (rtcpVideoPorts[i] == rtcpVideoPort))
            {
                bDuplicate = true ;
                break ;
            }
        }

        if (!bDuplicate)
        {
            rtpHostAddresses[nActualAddresses] = hostIp ;
            rtpAudioPorts[nActualAddresses] = rtpAudioPort ;
            rtcpAudioPorts[nActualAddresses] = rtcpAudioPort ;
            rtpVideoPorts[nActualAddresses] = rtpVideoPort ;
            rtcpVideoPorts[nActualAddresses] = rtcpVideoPort ;
            nActualAddresses++ ;

            rc = OS_SUCCESS ;
        }
    }

    return rc ;
}


void SipXMediaInterfaceImpl::applyAlternateDestinations(int connectionId) 
{
    UtlString destAddress ;
    int       destPort ;

    SipXMediaConnection* pMediaConnection = getMediaConnection(connectionId);

    if (pMediaConnection)
    {
        assert(!pMediaConnection->mIsMulticast);
        if (pMediaConnection->mIsMulticast)
        {
           return;
        }

        assert(!pMediaConnection->mDestinationSet) ;
        pMediaConnection->mDestinationSet = true ;

        pMediaConnection->mRtpSendHostAddress.remove(0) ;
        pMediaConnection->mRtpAudioSendHostPort = 0 ;
        pMediaConnection->mRtcpAudioSendHostPort = 0 ;
#ifdef VIDEO
        pMediaConnection->mRtpVideoSendHostPort = 0 ;
        pMediaConnection->mRtcpVideoSendHostPort = 0 ;
#endif

        // TODO: We should REALLY store a different host for each connection -- they could
        //       differ when using TURN (could get forwarded to another turn server)
        //       For now, we store the rtp host


        // Connect RTP Audio Socket
        if (pMediaConnection->mpRtpAudioSocket)
        {
           OsNatDatagramSocket *pSocket = (OsNatDatagramSocket*)pMediaConnection->mpRtpAudioSocket;
            if (pSocket->getBestDestinationAddress(destAddress, destPort))
            {
                pSocket->applyDestinationAddress(destAddress, destPort, !mStunServer.isNull(), mStunRefreshPeriodSecs, !mTurnServer.isNull());
                pMediaConnection->mRtpSendHostAddress = destAddress;
                pMediaConnection->mRtpAudioSendHostPort = destPort;
            }
        }

        // Connect RTCP Audio Socket
        if (pMediaConnection->mpRtcpAudioSocket)
        {
            OsNatDatagramSocket *pSocket = (OsNatDatagramSocket*)pMediaConnection->mpRtcpAudioSocket;
            if (pSocket->getBestDestinationAddress(destAddress, destPort))
            {
                pSocket->applyDestinationAddress(destAddress, destPort, !mStunServer.isNull(), mStunRefreshPeriodSecs, !mTurnServer.isNull()) ;                
                pMediaConnection->mRtcpAudioSendHostPort = destPort;                
            }            
        }

        // TODO:: Enable/Disable RTCP

#ifdef VIDEO
        // Connect RTP Video Socket
        if (pMediaConnection->mpRtpVideoSocket)
        {
            OsNatDatagramSocket *pSocket = (OsNatDatagramSocket*)pMediaConnection->mpRtpVideoSocket;
            if (pSocket->getBestDestinationAddress(destAddress, destPort))
            {
                pSocket->applyDestinationAddress(destAddress, destPort, !mStunServer.isNull(), mStunRefreshPeriodSecs, !mTurnServer.isNull()) ;                
                pMediaConnection->mRtpVideoSendHostPort = destPort;
            }            
        }

        // Connect RTCP Video Socket
        if (pMediaConnection->mpRtcpVideoSocket)
        {
            OsNatDatagramSocket *pSocket = (OsNatDatagramSocket*)pMediaConnection->mpRtcpVideoSocket;
            if (pSocket->getBestDestinationAddress(destAddress, destPort))
            {
                pSocket->applyDestinationAddress(destAddress, destPort, !mStunServer.isNull(), mStunRefreshPeriodSecs, !mTurnServer.isNull()) ;                
                pMediaConnection->mRtcpVideoSendHostPort = destPort;
            }            
        }

        // TODO:: Enable/Disable RTCP
#endif
    }
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

OsStatus SipXMediaInterfaceImpl::createRtpSocketPair(UtlString localAddress,
                                                    int localPort,
                                                    SIP_CONTACT_TYPE contactType,
                                                    OsDatagramSocket* &rtpSocket,
                                                    OsDatagramSocket* &rtcpSocket)
{
   int firstRtpPort;
   bool localPortGiven = (localPort != 0); // Does user specified the local port?
   UtlBoolean isMulticast = OsSocket::isMcastAddr(localAddress);

   if (!localPortGiven)
   {
      mpFactoryImpl->getNextRtpPort(localPort);
      firstRtpPort = localPort;
   }

   if (isMulticast)
   {
        rtpSocket = new OsMulticastSocket(localPort, localAddress,
                                          localPort, localAddress);
        rtcpSocket = new OsMulticastSocket(
                              localPort == 0 ? 0 : localPort + 1, localAddress,
                              localPort == 0 ? 0 : localPort + 1, localAddress);
   }
   else
   {
       rtpSocket = new OsNatDatagramSocket(0, NULL, localPort, localAddress, NULL);
       ((OsNatDatagramSocket*)rtpSocket)->enableTransparentReads(false);

       rtcpSocket = new OsNatDatagramSocket(0, NULL,localPort == 0 ? 0 : localPort+1,
                                            localAddress, NULL);
       ((OsNatDatagramSocket*)rtcpSocket)->enableTransparentReads(false);
   }

   // Validate local port is not auto-selecting.
   if (localPort != 0 && !localPortGiven)
   {
      // If either of the sockets are bad (e.g. already in use) or
      // if either have stuff on them to read (e.g. someone is
      // sending junk to the ports, look for another port pair
      while(!rtpSocket->isOk() ||
            !rtcpSocket->isOk() ||
             rtcpSocket->isReadyToRead() ||
             rtpSocket->isReadyToRead(60))
      {
            localPort +=2;
            // This should use mLastRtpPort instead of some
            // hardcoded MAX, but I do not think mLastRtpPort
            // is set correctly in all of the products.
            if(localPort > firstRtpPort + MAX_RTP_PORTS) 
            {
               OsSysLog::add(FAC_CP, PRI_ERR, 
                  "No available ports for RTP and RTCP in range %d - %d",
                  firstRtpPort, firstRtpPort + MAX_RTP_PORTS);
               break;  // time to give up
            }

            delete rtpSocket;
            delete rtcpSocket;
            if (isMulticast)
            {
               OsMulticastSocket* rtpSocket = new OsMulticastSocket(
                        localPort, localAddress,
                        localPort, localAddress);
               OsMulticastSocket* rtcpSocket = new OsMulticastSocket(
                        localPort == 0 ? 0 : localPort + 1, localAddress,
                        localPort == 0 ? 0 : localPort + 1, localAddress);
            }
            else
            {
               rtpSocket = new OsNatDatagramSocket(0, NULL, localPort, localAddress, NULL);
               ((OsNatDatagramSocket*)rtpSocket)->enableTransparentReads(false);

               rtcpSocket = new OsNatDatagramSocket(0, NULL,localPort == 0 ? 0 : localPort+1,
                                                   localAddress, NULL);
               ((OsNatDatagramSocket*)rtcpSocket)->enableTransparentReads(false);
            }
      }
   }

   // Did our sockets get created OK?
   if (!rtpSocket->isOk() || !rtcpSocket->isOk())
   {
       delete rtpSocket;
       delete rtcpSocket;
       return OS_NETWORK_UNAVAILABLE;
   }

   if (isMulticast)
   {
       // Set multicast options
       const unsigned char MC_HOP_COUNT = 8;
       ((OsMulticastSocket*)rtpSocket)->setHopCount(MC_HOP_COUNT);
       ((OsMulticastSocket*)rtcpSocket)->setHopCount(MC_HOP_COUNT);
       ((OsMulticastSocket*)rtpSocket)->setLoopback(false);
       ((OsMulticastSocket*)rtcpSocket)->setLoopback(false);
   }

   // Set a maximum on the buffers for the sockets so
   // that the network stack does not get swamped by early media
   // from the other side;
   {
      int sRtp, sRtcp, oRtp, oRtcp, optlen;

      sRtp = rtpSocket->getSocketDescriptor();
      sRtcp = rtcpSocket->getSocketDescriptor();

      optlen = sizeof(int);
      oRtp = 20000;
      setsockopt(sRtp, SOL_SOCKET, SO_RCVBUF, (char *) (&oRtp), optlen);
      oRtcp = 500;
      setsockopt(sRtcp, SOL_SOCKET, SO_RCVBUF, (char *) (&oRtcp), optlen);

      // Set the type of service (DiffServ code point) to low delay
      int tos = mExpeditedIpTos;
      
#ifndef WIN32 // [
            // Under Windows this options are supported under Win2000 only and
            // are not recommended to use.
      setsockopt (sRtp, IPPROTO_IP, IP_TOS, (char *)&tos, sizeof(int));
      setsockopt (sRtcp, IPPROTO_IP, IP_TOS, (char *)&tos, sizeof(int));
#else  // WIN32 ][
      // TODO:: Implement QoS  request under Windows.
#endif // WIN32 ]
   }

   if (!isMulticast)
   {
      NAT_BINDING rtpBindingMode = NO_BINDING;
      NAT_BINDING rtcpBindingMode = NO_BINDING;

      // Enable Stun if we have a stun server and either non-local contact type or 
      // ICE is enabled.
      if ((mStunServer.length() != 0) && ((contactType != SIP_CONTACT_LOCAL) || mEnableIce))
      {
         ((OsNatDatagramSocket*)rtpSocket)->enableStun(mStunServer, mStunPort, mStunRefreshPeriodSecs, 0, false) ;
         rtpBindingMode = STUN_BINDING;
      }

      // Enable Turn if we have a stun server and either non-local contact type or 
      // ICE is enabled.
      if ((mTurnServer.length() != 0) && ((contactType != SIP_CONTACT_LOCAL) || mEnableIce))
      {
         ((OsNatDatagramSocket*)rtpSocket)->enableTurn(mTurnServer, mTurnPort, 
                  mTurnRefreshPeriodSecs, mTurnUsername, mTurnPassword, false) ;

         if (rtpBindingMode == STUN_BINDING)
         {
            rtpBindingMode = STUN_TURN_BINDING;
         }
         else
         {
            rtpBindingMode = TURN_BINDING;
         }
      }

      // Enable Stun if we have a stun server and either non-local contact type or 
      // ICE is enabled.
      if ((mStunServer.length() != 0) && ((contactType != SIP_CONTACT_LOCAL) || mEnableIce))
      {
         ((OsNatDatagramSocket*)rtcpSocket)->enableStun(mStunServer, mStunPort, mStunRefreshPeriodSecs, 0, false) ;
         rtcpBindingMode = STUN_BINDING;
      }

      // Enable Turn if we have a stun server and either non-local contact type or 
      // ICE is enabled.
      if ((mTurnServer.length() != 0) && ((contactType != SIP_CONTACT_LOCAL) || mEnableIce))
      {
         ((OsNatDatagramSocket*)rtcpSocket)->enableTurn(mTurnServer, mTurnPort, 
                  mTurnRefreshPeriodSecs, mTurnUsername, mTurnPassword, false) ;

         if (rtcpBindingMode == STUN_BINDING)
         {
            rtcpBindingMode = STUN_TURN_BINDING;
         }
         else
         {
            rtcpBindingMode = TURN_BINDING;
         }
      }

      // wait until all sockets have results
      if (rtpBindingMode != NO_BINDING || rtcpBindingMode!= NO_BINDING)
      {
         bool bRepeat = true;
         while(bRepeat)
         {
            bRepeat = false;
            bRepeat |= ((OsNatDatagramSocket*)rtpSocket)->waitForBinding(rtpBindingMode, false);
            bRepeat |= ((OsNatDatagramSocket*)rtcpSocket)->waitForBinding(rtcpBindingMode, false);
            // repeat as long as one of sockets is waiting for result
         }
      }
   }

   return OS_SUCCESS;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


