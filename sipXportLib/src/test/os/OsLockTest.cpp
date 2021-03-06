//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <os/OsBSem.h>
#include <os/OsRWMutex.h>
#include <os/OsLock.h>
#include <os/OsReadLock.h>
#include <os/OsWriteLock.h>

class OsLockTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(OsLockTest);
    CPPUNIT_TEST(testLockBasicSemaphore);
    CPPUNIT_TEST(testLockMutex);
    CPPUNIT_TEST_SUITE_END();


public:

    /**
     * Locking of  basic semaphores
     */
    void testLockBasicSemaphore()
    {
        // Create a binary semaphore for use with an OsLock object
        OsBSem *pBSem = new OsBSem(OsBSem::Q_PRIORITY, OsBSem::FULL);

        // Acquire semaphore at the start of the block, release it on exit block
        {
            OsLock lock(*pBSem);
            // if this were a real guarded method, we'd do useful work here
            // destroying the OsReadLock variable that has been allocated on the stack
            // should release the reader lock automatically when we leave block
        }

        delete pBSem;
    }

    /**
     * Locking of mutex'es
     */
    void testLockMutex()
    {
        // Create an OsRWMutex for use with OsReadLock and OsWriteLock objects
        OsRWMutex *pRWMutex = new OsRWMutex(OsRWMutex::Q_FIFO);

        // Acquire read lock at the start of the block, release it on exit
        {
            OsReadLock lock(*pRWMutex);
        }

        // Acquire write lock at the start of the block, release it on exit
        {
            OsWriteLock lock(*pRWMutex);
        }

        delete pRWMutex;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(OsLockTest);

