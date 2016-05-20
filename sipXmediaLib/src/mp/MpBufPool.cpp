//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "mp/MpBufPool.h"
#include "mp/MpBuf.h"
#include "os/OsLock.h"

// DEFINES
#if defined(MPBUF_DEBUG) || defined(_DEBUG) // [
#  define MPBUF_CLEAR_EXIT_CHECK
#endif // MPBUF_DEBUG || _DEBUG ]

/// Round 'val' to be multiply of 'align'.
#define MP_ALIGN(val, align) ((((val)+((align)-1))/(align))*(align)) 

/// @brief Block size will be aligned to this value. Other bound will be aligned
//  to it later.
/**
 * Align block size to 4 bytes to avoid problems on ARM.
 */
#define MP_ALIGN_SIZE 4

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/// Class for internal MpBufPool use.
/**
*  This class provides single linked list interface for MpBuf class. It uses
*  MpBuf::mpPool to store pointer to next buffer.
*/
struct MpBufList : public MpBuf
{
    friend class MpBufPool;
public:

    /// Get buffer next to current.
    MpBufList *getNextBuf()
    {
       return (MpBufList*)mpPool;
    }

    /// Set buffer next to current.
    void setNextBuf(MpBuf *pNext)
    {
       mpPool = (MpBufPool*)pNext;
    }

private:

    /// Disable copy (and other) constructor.
    MpBufList(const MpBuf &);
    /**<
    * This struct will be initialized by init() member.
    */

    /// Disable assignment operator.
    MpBufList &operator=(const MpBuf &);
    /**<
    * Buffers may be copied. But do we need this?
    */
};

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MpBufPool::MpBufPool(unsigned blockSize, unsigned numInitialBlocks)
: mBlockSize(MP_ALIGN(blockSize,MP_ALIGN_SIZE))
, mNumBlocks(numInitialBlocks + 1)
, mPoolBytes(mBlockSize*mNumBlocks)
, mpFreeList(NULL)
, mpPoolList(NULL)
, mPoolListSize(0)
, mMutex(OsMutex::Q_PRIORITY)
{
    assert(mBlockSize >= sizeof(MpBuf));
    assert(mNumBlocks > 1);

    char* pFirstPool = new char[mPoolBytes];
    attachNewPool(pFirstPool);
}

MpBufPool::~MpBufPool()
{
   MpBufList* pNext = NULL;
   MpBufList* pPoolListTmp = mpPoolList;
#ifdef MPBUF_CLEAR_EXIT_CHECK
   // print content of all buffers in all pools
   while(pPoolListTmp)
   {
      // get pointer to next pool
      pNext = pPoolListTmp->getNextBuf();

      // skip the 1st buffer
      char *pBlock = getNextBlock((char*)pPoolListTmp);
      for (unsigned int i = 1; i < mNumBlocks; i++)
      {
         MpBuf *pBuf = (MpBuf *)pBlock;
         if (pBuf->mRefCounter > 0 || pBuf->mpPool == this)
         {
            osPrintf( "Buffer %d from pool %x was not correctly freed!!!\n"
               , (pBlock-(char*)pPoolListTmp)/mBlockSize
               , this);
         }
         pBlock = getNextBlock(pBlock);
      }

      pPoolListTmp = pNext;
   }
#endif

    // delete all pools
    pNext = NULL;
    while(mpPoolList)
    {
       pNext = mpPoolList->getNextBuf();
       delete[] mpPoolList;
       mpPoolList = pNext;
    }
}

/* ============================ MANIPULATORS ============================== */

MpBuf *MpBufPool::getBuffer()
{
   OsLock lock(mMutex);

   // No free blocks found, allocate new pool
   if (mpFreeList == NULL)
   {
      char* pNewPool = new char[mPoolBytes];
      attachNewPool(pNewPool);
      // now we have new buffers in mpFreeList
      assert(mpFreeList);
   }

   MpBuf *pFreeBuffer = mpFreeList;
   mpFreeList = mpFreeList->getNextBuf();
   pFreeBuffer->mpPool = this;

   return pFreeBuffer;
}

void MpBufPool::releaseBuffer(MpBuf *pBuffer)
{
    OsLock lock(mMutex);
    assert(pBuffer->mRefCounter == 0);

    // This check is need cause we don't synchronize MpBuf's reference counter.
    // See note in MpBuf::detach().
    if (pBuffer->mpPool == this)
    {
        appendBufToList(pBuffer, &mpFreeList);
    }
    else
    {
#ifdef MPBUF_DEBUG
        osPrintf("Error: freeing buffer with wrong pool or freeing buffer twice!");
#endif
        assert(false);
    }
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

void MpBufPool::appendBufToList(MpBuf *pBuffer, MpBufList **pTargetList)
{
    ((MpBufList*)pBuffer)->setNextBuf(*pTargetList);
    *pTargetList = (MpBufList*)pBuffer;
}

void MpBufPool::attachNewPool(char* pNewPool)
{
   assert(pNewPool);
   memset(pNewPool, 0, mPoolBytes);
   int bufferId = mPoolListSize*mNumBlocks;

   // put first buffer to the list of all pools for easy deletion
   appendBufToList((MpBufList*)pNewPool, &mpPoolList);
   mPoolListSize++;

   // Init buffers, we will use numBlocks-1, so skip the 1st buffer
   char *pBlock = getNextBlock(pNewPool);
   for (unsigned int i = 1; i < mNumBlocks; i++)
   {
      MpBuf *pBuf = (MpBufList*)pBlock;

      pBuf->mBufferId = bufferId;
      // Add buffer to the end of free list
      appendBufToList(pBuf, &mpFreeList);

      // Jump to next block
      pBlock = getNextBlock(pBlock);
      bufferId++; // advance buffer ID
   }
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
