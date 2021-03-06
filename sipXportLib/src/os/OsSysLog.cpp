//  
// Copyright (C) 2006 SIPez LLC. 
// Licensed to SIPfoundry under a Contributor Agreement. 
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
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>


// APPLICATION INCLUDES
#include "utl/UtlDefs.h"
#include "os/OsSysLog.h"
#include "os/OsSysLogMsg.h"
#include "os/OsSysLogTask.h"
#include "os/OsStatus.h"
#include "os/OsServerTask.h"
#include "os/OsDateTime.h"
#include "os/OsSocket.h"

// EXTERNAL FUNCTIONS
#if defined(_VXWORKS)
extern "C" char* strdup(const char* str);
#else
#ifndef WIN32
extern char* strdup(const char*) ;
#endif
#endif


// EXTERNAL VARIABLES
// CONSTANTS
#if defined(_WIN32)
     // Windows va_arg function does not take a const 
#    define OS_VA_ARG_CONST 
#elif defined(__pingtel_on_posix__)
     // Posix va_arg function takes a const
#    define OS_VA_ARG_CONST const
#elif _VXWORKS
     // Vxworks va_arg function does not take a const 
#    define OS_VA_ARG_CONST 
#else
#error Unsupported target platform.
#endif
// STATIC VARIABLE INITIALIZATIONS
OsSysLogTask* OsSysLog::spOsSysLogTask = NULL;
unsigned long OsSysLog::sEventCount = 0;
UtlString OsSysLog::sProcessId = "" ;
UtlString OsSysLog::sHostname = "" ;
OsSysLogPriority OsSysLog::spPriorities[FAC_MAX_FACILITY] ;
// Initial logging level is PRI_ERR.
OsSysLogPriority OsSysLog::sLoggingPriority = PRI_ERR ;
UtlBoolean OsSysLog::bPrioritiesInitialized = FALSE ;

// A static array of priority names uses for displaying log entries
const char* OsSysLog::sPriorityNames[] =
{
   "DEBUG",
   "INFO",
   "NOTICE",
   "WARNING",
   "ERR",
   "CRIT",
   "ALERT",
   "EMERG"
};

// LOCAL FUNCTIONS
static void mysprintf(UtlString& results, const char* format, ...) ;
static void myvsprintf(UtlString& results, const char* format, OS_VA_ARG_CONST va_list args) ;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */



// Initialize the log
OsStatus OsSysLog::initialize(const int   maxInMemoryLogEntries,
                              const char* processId,
                              const int   options /* = OPT_NONE */)
{
   OsStatus rc = OS_SUCCESS ;

   setLoggingPriority(OsSysLog::sLoggingPriority) ;

   if (spOsSysLogTask == NULL)
   {
      spOsSysLogTask = new OsSysLogTask(maxInMemoryLogEntries, options) ;
      sProcessId = processId ;
      OsSocket::getHostName(&sHostname) ;  
   }
   else
      rc = OS_UNSPECIFIED ;  

   return rc ;   
}
     
// Shutdown log
OsStatus OsSysLog::shutdown()
{
   OsSysLogTask* pTask = spOsSysLogTask ;
   spOsSysLogTask = NULL ;
   if (pTask != NULL)
   {
      pTask->flush() ;
      pTask->requestShutdown() ;
      delete pTask ;
   }

   return OS_SUCCESS ;
}

     
OsTimer* OsSysLog::getTimer()
{
    return spOsSysLogTask->getTimer();
}

// Set the output file target
OsStatus OsSysLog::setOutputFile(const int minFlushPeriod,
                                 const char* logfile)
{
   OsStatus rc = OS_SUCCESS ;

   if (spOsSysLogTask != NULL)
   {
      OsSysLogMsg msgFlush(OsSysLogMsg::SET_FLUSH_PERIOD, (void*) minFlushPeriod) ;
      spOsSysLogTask->postMessage(msgFlush) ;

	  if (logfile)
	  {
	     OsSysLogMsg msgSetFile(OsSysLogMsg::SET_FILE, SAFE_STRDUP(logfile)) ;
         spOsSysLogTask->postMessage(msgSetFile) ;
	  }
	  else
	  {
	     OsSysLogMsg msgSetFile(OsSysLogMsg::SET_FILE, NULL) ;
         spOsSysLogTask->postMessage(msgSetFile) ;
      }
   }
   else
      rc = OS_UNSPECIFIED ;
   
   return rc ;
}

