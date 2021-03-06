//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////


#ifndef _UtlSortedList_h_
#define _UtlSortedList_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlList.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlContainable;

/**
 * UtlSortedList is a list that is always sorted
 * 
 * Most list accessors and inquiry methods are performed by equality as 
 * opposed to by referencing (pointers).  For example, a list.contains(obj) 
 * call will loop through all of the list objects and test equality by calling
 * the isEquals(...) method.  A  list.containsReference(obj) call will search 
 * for a pointer match.
 * 
 * @see UtlSListIterator
 * @see UtlList
 * @see UtlContainer
 * @see UtlContainable
 */
class UtlSortedList : public UtlList
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    static const UtlContainableType TYPE;

/* ============================ CREATORS ================================== */

   /**
    * Default Constructor
    */
   UtlSortedList();

/* ============================ MANIPULATORS ============================== */

    /**
     * Inserts the designated containable object into the list
     * 
     * @return the object if successful, otherwise null
     */
    virtual UtlContainable* insert(UtlContainable* obj);

    /**
     * Remove the designated object by equality (as opposed to by reference).
     */
    virtual UtlContainable* remove(const UtlContainable*);


/* ============================ ACCESSORS ================================= */

    /**
     * Find the first occurrence of the designated object by equality (as 
     * opposed to by reference).
     */
    virtual UtlContainable* find(const UtlContainable*) const;

/* ============================ INQUIRY =================================== */

    /**
     * Return the list position of the designated object or UTL_NOT_FOUND  if
     * not found.
     */
    virtual size_t index(const UtlContainable* obj) const;

    /**
     * Return the number of occurrences of the designated object
     */
    virtual size_t occurrencesOf(const UtlContainable* obj) const;


    /**
     * Get the ContainableType for the list as a contained object.
     */
    UtlContainableType getContainableType() const;
    

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    friend class UtlSortedListIterator;
    
    typedef enum {POSITION, EXACTLY} MatchType;

    /**
     * Return the first UtlLink which is greater or equal to the designated
     * object, or NULL if not found.
     *
     * The caller must hold the mContainerLock
     */
    UtlLink* findNode(UtlLink* starting, MatchType match, const UtlContainable* obj) const;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    /* for now, don't provide a copy constructor */
    UtlSortedList(UtlSortedList& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif    // _UtlSortedList_h_


