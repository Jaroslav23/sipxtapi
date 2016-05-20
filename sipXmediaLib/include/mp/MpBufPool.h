//  
// Copyright (C) 2006 SIPfoundry Inc. 
// Licensed by SIPfoundry under the LGPL license. 
//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
//  
// $$ 
////////////////////////////////////////////////////////////////////////////// 

#ifndef _INCLUDED_MPBUFPOOL_H // [
#define _INCLUDED_MPBUFPOOL_H

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "os/OsMutex.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

struct MpBuf;
struct MpBufList;

/// Pool of buffers.
class MpBufPool {

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
///@name Creators
//@{

    /// Creates pool with numBlocks in it. Each block have size blockSize.
    MpBufPool(unsigned blockSize, unsigned numInitialBlocks);

    /// Destroys pool.
    virtual
    ~MpBufPool();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{

    /// Get free block from pool.
    MpBuf *getBuffer();
    /**<
    * @return If there are no free blocks in pool invalid pointer returned.
    */

    /// Bring this buffer back to pool.
    void releaseBuffer(MpBuf *pBuffer);

//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

    /// Return size of the one block in the pool (in bytes).
    unsigned getBlockSize() const {return mBlockSize;};

    /// Return number of blocks in the pool.
    unsigned getNumBlocks() const {return mNumBlocks;};

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    /// Return pointer to the block, next to this.
    char *getNextBlock(char *pBlock) {return pBlock + mBlockSize;}
    
    void appendBufToList(MpBuf *pBuf, MpBufList **pTargetList);

    void attachNewPool(char* pNewPool);

    unsigned   mBlockSize;     ///< Size of one block in pool (in bytes).
    unsigned   mNumBlocks;     ///< Number of blocks in pool.
    unsigned   mPoolBytes;     ///< Size of all pool in bytes.
    MpBufList *mpFreeList;     ///< Begin of the free blocks list.
                               ///<  NULL if there are no free blocks availiable.
    MpBufList *mpPoolList;     ///< List of all pools that were allocated
    int mPoolListSize;         ///< Number of entries in pool list, for debugging
    OsMutex    mMutex;         ///< Mutex to avoid concurrent access to the pool.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};


#endif // _INCLUDED_MPBUFPOOL_H ]