OsStatus OsSysLog::getOutputFile(UtlString& logfile)
{
   if (spOsSysLogTask != NULL)
   {
      logfile = spOsSysLogTask->getLogFileName();
      return OS_SUCCESS;
   }

   return OS_FAILED;
}

OsStatus OsSysLog::setCallbackFunction(OsSysLogCallback pCallback)
{
   OsStatus rc = OS_SUCCESS ;

   if (spOsSysLogTask != NULL)
   {
      OsSysLogMsg msgSetCallback(OsSysLogMsg::SET_CALLBACK, (void*) pCallback);
      spOsSysLogTask->postMessage(msgSetCallback);
   }
   else
   {
      rc = OS_UNSPECIFIED;
   }

   return rc;
}


// Add a target output socket 
OsStatus OsSysLog::addOutputSocket(const char* remoteHost)
{
   OsStatus rc = OS_SUCCESS ;

   if (spOsSysLogTask != NULL)
   {
      OsSysLogMsg msg(OsSysLogMsg::ADD_SOCKET, (void*) SAFE_STRDUP(remoteHost)) ;
      spOsSysLogTask->postMessage(msg) ;
   }
   else
      rc = OS_UNSPECIFIED ;
   
   return rc ;
}

// Enable/disable console output
OsStatus OsSysLog::enableConsoleOutput(const UtlBoolean enable)
{
   OsStatus rc = OS_SUCCESS ;

   if (spOsSysLogTask != NULL)
   {      
      OsSysLogMsg msg(enable ? OsSysLogMsg::ENABLE_CONSOLE : 
            OsSysLogMsg::DISABLE_CONSOLE, NULL) ;
      spOsSysLogTask->postMessage(msg) ;
   }
   else
      rc = OS_UNSPECIFIED ;
   
   return rc ;
}

// Set the logging priority for all facilities
OsStatus OsSysLog::setLoggingPriority(const OsSysLogPriority priority)
{
   OsStatus rc = OS_SUCCESS ;

   int iNumFacilities = getNumFacilities() ;
   for (int i=0; i<iNumFacilities; i++)
   {
      spPriorities[i] = priority;
   }
   bPrioritiesInitialized = TRUE;

   sLoggingPriority = priority;
   if (spOsSysLogTask != NULL)
   {
      spOsSysLogTask->syslog(FAC_LOG, PRI_INFO, "logging priority changed to %s (%d)", OsSysLog::sPriorityNames[priority], priority) ;
   }
   
   return rc ;
}

// Set the logging priority for a specific facility
OsStatus OsSysLog::setLoggingPriorityForFacility(const OsSysLogFacility facility, 
                                                 const OsSysLogPriority priority)
{
   OsStatus rc = OS_SUCCESS ;
   int iNumFacilities = getNumFacilities() ;
   if ((facility >= 0) && (facility < iNumFacilities))      
   {
      spPriorities[facility] = priority ;

      if (spOsSysLogTask != NULL)
      {
         spOsSysLogTask->syslog(FAC_LOG, PRI_INFO, 
               "priority changed to %s for facility %s", 
               OsSysLog::sPriorityNames[priority],
               OsSysLog::sFacilityNames[facility]);

      }
   } 
   else
   {
      rc = OS_INVALID_ARGUMENT ;
   }
   return rc ;   
}


// Add a log entry
OsStatus OsSysLog::add(const char*            taskName,
                       const int              taskId,
                       const OsSysLogFacility facility,
                       const OsSysLogPriority priority,
                       const char*            format,
                                              ...)
{
   OsStatus rc = OS_UNSPECIFIED;

   // If the log has not been initialized, print everything
   if (spOsSysLogTask == NULL)
   {
      // Convert the variable arguments into a single string
      UtlString data ;
      va_list ap;
      va_start(ap, format); 
      myvsprintf(data, format, ap) ;
      data = escape(data) ;
      va_end(ap);

      // Display all of the data
      osPrintf("%s %s %s 0x%08X %s\n", 
            OsSysLog::sFacilityNames[facility], 
            OsSysLog::sPriorityNames[priority],
            (taskName == NULL) ? "" : taskName, 
            taskId,
            data.data()) ;

      rc = OS_SUCCESS ;
   }
   // Otherwise make sure we want to handle the log entry before we process
   // the variable arguments.
   else
   {
      if (willLog(facility, priority))
      {
         va_list ap;
         va_start(ap, format);
         rc = vadd(taskName, taskId, facility, priority, format, ap);
         va_end(ap);
      }  
   }
   return rc;
}


