//
// Copyright (C) 2007 Jaroslav Libak
//
// $$
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsLock.h>
#include <mp/MpMediaTaskHelper.h>
#include <mp/MpFlowGraphBase.h>

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

MpMediaTaskHelper::MpMediaTaskHelper(const UtlString& name /*= ""*/,
                                     void* pArg /*= NULL*/,
                                     const int priority /*= DEF_PRIO*/,
                                     const int options /*= DEF_OPTIONS*/,
                                     const int stackSize /*= DEF_STACKSIZE*/)
: OsTask(name, pArg, priority, options, stackSize)
, m_workSemaphore(0, OsBSem::EMPTY)
, m_freeSemaphore(0, OsBSem::EMPTY)
, m_FlowGraphArrayGuard(OsMutex::Q_FIFO)
, m_flowGraphCount(0)
, m_flowGraphMax(100)
, m_flowGraphArray(NULL)
{
   m_flowGraphArray = (MpFlowGraphBase**)malloc(sizeof(MpFlowGraphBase*)*m_flowGraphMax);
   assert(m_flowGraphArray);
}

MpMediaTaskHelper::~MpMediaTaskHelper()
{
   OsLock lock(m_FlowGraphArrayGuard);

   if (m_flowGraphArray)
   {
      free(m_flowGraphArray);
      m_flowGraphArray = NULL;
      m_flowGraphCount = 0;
   }
}

void MpMediaTaskHelper::requestShutdown(void)
{
   OsTaskBase::requestShutdown();
   m_workSemaphore.release();
}

void MpMediaTaskHelper::addFlowgraphForProcessing(MpFlowGraphBase* pFlowGraph)
{
   OsLock lock(m_FlowGraphArrayGuard);

   if (m_flowGraphArray)
   {
      if (m_flowGraphCount < m_flowGraphMax)
      {
         m_flowGraphArray[m_flowGraphCount++] = pFlowGraph;
      }
      else
      {
         m_flowGraphMax = m_flowGraphMax * 2;
         m_flowGraphArray = (MpFlowGraphBase**)realloc(m_flowGraphArray, sizeof(MpFlowGraphBase*)*m_flowGraphMax);
         assert(m_flowGraphArray);
         // extend array
         if (m_flowGraphArray)
         {
            // if array is ok, use it
            m_flowGraphArray[m_flowGraphCount++] = pFlowGraph;
         }
      }
   }      
}

void MpMediaTaskHelper::processWork()
{
   m_workSemaphore.release();
}

void MpMediaTaskHelper::waitUntilDone()
{
   m_freeSemaphore.acquire();
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

int MpMediaTaskHelper::run(void* pArg)
{
   // repeat as long as we are not shutting down
   while (!isShuttingDown())
   {
      {
         OsLock lock(m_FlowGraphArrayGuard);

         if (m_flowGraphArray)
         {
            for (unsigned int i = 0; i < m_flowGraphCount; i++)
            {
               m_flowGraphArray[i]->processNextFrame();
               m_flowGraphArray[i] = NULL;
            }
            m_flowGraphCount = 0;
         }
      }
      // we are now done with work
      m_freeSemaphore.release();

      m_workSemaphore.acquire();
   }
   
   return 0;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */


