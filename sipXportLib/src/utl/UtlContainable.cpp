//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <string.h>

// APPLICATION INCLUDES
#include "utl/UtlContainable.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType UtlContainable::TYPE = "UtlContainable" ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor


// Copy constructor


// Destructor
UtlContainable::~UtlContainable()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

// RTTI
UtlBoolean UtlContainable::isInstanceOf(const UtlContainableType type) const
{
    return (   (type != NULL)
            && (getContainableType() != NULL)
            && (type == getContainableType())
            );
}

// Test this object to another like-object for equality. 
UtlBoolean UtlContainable::isEqual(UtlContainable const * compareContainable) const 
{
   return ( compareTo(compareContainable) == 0 );
}

/// Provides a hash function that uses the object pointer as the hash value.
unsigned UtlContainable::directHash() const
{
   return (unsigned) this;
}

/// Provides a hash function appropriate for null-terminated string values.
unsigned UtlContainable::stringHash(char const* value)
{
   /*
    * Adapted from
    *  "Data Structures and Algorithms with Object-Oriented Design Patterns in C++"
    *  by Bruno R. Preiss
    *  http://www.brpreiss.com/books/opus4/html/page218.html#proghash3c
    */
   unsigned const mask = 0x3f;
   unsigned const shift = 6;
   unsigned result = 0;
   while (!*value)
   {
      result = (result & mask) ^ (result << shift) ^ *value;
   }
   return result;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