// Add a log entry
OsStatus OsSysLog::add(const OsSysLogFacility facility,
                       const OsSysLogPriority priority,
                       const char*            format,
                                              ...)
{
   OsStatus rc = OS_UNSPECIFIED;

   // If the log has not been initialized, print everything
   if (spOsSysLogTask != NULL)
   {
      if (willLog(facility, priority))
      {
         UtlString taskName ;
         int      taskId = 0 ;

         va_list ap;
         va_start(ap, format);

         OsTaskBase* pBase = OsTask::getCurrentTask() ;
         if (pBase != NULL)
         {
            taskName = pBase->getName() ;
            pBase->id(taskId) ;
         }
         
         rc = vadd(taskName.data(), taskId, facility, priority, format, ap);         
         va_end(ap);
      }  
   }
   else
      rc = OS_SUCCESS ;

   return rc;
}

// Add a log entry given a variable argument list
OsStatus OsSysLog::vadd(const char*            taskName,
                        const int              taskId,                        
                        const OsSysLogFacility facility,
                        const OsSysLogPriority priority,
                        const char*            format,
                        const va_list          ap)
{
   // If the log has not been initialized, print everything to the console
   if (spOsSysLogTask != NULL)
   {
      if (willLog(facility, priority))
      {
         UtlString logData;
         UtlString logEntry;
         myvsprintf(logData, format, ap) ;
         logData = escape(logData) ;

         OsTime timeNow;
         OsDateTime::getCurTime(timeNow); 
         OsDateTime logTime(timeNow);
         
         UtlString   strTime ;
         logTime.getIsoTimeStringZus(strTime) ;

         mysprintf(logEntry, "\"%s\":%d:%s:%s:%s:%s:%08X:%s:\"%s\"",
               strTime.data(),
               ++sEventCount,
               OsSysLog::sFacilityNames[facility], 
               OsSysLog::sPriorityNames[priority],
               sHostname.data(),
               (taskName == NULL) ? "" : taskName,
               taskId,
               sProcessId.data(),
               logData.data()) ;         

         // If the logger for some reason trys to log a message
         // there is a recursive problem.  Drop the message on the
         // floor for now.  This can occur if one of the os utilities
         // logs a message.
         if(strcmp("syslog", taskName) == 0)
         {
             // Just discard the log entry
             //
             // (rschaaf):
             // NOTE: Don't try to use osPrintf() to emit the log entry since this
             // can cause consternation for applications (e.g. CGIs) that expect to
             // use stdout for further processing.
         }
         else
         {
             char* szPtr = SAFE_STRDUP(logEntry.data()) ;
             OsSysLogMsg msg(OsSysLogMsg::LOG, szPtr) ;
             OsTime timeout(1000) ;
             if (spOsSysLogTask->postMessage(msg, timeout) != OS_SUCCESS)
             {
                 printf("OsSysLog jamed: %s\n", szPtr) ;
                 free(szPtr) ;
                 OsTask::yield() ;
              }
          }
       }
   }

   return OS_SUCCESS ;
}


// Clear the in memory log buffer
OsStatus OsSysLog::clearInMemoryLog()
{
   OsStatus rc = OS_SUCCESS ;

   if (spOsSysLogTask != NULL)
   {      
      spOsSysLogTask->clear() ;
   }
   else
      rc = OS_UNSPECIFIED ;
   
   return rc ;
}

// Flushes the in-memory circular buffer log to disk or an unbounded log file.
OsStatus OsSysLog::flush(const OsTime& rTimeout)
{
   OsStatus rc = OS_UNSPECIFIED ;

   if (spOsSysLogTask != NULL)
   {      
      rc = spOsSysLogTask->flush(rTimeout) ;
   }
   
   return rc ;
}

