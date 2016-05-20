/* Copyright (C) 2002  Andy Pennell
   2007,2008 - minor modifications by Jaroslav Libak

   You can use code snippets and source code downloads in your applications as long as

   * You keep all copyright notices in the code intact.
   * You do not sell or republish the code or it's associated article without the author's written agreement
   * You agree the code is provided as-is without any implied or express warranty. 
*/

#ifdef _WIN32

// SYSTEM INCLUDES
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>

// APPLICATION INCLUDES
#include "os/wnt/mdump.h"

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const char* MDump::m_szAppName = _T("SipXportLib");

// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

MDump::MDump()
{
   ::SetUnhandledExceptionFilter(topLevelFilter);
}

MDump::~MDump()
{
}

/* ============================ MANIPULATORS ============================== */

bool MDump::saveDump(const char* szDumpFilePath)
{
   LONG retval = 0;

   return dump(retval, NULL, szDumpFilePath);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

MINIDUMPWRITEDUMP MDump::getMiniDumpWriteDumpAddr()
{
   HWND hParent = NULL;						// find a better value for your app

   // firstly see if dbghelp.dll is around and has the function we need
   // look next to the EXE first, as the one in System32 might be old 
   // (e.g. Windows 2000)
   HMODULE hDll = NULL;
   TCHAR szDbgHelpPath[_MAX_PATH];

   if (GetModuleFileName( NULL, szDbgHelpPath, _MAX_PATH))
   {
      TCHAR *pSlash = strrchr(szDbgHelpPath, _T('\\'));
      if (pSlash)
      {
         strcpy(pSlash + 1, _T("DBGHELP.DLL"));
         hDll = ::LoadLibrary(szDbgHelpPath);
      }
   }

   if (hDll == NULL)
   {
      // load any version we can
      hDll = ::LoadLibrary(_T("DBGHELP.DLL"));
   }

   if (hDll)
   {
      MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress( hDll, "MiniDumpWriteDump");
      return pDump;
   }

   return NULL;
}

LONG MDump::topLevelFilter(struct _EXCEPTION_POINTERS *pExceptionInfo)
{
   LONG retval = EXCEPTION_CONTINUE_SEARCH;

   // ask the user if they want to save a dump file
   if (::MessageBox(NULL, _T("Fatal exception occurred in the program,\nwould you like to save a dump?\nDumps can help developers to fix bugs."), m_szAppName, MB_YESNO )==IDYES)
   {
      dump(retval, pExceptionInfo);
   }

   return retval;
}

bool MDump::dump(LONG& exResult, struct _EXCEPTION_POINTERS *pExceptionInfo, const char* szDumpFilePath)
{
   MINIDUMPWRITEDUMP pDump = getMiniDumpWriteDumpAddr();
   exResult = EXCEPTION_CONTINUE_SEARCH;

   if (pDump)
   {
      TCHAR szDumpPath[_MAX_PATH];
      memset(szDumpPath, 0, sizeof(szDumpPath));

      // select file path
      if (szDumpFilePath)
      {
         strcpy(szDumpPath, szDumpFilePath);
      }
      else
      {
         strcat(szDumpPath, m_szAppName);
         strcat(szDumpPath, _T(".dmp"));
      }

      // create the file
      HANDLE hFile = ::CreateFile(szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
         FILE_ATTRIBUTE_NORMAL, NULL);

      if (hFile != INVALID_HANDLE_VALUE)
      {
         _MINIDUMP_EXCEPTION_INFORMATION ExInfo;
         _MINIDUMP_EXCEPTION_INFORMATION *pExInfo = NULL;

         if (pExceptionInfo)
         {
            // only include exception info if it was supplied
            ExInfo.ThreadId = ::GetCurrentThreadId();
            ExInfo.ExceptionPointers = pExceptionInfo;
            ExInfo.ClientPointers = NULL;
            pExInfo = &ExInfo;
         }

         // write the dump
         BOOL bOK = pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithFullMemory, pExInfo, NULL, NULL);
         if (bOK)
         {
            exResult = EXCEPTION_EXECUTE_HANDLER;
            ::CloseHandle(hFile);
            return true;
         }

         ::CloseHandle(hFile);
      }
   }

   return false;
}

/* ============================ FUNCTIONS ================================= */


#endif // _WIN32
