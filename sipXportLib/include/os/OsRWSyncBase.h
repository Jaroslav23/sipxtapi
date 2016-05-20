//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef OsRWSyncBase_h__
#define OsRWSyncBase_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * @brief Base class for the synchronization mechanisms in the OS abstraction layer
 */
class OsRWSyncBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

     /// Destructor
   virtual ~OsRWSyncBase() { };

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

     /// Assignment operator
   OsRWSyncBase& operator=(const OsRWSyncBase& rhs);

   virtual OsStatus acquireRead(void) = 0;
   //:Block (if necessary) until the task acquires the resource for reading
   // Multiple simultaneous readers are allowed.

   virtual OsStatus acquireWrite(void) = 0;
   //:Block (if necessary) until the task acquires the resource for writing
   // Only one writer at a time is allowed (and no readers).

   virtual OsStatus tryAcquireRead(void) = 0;
   //:Conditionally acquire the resource for reading (i.e., don't block)
   // Multiple simultaneous readers are allowed.
   // Return OS_BUSY if the resource is held for writing by some other task

   virtual OsStatus tryAcquireWrite(void) = 0;
   //:Conditionally acquire the resource for writing (i.e., don't block).
   // Only one writer at a time is allowed (and no readers).
   // Return OS_BUSY if the resource is held for writing by some other task
   // or if there are running readers.

   virtual OsStatus releaseRead(void) = 0;
   //:Release the resource for reading

   virtual OsStatus releaseWrite(void) = 0;
   //:Release the resource for writing

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
	 /// Default constructor
   OsRWSyncBase() { };

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
     /// Copy constructor
   OsRWSyncBase(const OsRWSyncBase& rRWSyncBase);
};

/* ============================ INLINE METHODS ============================ */

#endif // OsRWSyncBase_h__