// Initialize the OsSysLog priority
void
OsSysLog::initSysLog(const OsSysLogFacility facility,
           const char* processID, 
           const char* logname, 
           const char* loglevel)
{

  initialize(0, processID) ;  
  setOutputFile(0, logname) ;  

   UtlString logLevel(loglevel); 
                                    
   struct tagPrioriotyLookupTable
   {
      const char*      pIdentity;
      OsSysLogPriority ePriority;
   };

   struct tagPrioriotyLookupTable lkupTable[] =
   {
      { "DEBUG",   PRI_DEBUG},
      { "INFO",    PRI_INFO},
      { "NOTICE",  PRI_NOTICE},
      { "WARNING", PRI_WARNING},
      { "ERR",     PRI_ERR},
      { "CRIT",    PRI_CRIT},
      { "ALERT",   PRI_ALERT},
      { "EMERG",   PRI_EMERG}
   };

   logLevel.toUpper();
   OsSysLogPriority priority = PRI_ERR;
   int iEntries = sizeof(lkupTable)/sizeof(struct tagPrioriotyLookupTable);
   for (int i=0; i<iEntries; i++)
   {
      if (logLevel == lkupTable[i].pIdentity)
      {
         priority = lkupTable[i].ePriority;
         osPrintf("Setting %s syslog level : %s\n", sFacilityNames[facility], lkupTable[i].pIdentity) ;
         setLoggingPriority(priority);
         add(facility, PRI_NOTICE, "Setting %s syslog level : %s", sFacilityNames[facility], lkupTable[i].pIdentity) ;
         break;
      }
   }

}

/* ============================ ACCESSORS ================================= */

// Get the max number of in memory log entries
OsStatus OsSysLog::getMaxInMemoryLogEntries(int& maxEntries)
{
   OsStatus rc = OS_SUCCESS ;

   if (spOsSysLogTask != NULL)
   {      
      spOsSysLogTask->getMaxEntries(maxEntries);
   }
   else
      rc = OS_UNSPECIFIED ;
   
   return rc ;
}

// Display the last numEntries log entries to the console
OsStatus OsSysLog::tailMemoryLog(const int numEntries)
{
   OsStatus rc = OS_SUCCESS ;

   if (spOsSysLogTask != NULL)
   {      
      OsSysLogMsg msg(OsSysLogMsg::TAIL, (void*) numEntries) ;
      spOsSysLogTask->postMessage(msg) ;
   }
   else
      rc = OS_UNSPECIFIED ;
   
   return rc ;
}

// Display the first numEntries log entries to the console
OsStatus OsSysLog::headMemoryLog(const int numEntries)
{
   OsStatus rc = OS_SUCCESS ;

   if (spOsSysLogTask != NULL)
   {      
      OsSysLogMsg msg(OsSysLogMsg::HEAD, (void*) numEntries) ;
      spOsSysLogTask->postMessage(msg) ;
   }
   else
      rc = OS_UNSPECIFIED ;
   
   return rc ;
}

// Get at most maxEntries log entries
OsStatus OsSysLog::getLogEntries(  const int maxEntries,
                                   char* entries[],
                                   int& actualEntries)
{
   OsStatus rc = OS_SUCCESS ;

   if (spOsSysLogTask != NULL)
   {
      spOsSysLogTask->getLogEntries(maxEntries, entries, actualEntries);
   }
   else
      rc = OS_UNSPECIFIED ;
   
   return rc ;
}


