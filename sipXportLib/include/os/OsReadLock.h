//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _OsReadLock_h_
#define _OsReadLock_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsRWSyncBase.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Lock class allowing multiple simultaneous readers within a critical section
// This class uses OsRWSyncBase objects for synchronization.
// The constructor for the class automatically blocks until the OsRWSyncBase
// is acquired for reading. Similarly, the destructor automatically releases
// the lock. The easiest way to use this object as a guard is to create the
// object as a variable on the stack just before the section that needs to
// support multiple simultaneous readers. When the OsReadLock object goes out
// of scope, the reader lock will be automatically released.
// An example of this form of use is shown below.
// <p>
// <font face="courier">
// &nbsp;&nbsp;                      someMethod()                    <br>
// &nbsp;&nbsp;                      {                               <br>
// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;      OsReadLock lock(myRWMutex);   <br>
//                                                                   <br>
// &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;      < section allowing multiple
//                                       simultaneous readers >      <br>
// &nbsp;&nbsp;                      }                               </font>

class OsReadLock
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   OsReadLock(OsRWSyncBase& rRWSyncBase)
   : m_rRWSyncBase(rRWSyncBase)
   {
      rRWSyncBase.acquireRead();
   };
   //:Constructor

   virtual ~OsReadLock()
   {
      m_rRWSyncBase.releaseRead();
   };
   //:Destructor

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsRWSyncBase& m_rRWSyncBase;

   OsReadLock();
     //:Default constructor (not implemented for this class)

   OsReadLock(const OsReadLock& rOsReadLock);
     //:Copy constructor (not implemented for this class)

   OsReadLock& operator=(const OsReadLock& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsReadLock_h_
