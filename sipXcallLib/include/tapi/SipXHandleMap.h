//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _SipXHandleMap_h_
#define _SipXHandleMap_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsMutex.h"
#include "utl/UtlHashMap.h"


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef unsigned int SIPXHANDLE;
// FORWARD DECLARATIONS

/**
 * SipXHandleMap provides a very simple container that associates a void*
 * with a handle value.  The handle value is a unique incrementing number.
 * In theory, we could get collisions if the numbers wrap, however, this
 * is not designed for that type of call volume (millions of call per
 * hour?)
 * <p>
 * Generally, use the allocHandle, removeHandle, and findHandle methods.
 * lock() and unlock() methods are also provided for external iterators.
 */
class SipXHandleMap : public UtlHashMap
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

   /**
    * Default constructor
    */
   SipXHandleMap(int startingHandle = 1, int avoidHandle = 0);

   /**
    * Destructor
    */
   virtual ~SipXHandleMap();

/* ============================ MANIPULATORS ============================== */

    
    /**
     * Adds a reference count to the handle lock.  In this way, removeHandle is 
     * guarded against removal while a handle is in use.
     * releaseHandleRef decrements the reference count.
     * addHandleRef should only be used in very specific
     * cases, when the handle might become invalid before it is needed again.
     */
     void addHandleRef(SIPXHANDLE handle);
     
    /**
     * Allocate a unique handle and associate the designed pData value
     * with that handle.
     *
     * @param pData Data to be associated with the newly allocated handle
     */
     UtlBoolean allocHandle(SIPXHANDLE& hHandle, const void* pData);


    /**
     * Find the data associated with the designated handle and return it.
     */
     const void* findHandle(SIPXHANDLE handle) const;

    /**
     * Remove the handle and data associated with it from the map.
     */
    const void* removeHandle(SIPXHANDLE handle);

    /**
     * Lock/guard access to the allocHandle, findHandle, and removeHandle
     * routines.  This is called automatically for those routines, however,
     * should be called explicitly if using an external iterator on the map.
     */
    void lock() const;

    /**
     * Unlock access to the allocHandle, findHandle, and removeHandle
     * routines.  This is called automatically for those routines, however,
     * should be called explicitly if using an external iterator on the map.
     */
    void unlock() const;
/* ============================ ACCESSORS ================================= */

    void dump();

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
    mutable OsMutex mLock;       /**< Locked used for addEntry and removeEntry */
    SIPXHANDLE mNextHandle; /**< Next available handle index */
    SIPXHANDLE mAvoidHandle; /**< Handle to avoid */

    
/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:
    UtlHashMap mLockCountHash;
    
    /**
     * Decrements reference count for handle locking.
     * This should only be called from withing 
     * removeHandle. 
     * So, removeHandle will only actually remove the handle and
     * return a pointer when there are no outstanding locks.
     */
    void releaseHandleRef(SIPXHANDLE handle);
    
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipXHandleMap_h_