//:Parses a log string into its parts.
OsStatus OsSysLog::parseLogString(const char *szSource,
                                  UtlString& date,
                                  UtlString& eventCount,
                                  UtlString& facility,
                                  UtlString& priority,
                                  UtlString& hostname,
                                  UtlString& taskname,
                                  UtlString& taskId,
                                  UtlString& processId,
                                  UtlString& content)
{
   #define PS_DATE         0
   #define PS_EVENTCOUNT   1
   #define PS_FACILITY     2
   #define PS_PRIORITY     3
   #define PS_HOSTNAME     4
   #define PS_TASKNAME     5
   #define PS_TASKID       6
   #define PS_PROCESSID    7
   #define PS_CONTENT      8

   const char* pTraverse = szSource ;  // Traverses the source string
   UtlBoolean   bWithinQuote = FALSE;   // Are we within a quoted string?
   UtlBoolean   bEscapeNext = FALSE;    // The next char is an escape char.
   int         iParseState ;           // What are we parsing (PS_*)
   
   // Clean all of the passed objects
   date.remove(0) ;
   eventCount.remove(0) ;
   facility.remove(0) ;
   priority.remove(0) ;
   hostname.remove(0) ;
   taskname.remove(0) ;
   processId.remove(0) ;
   content.remove(0) ;

   // Loop through the source string and add characters to the appropriate
   // data object
   iParseState = PS_DATE ;
   while (*pTraverse)
   {
      switch (*pTraverse)
      {
         case ':':
            if (!bWithinQuote)
            {
               iParseState++ ;
               pTraverse++ ;
               continue ;
            }
            break ;
         case '"':
            if (!bEscapeNext)
            {
               bWithinQuote = !bWithinQuote;
               pTraverse++ ;
               continue ;
            }
            break ;
         case '\\':
            bEscapeNext = true ;
            break ;
      }

      switch (iParseState)
      {
         case PS_DATE:
            date.append(*pTraverse) ;
            break ;
         case PS_EVENTCOUNT:
            eventCount.append(*pTraverse) ;
            break ;
         case PS_FACILITY:
            facility.append(*pTraverse) ;
            break ;
         case PS_PRIORITY:
            priority.append(*pTraverse) ;
            break ;
         case PS_HOSTNAME:
            hostname.append(*pTraverse) ;
            break ;
         case PS_TASKNAME:
            taskname.append(*pTraverse) ;
            break ;
         case PS_TASKID:
            taskId.append(*pTraverse) ;
            break ;
         case PS_PROCESSID:
            processId.append(*pTraverse) ;
            break ;
         case PS_CONTENT:
            content.append(*pTraverse) ;
            break ;
      }

      pTraverse++ ;
   }

   content = unescape(content) ;

   return OS_SUCCESS ;
}
/* ============================ INQUIRY =================================== */

// Get the present logging priority
OsSysLogPriority OsSysLog::getLoggingPriority() 
{
   return sLoggingPriority ;  
}


// Get the logging priority for a specific facility
OsSysLogPriority OsSysLog::getLoggingPriorityForFacility(const OsSysLogFacility facility) 
{
   OsSysLogPriority rc = PRI_DEBUG ;

   if ((facility >=0) && (facility < getNumFacilities())) 
   {
      rc = spPriorities[facility] ;
   } 
   return rc ;     
}


// Determine if a message will be logged given a facility and priority
UtlBoolean OsSysLog::willLog(OsSysLogFacility facility, 
                            OsSysLogPriority priority) 
{
   UtlBoolean bwillLog = false ;
   if ((facility >=0) && (facility < getNumFacilities()))
   {
      initializePriorities();
      bwillLog = (spPriorities[facility] <= priority) ;
   }

   return bwillLog ;
}

void OsSysLog::initializePriorities()
{
    if (bPrioritiesInitialized == FALSE)
    {
        setLoggingPriority(PRI_ERR);
    }
}

