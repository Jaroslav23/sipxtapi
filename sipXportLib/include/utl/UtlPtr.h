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

#ifndef UtlPtr_h__
#define UtlPtr_h__

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

/// UtlPtr is a template UtlContainable wrapper for any pointer
template <class T>
class UtlPtr : public UtlContainable
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   static const UtlContainableType TYPE;   /** < Class type used for runtime checking */ 

   /* ============================ CREATORS ================================== */

   /**
   * Constructor accepting an optional default value.
   */
   UtlPtr(T* pValue = NULL, UtlBoolean deleteContent = FALSE)
   {
      mpValue = pValue;
      m_deleteContent = deleteContent;
   }

   /**
   * Destructor
   */
   virtual ~UtlPtr()
   {
      if (m_deleteContent)
      {
         delete mpValue;
         mpValue = NULL;
      }
   }

   /* ============================ MANIPULATORS ============================== */

   /**
   * Set a new void ptr value for this object.
   *
   * @returns the old value
   */
   T* setValue(T* pValue)
   {
      T* pOldValue = mpValue;
      mpValue = pValue;
      return pOldValue;
   }

   /* ============================ ACCESSORS ================================= */

   /**
   * Get the void ptr wrapped by this object.
   */
   T* getValue() const
   {
      return mpValue;
   }

   /**
   * Calculate a unique hash code for this object.  If the equals
   * operator returns true for another object, then both of those
   * objects must return the same hashcode.
   */    
   virtual unsigned hash() const
   {
      return (unsigned)mpValue;
   }

   /**
   * Get the ContainableType for a UtlContainable derived class.
   */
   virtual UtlContainableType getContainableType() const
   {
      return UtlPtr::TYPE;
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

      if (inVal->isInstanceOf(UtlPtr::TYPE))
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
   T* mpValue;  /** < The void ptr wrapped by this object */
   UtlBoolean m_deleteContent; ///< whether to delete content when being deleted
};

template <class T>
const UtlContainableType UtlPtr<T>::TYPE = "UtlPtr";

/* ============================ INLINE METHODS ============================ */


#endif // UtlPtr_h__
