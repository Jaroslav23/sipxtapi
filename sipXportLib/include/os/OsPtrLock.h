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

#ifndef OsPtrLock_h__
#define OsPtrLock_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsSyncBase.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
 * OsPtrLock is a locking container of any pointer type. Doesn't delete the stored pointer.
 * T must inherit from OsSyncBase.
 */
template <class T>
class OsPtrLock
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   /**
    * Constructor accepting an optional default value.
    */
   OsPtrLock(T* pValue = NULL)
      : m_pValue(pValue)
   {
      superclassCheck();
      acquire();
   }

   /**
    * Destructor
    */
   virtual ~OsPtrLock()
   {
      release();
      m_pValue = NULL;
   }

   /**
    * Copy constructor for assigning instance of OsPtrLock. Copies the pointer
    * stored in OsPtrLock, and locks it again.
    */
   OsPtrLock(const OsPtrLock<T>& rhs)
   {
      superclassCheck();
      m_pValue = rhs.m_pValue;
      acquire();
   }

   /* ============================ MANIPULATORS ============================== */

   /** 
   * Assignment operator for assigning instance of OsPtrLock into OsPtrLock.
   * Locks the assigned object. Object will get unlocked during destruction or
   * another assignment.
   */
   OsPtrLock<T>& operator=(const OsPtrLock<T>& rhs)
   {
      if ((void*)&rhs == (void*)this)
      {
         // when self assignment do not lock or unlock anything
         return *this;
      }

      release(); // release old lock
      m_pValue = dynamic_cast<T*>(rhs.m_pValue);
      acquire(); // acquire new lock
      return *this;
   }

   /** 
    * Assignment operator for assigning instance of OsSyncBase into OsPtrLock.
    * Locks the assigned object. Object will get unlocked during destruction or
    * another assignment.
    */
   OsPtrLock& operator=(OsSyncBase* rhs)
   {
      release(); // release old lock
      m_pValue = dynamic_cast<T*>(rhs);
      acquire(); // acquire new lock
      return *this;
   }

   /**
    * -> operator returns contents of OsPtrLock.
    */
   T* operator->()
   {
      return m_pValue;
   }

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /**
    * Returns TRUE if pointer is NULL.
    */
   UtlBoolean isNull() const
   {
      return m_pValue == NULL ? TRUE : FALSE;
   }

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   void acquire()
   {
      if (m_pValue)
      {
         OsSyncBase *pSyncBase = dynamic_cast<OsSyncBase*>(m_pValue);
         if (pSyncBase)
         {
            // cast succeeded
            pSyncBase->acquire();
         }
      }
   }

   void release()
   {
      // unlock if not null
      if (m_pValue)
      {
         OsSyncBase *pSyncBase = dynamic_cast<OsSyncBase*>(m_pValue);
         if (pSyncBase)
         {
            // cast succeeded
            pSyncBase->release();
         }
      }
   }

   /** Helper method to limit possible T parameters to subclasses of OsSyncBase */
   void superclassCheck()
   {
      OsSyncBase* pSyncBase = (T*)NULL;
   }

   T* m_pValue;  /** < The void ptr wrapped by this object */
};

#endif // OsPtrLock_h__
