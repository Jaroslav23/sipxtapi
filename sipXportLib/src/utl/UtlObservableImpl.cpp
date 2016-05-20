//
// Copyright (C) 2007 Jaroslav Libak
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsLock.h"
#include "utl/UtlObservableImpl.h"
#include "utl/UtlSListIterator.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

UtlObservableImpl::UtlObservableImpl()
: m_Mutex(OsMutex::Q_FIFO)
{

}

UtlObservableImpl::~UtlObservableImpl()
{
   OsLock lock(m_Mutex);
   mObservers.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

void UtlObservableImpl::registerObserver(UtlObserver* observer)
{
   OsLock lock(m_Mutex);
   mObservers.insert(new UtlPtr<UtlObserver>(observer));
}

void UtlObservableImpl::removeObserver(UtlObserver* observer)
{
   OsLock lock(m_Mutex);
   UtlPtr<UtlObserver> value(observer);
   mObservers.destroy(&value);
}

void UtlObservableImpl::removeAllObservers()
{
   OsLock lock(m_Mutex);
   mObservers.destroyAll();
}

void UtlObservableImpl::notify(int code, intptr_t userData)
{
   OsLock lock(m_Mutex);
   UtlSListIterator iterator(mObservers);
   UtlObserver* pObserver = NULL;
   UtlPtr<UtlObserver>* pContainer = NULL;

   while ((pContainer = (UtlPtr<UtlObserver>*)iterator()))
   {
      pObserver = pContainer->getValue();
      pObserver->onNotify(this, code, userData);
   }
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


