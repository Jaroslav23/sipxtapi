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
#include <limits.h>

// APPLICATION INCLUDES
#include "utl/UtlInt.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
UtlContainableType UtlInt::TYPE = "UtlInt" ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor accepting an optional default value.
UtlInt::UtlInt(int value) : mValue(value)
{
} 


// Copy constructor
UtlInt::UtlInt(const UtlInt& rhs) : mValue(rhs.mValue)
{
}

// Destructor
UtlInt::~UtlInt()
{
}

UtlCopyableContainable* UtlInt::clone() const
{
   return new UtlInt(*this); 
}

/* ============================ OPERATORS ============================== */

// Prefix increment operator
UtlInt& UtlInt::operator++() {
    mValue++;
    return *this;
}

// Postfix increment operator
UtlInt UtlInt::operator++(int) {
    UtlInt temp = *this;
    ++*this;
    return temp;
}

// Prefix decrement operator
UtlInt& UtlInt::operator--() {
    mValue--;
    return *this;
}

// Postfix decrement operator
UtlInt UtlInt::operator--(int) {
    UtlInt temp = *this;
    --*this;
    return temp;
}

UtlInt& UtlInt::operator=(const UtlInt& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   // Assign values
   mValue = rhs.mValue;

   return *this;
}

/* ============================ MANIPULATORS ============================== */

int UtlInt::setValue(int iValue)
{
    int iOldValue = mValue ;
    mValue = iValue ;

    return iOldValue ;
}

/* ============================ ACCESSORS ================================= */

int UtlInt::getValue() const 
{
    return mValue ; 
}


unsigned UtlInt::hash() const
{
   return mValue ; 
}


UtlContainableType UtlInt::getContainableType() const
{
    return UtlInt::TYPE ;
}

/* ============================ INQUIRY =================================== */

int UtlInt::compareTo(UtlContainable const * inVal) const
{
   int result ; 
   
   if (inVal->isInstanceOf(UtlInt::TYPE))
    {
        UtlInt* temp = (UtlInt*)inVal ; 
        int inInt = temp -> getValue() ; 
        result = mValue - inInt ; 
    }
    else
    {
        result = INT_MAX ; 
    }

    return result ;
}


UtlBoolean UtlInt::isEqual(UtlContainable const * inVal) const
{
    return (compareTo(inVal) == 0) ; 
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
