/* Copyright (C) 2002  Andy Pennell
   2007,2008 - modifications by Jaroslav Libak

   You can use code snippets and source code downloads in your applications as long as

   * You keep all copyright notices in the code intact.
   * You do not sell or republish the code or it's associated article without the author's written agreement
   * You agree the code is provided as-is without any implied or express warranty. 
*/

#ifndef minidump_h__
#define minidump_h__

#if defined _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifndef __out_xcount
#define __out_xcount(x)
#endif

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#if _MSC_VER < 1300
#define DECLSPEC_DEPRECATED
// VC6: change this path to your Platform SDK headers
#include "M:\\dev7\\vs\\devtools\\common\\win32sdk\\include\\dbghelp.h"			// must be XP version of file
#else
// VC7: ships with updated headers
#include "dbghelp.h"
#endif

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
// STRUCTS
// TYPEDEFS

// based on dbghelp.h
typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
									CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
									CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
									CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
									);

/**
 * Class that can write minidumps during crash.
 */
class MDump
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
///@name Creators
//@{
   /**
    * Constructor.
    */
   MDump();

   /**
    * Destructor.
    */
   ~MDump();
//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
   /**
    * Function that can be used to write dumps at any point during
    * program execution, even if exception didn't occur.
    *
    * @return true if successful
    */
   static bool saveDump(const char* szDumpFilePath);

//@}
/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{
//@}
/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{
//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
	static const char* m_szAppName;   ///< application name

   /**
    * Exception handler, writes the minidump.
    *
    * @param pExceptionInfo Information about exception.
    */
	static LONG WINAPI topLevelFilter(struct _EXCEPTION_POINTERS *pExceptionInfo);

   static MINIDUMPWRITEDUMP getMiniDumpWriteDumpAddr();

   static bool dump(LONG& exResult, struct _EXCEPTION_POINTERS *pExceptionInfo = NULL, const char* szDumpFilePath = NULL);

};

#endif // _WIN32

#endif // minidump_h__
