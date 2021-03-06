//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _OsSSLServerSocket_h_
#define _OsSSLServerSocket_h_

#ifdef HAVE_SSL

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsConnectionSocket.h>
#include <os/OsServerSocket.h>
#include <os/OsSSLConnectionSocket.h>
#include <os/OsFS.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Implements Server for accepting TLS/SSL connections
class OsSSLServerSocket : public OsServerSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   /// Constructor to set up TCP socket server
   OsSSLServerSocket(int connectionQueueSize, /**< The maximum number of outstanding connection
                                               *   requests which are allowed before subsequent
                                               *   requests are turned away.*/ 
                     int serverPort=PORT_DEFAULT /**< The port on which the server will listen to
                                                  *   accept connection requests.
                                                  *   PORT_DEFAULT means let OS pick port. */
                     );
   /**
    * Sets the socket connection queue and starts listening on the
    * port for requests to connect.
    */

   /// Assignment operator
   OsSSLServerSocket& operator=(const OsSSLServerSocket& rhs);

  virtual
   ~OsSSLServerSocket();
     //:Destructor


/* ============================ MANIPULATORS ============================== */

   virtual OsConnectionSocket* accept();
   //:Blocking accept of next connection
   // Blocks and waits for the next TCP connection request.
   //!returns: Returns a socket connected to the client requesting the
   //!returns: connection.  If an error occurs returns NULL.


   void close();
   //: Close down the server

/* ============================ ACCESSORS ================================= */

   virtual int getLocalHostPort() const;
   //:Return the local port number
   // Returns the port to which this socket is bound on this host.

/* ============================ INQUIRY =================================== */

   virtual OsSocket::IpProtocolSocketType getIpProtocol() const;
   //: Returns the protocol type of this socket

   virtual UtlBoolean isOk() const;
   //: Server socket is in ready to accept incoming conection requests.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsSSLServerSocket(const OsSSLServerSocket& rOsSSLServerSocket);
     //:Disable copy constructor

   OsSSLServerSocket();
     //:Disable default constructor

};

/* ============================ INLINE METHODS ============================ */

#endif // HAVE_SSL

#endif  // _OsSSLServerSocket_h_