// Return the max number of facilities
int OsSysLog::getNumFacilities()
{
   return (FAC_MAX_FACILITY) ;
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Assignment operator
OsSysLog& 
OsSysLog::operator=(const OsSysLog& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

// Constructor
OsSysLog::OsSysLog()
{
}

// Copy constructor
OsSysLog::OsSysLog(const OsSysLog& rOsSysLog)
{
}

// Destructor
OsSysLog::~OsSysLog()
{
}

// Returns an escaped version of the specified source string
UtlString OsSysLog::escape(const UtlString& source)
{
   UtlString    results ;        
   const char* pStart = source.data() ;
   const char* pTraverse = pStart ;
   const char* pLast = pStart ;

   while (*pTraverse)
   {
      switch (*pTraverse)
      {
         case '\\':
            // Copy old data
            if (pLast < pTraverse)
            {
               results.append(pLast, pTraverse-pLast);               
            }
            pLast = pTraverse + 1 ;

            // Add escaped Text
            results.append("\\\\") ;
            break ;
         case '\r':
            // Copy old data
            if (pLast < pTraverse)
            {
               results.append(pLast, pTraverse-pLast);               
            }
            pLast = pTraverse + 1 ;

            // Add escaped Text
            results.append("\\r") ;
            break ;
         case '\n':
            // Copy old data
            if (pLast < pTraverse)
            {
               results.append(pLast, pTraverse-pLast);               
            }
            pLast = pTraverse + 1 ;

            // Add escaped Text
            results.append("\\n") ;
            break ;
         case '\"':
            // Copy old data
            if (pLast < pTraverse)
            {
               results.append(pLast, pTraverse-pLast);
            }
            pLast = pTraverse + 1 ;

            // Add escaped Text
            results.append("\\\"") ;
            break ;            
         default:
            break ;
      }
      pTraverse++ ;
   }

   // if nothing to escape, short-circuit
   if (pLast == pStart)
   {
      return source ;
   } 
   else if (pLast < pTraverse)
   {
      results.append(pLast, pTraverse-pLast);
   }
  
   return results ;
}


// Unescapes previously escaped Quotes and CrLfs 
UtlString OsSysLog::unescape(const UtlString& source)
{
   UtlString    results ;        
   const char* pStart = source.data() ;
   const char* pTraverse = pStart ;
   const char* pLast = pStart ;
   UtlBoolean   bLastWasEscapeChar = false;

   while (*pTraverse)
   {
      if (bLastWasEscapeChar)
      {
         switch (*pTraverse)
         {
            case '\\':
            case '"':
               if (pLast < pTraverse)
               {
                  results.append(pLast, pTraverse-pLast-1);               
               }
               pLast = pTraverse + 1 ;
               results.append(*pTraverse) ;
               break ;
            case 'r':
               if (pLast < pTraverse)
               {
                  results.append(pLast, pTraverse-pLast-1);               
               }
               pLast = pTraverse + 1 ;
               results.append("\r") ;
               break ;
            case 'n':
               if (pLast < pTraverse)
               {
                  results.append(pLast, pTraverse-pLast-1);               
               }
               pLast = pTraverse + 1 ;
               results.append("\n") ;
               break;
            default:
               // Invalid/Illegal Escape Character
               break ;
         }
         bLastWasEscapeChar = false ;
      }
      else
      {
         if (*pTraverse == '\\')
         {
            bLastWasEscapeChar = true ;
         }
      }

      pTraverse++ ;
   }

   // if nothing to escape, short-circuit
   if (pLast == pStart)
   {
      return source ;
   } 
   else if (pLast < pTraverse)
   {
      results.append(pLast, (pTraverse-1)-pLast);
   }  
  
   return results ;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

// a version of sprintf that stores results into an UtlString
void mysprintf(UtlString& results, const char* format, ...)
{
   va_list args;
   va_start(args, format);

   myvsprintf(results, format, args) ;

   va_end(args);
}


// a version of vsprintf that stores results in an UtlString
void myvsprintf(UtlString& results, const char* format, OS_VA_ARG_CONST va_list args)
{    
    /* Guess we need no more than 384 bytes. */
    int n, size = 384;
    char *p;

    results.remove(0) ;
    p = (char*) malloc(size) ;
     
    while (p != NULL)
    {
        /* Try to print in the allocated space. */
#ifdef _WIN32
        n = _vsnprintf (p, size, format, args);
#else
        n = vsnprintf (p, size, format, args);
#endif

        /* If that worked, return the string. */
        if (n > -1 && n < size)
        {
            break;
        }
        /* Else try again with more space. */
        if (n > -1)    /* glibc 2.1 */
            size = n+1; /* precisely what is needed */
        else           /* glibc 2.0 */
            size *= 2;  /* twice the old size */

        if ((p = (char*) realloc (p, size)) == NULL)
        {
            break;
        }
    }

    if (p != NULL)
    {
        results.append(p) ;
        free(p) ;
    }
}

OsStackTraceLogger::OsStackTraceLogger(const OsSysLogFacility facility,
                    const OsSysLogPriority priority,
                    const char* methodName) : 
    mMethodName(methodName),
    mFacility(facility),
    mPriority(priority)
{
    OsTask::getCurrentTaskId(mTid);
    OsSysLog::add(mFacility, mPriority, "ENTER FUNC (tid=%d) %s\n",
        mTid, methodName);
}

OsStackTraceLogger::~OsStackTraceLogger()
{
    OsSysLog::add(mFacility, mPriority, "EXIT  FUNC (tid=%d) %s\n",
        mTid, mMethodName.data());
}

OsStackTraceLogger::OsStackTraceLogger(const OsSysLogFacility facility,
                    const OsSysLogPriority priority,
                    const char* methodName,
                    const OsStackTraceLogger& oneBackInStack) : 
    mMethodName(methodName),
    mFacility(facility),
    mPriority(priority)
{
    mMethodName = UtlString(oneBackInStack.mMethodName) + UtlString("->") + mMethodName;
    OsTask::getCurrentTaskId(mTid);
    OsSysLog::add(mFacility, mPriority, "ENTER FUNC (tid=%d) %s\n",
        mTid, mMethodName.data());
}
