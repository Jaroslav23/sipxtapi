//
// Copyright (C) 2007 Jaroslav Libak
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef MpMediaTaskHelper_h__
#define MpMediaTaskHelper_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsTask.h>
#include <os/OsBSem.h>
#include <os/OsMutex.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
class MpFlowGraphBase;

// STRUCTS
// TYPEDEFS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

class MpMediaTaskHelper : public OsTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   MpMediaTaskHelper(const UtlString& name = "",
                     void* pArg = NULL,
                     const int priority = DEF_PRIO,
                     const int options = DEF_OPTIONS,
                     const int stackSize = DEF_STACKSIZE);

   virtual ~MpMediaTaskHelper();

/* ============================ MANIPULATORS ============================== */

   /**
    * Overriden method to unblock run() for shutdown
    */
   virtual void requestShutdown(void);

   /**
    * Process frames in all our flowgraphs
    */
   void processWork();

   /**
    * Wait until all frame processing is done. Used to synchronize multiple threads
    */
   void waitUntilDone();

   /**
    *  Add flowgraph for frame processing to this thread. Make sure to only
    *  add flowgraphs that are started!!
    */
   void addFlowgraphForProcessing(MpFlowGraphBase* pFlowGraph);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   /**
    * Task thread entry
    */
   virtual int run(void* pArg);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   OsMutex m_FlowGraphArrayGuard; ///< guard against concurrent access to array
   OsBSem m_workSemaphore; ///< binary semaphore to signal more work is available or shutdown
   OsBSem m_freeSemaphore; ///< binary semaphore to signal we are done with frame processing
   unsigned int m_flowGraphCount; ///< number of flowgraphs we have
   unsigned int m_flowGraphMax; ///< maximum number of flowgraphs without reallocation, can be exceeded
   MpFlowGraphBase** m_flowGraphArray; ///< array of pointers to flowgraphs
};

#endif // MpMediaTaskHelper_h__