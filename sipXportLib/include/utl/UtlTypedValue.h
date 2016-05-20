//
// Copyright (C) 2007 Jaroslav Libak
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef UtlTypedValue_h__
#define UtlTypedValue_h__

// SYSTEM INCLUDES
#include "os/OsDefs.h"

// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "utl/UtlBool.h"
#include "utl/UtlContainable.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// UtlTypedValue is a template UtlContainable wrapper for any value
template <class T>
class UtlTypedValue : public UtlContainable
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   static const UtlContainableType TYPE;   /** < Class type used for runtime checking */ 

   /* ============================ CREATORS ================================== */

   /**
   * Constructor accepting an optional default value.
   */
   UtlTypedValue(T value)
   {
      m_value = value;
   }

   /**
   * Destructor
   */
   virtual ~UtlTypedValue()
   {
   }

   /* ============================ MANIPULATORS ============================== */

   /**
   * Set a new void ptr value for this object.
   *
   * @returns the old value
   */
   T setValue(T value)
   {
      T oldValue = m_value;
      m_value = value;
      return oldValue;
   }

   /* ============================ ACCESSORS ================================= */

   /**
   * Get the void ptr wrapped by this object.
   */
   T getValue() const
   {
      return m_value;
   }

   /**
   * Calculate a unique hash code for this object.  If the equals
   * operator returns true for another object, then both of those
   * objects must return the same hashcode.
   */    
   virtual unsigned hash() const
   {
      return (unsigned)m_value;
   }

   /**
   * Get the ContainableType for a UtlContainable derived class.
   */
   virtual UtlContainableType getContainableType() const
   {
      return UtlTypedValue::TYPE;
   }

   /* ============================ INQUIRY =================================== */

   /**
   * Compare the this object to another like-objects.  Results for 
   * designating a non-like object are undefined.
   *
   * @returns 0 if equal, < 0 if less then and >0 if greater.
   */
   virtual int compareTo(UtlContainable const* inVal) const
   {
      int result;

      if (inVal->isInstanceOf(UtlTypedValue::TYPE))
      {
         result = hash() - inVal->hash();
      }
      else
      {
         result = -1; 
      }

      return result;
   }


   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   T m_value;  /** < The type wrapped by this object */
};

template <class T>
const UtlContainableType UtlTypedValue<T>::TYPE = "UtlTypedValue";

/* ============================ INLINE METHODS ============================ */


#endif // UtlTypedValue_h__
