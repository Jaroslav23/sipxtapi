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


#ifndef _OsDatagramSocket_h_
#define _OsDatagramSocket_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsSocket.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: Implements UDP version of OsSocket
// This class provides the implementation of the UDP datagram-based
// socket class which may be instantiated.

class OsDatagramSocket : public OsSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   
   OsDatagramSocket(int remoteHostPort, const char* remoteHostName,
                    int localHostPort = PORT_DEFAULT,
                    const char* localHostName = NULL);
     //:Constructor

   virtual
   ~OsDatagramSocket();
     //:Destructor


/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean reconnect();
   //: Sets up the connection again, assuming the connection failed

   void doConnect(int remotePort, const char* remoteHostName,
       UtlBoolean simulateConnect = FALSE);
   // Setup a connection with a remote host on the specified remote port
   //! param: remoteHostName - remote host to send datagram(s) in subsequent calls to write (overloaded version without host and port)
   //! param: remotePort - port on the remote host to send the datgram(s)
   //! param: simulateConnect - if TRUE do not call connect but default the to location (as in sendto) to the given host and port.  This is desirable to prevent the receiving of packets from other hosts from being filtered out by the IP stack.

   virtual int write(const char* buffer, int bufferLength);
   //: Blocking write to the socket
   // Write the characters in the given buffer to the socket.
   // This method will block until all of the bytes are written.
   //! param: buffer - the bytes to be written to the socket
   //! param: bufferLength - the number of bytes contained in buffer
   //! returns: the number of bytes actually written to the socket

   virtual int write(const char* buffer, int bufferLength,
       const char* ipAddress, int port);
   //: Blocking write to the socket
   // Write the characters in the given buffer to the socket.
   // This method will block until all of the bytes are written.
   //! param: buffer - the bytes to be written to the socket
   //! param: bufferLength - the number of bytes contained in buffer
   //! param: ipAddress - remote host to send datagram(s) to
   //! param: port - port on the remote host to send the datagram(s)
   //! returns: the number of bytes actually written to the socket

   virtual int read(char* buffer, int bufferLength);

/* ============================ ACCESSORS ================================= */
   virtual OsSocket::IpProtocolSocketType getIpProtocol() const;
   //: Returns the protocol type of this socket


   virtual void getRemoteHostIp(struct in_addr* remoteHostAddress,
                        int* remotePort = NULL);
   //:Return remote host ip address
   // Returns the ip address for the host on which the socket on the
   // other end of this socket is bound.

   virtual UtlBoolean getMappedIp(UtlString* ip, int* port) ;
     //:Return the external IP address for this socket.
     // OsStunDatagramSocket will return the NATted address if available,

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   OsDatagramSocket();
     //:Bare-bones constructor
     // Callers will need to call bind & doConnect on their own when this returns
     // (This allows for children to set socket options before the bind call)

   int bind(int localHostPortNum = PORT_DEFAULT, const char* localHost = NULL);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsDatagramSocket(const OsDatagramSocket& rOsDatagramSocket);
     //:Disable copy constructor

   OsDatagramSocket& operator=(const OsDatagramSocket& rhs);
     //:Disable assignment operator

   time_t mLastWriteErrorTime;
   int    mNumTotalWriteErrors;
   int    mNumRecentWriteErrors;

   UtlBoolean mSimulatedConnect;
   UtlBoolean mToSockaddrValid;
   struct sockaddr_in* mpToSockaddr;

   int ctorCommonCode();
   //:Common code for both constructors.

   virtual int writeTo(const char* buffer, int bufferLength);
   //: Blocking write to the socket, with simulated connection
   // Write the characters in the given buffer to the socket.
   // This method will block until all of the bytes are written.
   //! param: buffer - the bytes to be written to the socket
   //! param: bufferLength - the number of bytes contained in buffer
   //! returns: the number of bytes actually written to the socket

   virtual UtlBoolean getToSockaddr(void);

   void makeICMPPortUnreachableResistant();
};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsDatagramSocket_h_
