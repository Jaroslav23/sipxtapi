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
#define WIN32_LEAN_AND_MEAN
#include <assert.h>
#include <windows.h>
#include <tlhelp32.h>

// APPLICATION INCLUDES
#include "os/wnt/OsProcessIteratorWnt.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsProcessIteratorWnt::OsProcessIteratorWnt() :
hProcessSnapshot(INVALID_HANDLE_VALUE)
{
}

// Destructor
OsProcessIteratorWnt::~OsProcessIteratorWnt()
{
   if (hProcessSnapshot != (HANDLE)INVALID_HANDLE_VALUE)
      CloseHandle(hProcessSnapshot);
}

/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */
OsStatus OsProcessIteratorWnt::findFirst(OsProcess &rProcess)
{
    OsStatus retval = OS_FAILED;
    
    if (hProcessSnapshot != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hProcessSnapshot);
    }
    
    hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if (hProcessSnapshot != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 lppe;
        lppe.dwSize = sizeof(PROCESSENTRY32);
        //now actuall go find the process
        if (Process32First(hProcessSnapshot,&lppe))
        {
            if (GetLastError() != ERROR_NO_MORE_FILES)
            {
                rProcess.mPID           = lppe.th32ProcessID;
                rProcess.mParentPID     = lppe.th32ParentProcessID;
                rProcess.mProcessName   = lppe.szExeFile;

                retval = OS_SUCCESS;
            }
            else
                retval = OS_FAILED;
        }
    }

    return retval;
}

OsStatus OsProcessIteratorWnt::findNext(OsProcess &rProcess)
{
    OsStatus retval = OS_FAILED;

    //hProcessSnapshot was retrieved with a call to findFirst above
    if (hProcessSnapshot != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 lppe;
        lppe.dwSize = sizeof(PROCESSENTRY32);
        
        //now actuall go find the process
        if (Process32Next(hProcessSnapshot,&lppe))
        {
            if (GetLastError() != ERROR_NO_MORE_FILES)
            {
                rProcess.mPID           = lppe.th32ProcessID;
                rProcess.mParentPID     = lppe.th32ParentProcessID;
                rProcess.mProcessName   = lppe.szExeFile;

                retval = OS_SUCCESS;
            }
            else
                retval = OS_FAILED;
        }
    }
    
    return retval;
}

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */



