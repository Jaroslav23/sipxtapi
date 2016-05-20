//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef XCpCallConnectionListener_h__
#define XCpCallConnectionListener_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS
class XCpAbstractCall;

/**
 * XCpCallConnectionListener is interface meant to be implemented by classes
 * wishing to observe changes in number of XSipConnections used.
 *
 * Instance of SipConnection is not passed in notifications.
 */
class XCpCallConnectionListener
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /* ============================ MANIPULATORS ============================== */

   /**
    * Called when a SipConnection is added to some abstract call.
    */
   virtual void onConnectionAdded(const UtlString& sSipCallId,
                                  XCpAbstractCall* pAbstractCall) = 0;

   /**
    * Called before a SipConnection is removed from some abstract call.
    */
   virtual void onConnectionRemoved(const UtlString& sSipCallId,
                                    XCpAbstractCall* pAbstractCall) = 0;

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

#endif // XCpCallConnectionListener_h__
