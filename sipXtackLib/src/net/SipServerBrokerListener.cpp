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
#include <os/OsPtrMsg.h>
#include "net/SipServerBrokerListener.h"
#include <net/SipUserAgent.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

UtlBoolean SipServerBrokerListener::handleMessage(OsMsg& eventMessage)
{
   UtlBoolean bRet(FALSE);
   int msgType = eventMessage.getMsgType();
   int msgSubType = eventMessage.getMsgSubType();
   OsPtrMsg *pPtrMsg = NULL;


   if (msgType == OsMsg::OS_EVENT)
   {
      // if we are receiving this message, a socket an accept has
      // occurred, and the socket is being sent to us in this message
      if (msgSubType == SIP_SERVER_BROKER_NOTIFY)
      {
         // unpackage the client socket
         pPtrMsg = dynamic_cast<OsPtrMsg*>(&eventMessage);

         assert(pPtrMsg);

         OsConnectionSocket* clientSocket = reinterpret_cast<OsConnectionSocket*>(pPtrMsg->getPtr());
         assert (clientSocket);

         SipClient* client = NULL;
         client = new SipClient(clientSocket);
         if(mpOwner->mSipUserAgent)
         {
            client->setUserAgent(mpOwner->mSipUserAgent);
         }

         UtlString hostAddress;
         int hostPort;
         clientSocket->getRemoteHostIp(&hostAddress, &hostPort);

         OsSysLog::add(FAC_SIP, PRI_DEBUG, "Sip%sServer::run client: %p %s:%d",
            mpOwner->mProtocolString.data(), client, hostAddress.data(), hostPort);

         UtlBoolean clientStarted = client->start();
         if(!clientStarted)
         {
            OsSysLog::add(FAC_SIP, PRI_ERR, "SIP %s Client failed to start", mpOwner->mProtocolString.data());
         }
         mpOwner->addClient(client);
         bRet = TRUE;
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_ERR, "SIP %s Client received spurious message", mpOwner->mProtocolString.data());
      }
   }

   return bRet;
}
