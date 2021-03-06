//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <cppunit/TestCase.h>
#include <cppunit/SourceLine.h>
#include <cppunit/Asserter.h>
#include <string>

//Application Includes
#include <os/OsDefs.h>
#include <os/OsFileSystem.h>
#include <sipxunit/TestUtilities.h>
#include <os/OsTestUtilities.h>

void OsTestUtilities::createTestDir(OsPath& root)
{
    OsStatus stat;

    OsFileSystem::getWorkingDirectory(root);
    root.append(OsPath::separator).append("OsFileSystemTest");

    if (OsFileSystem::exists(root))
    {
        removeTestDir(root);
    }

    stat = OsFileSystem::createDir(root);
    CPPUNIT_ASSERT_MESSAGE("setup root test dir", stat == OS_SUCCESS);
}

void OsTestUtilities::removeTestDir(OsPath &root)
{
    OsStatus stat;
    if (OsFileSystem::exists(root))
    {
        stat = OsFileSystem::remove(root, TRUE, TRUE);
 
         KNOWN_BUG("Fails randomly on build server and fails everytime, the first time its run on a new machine", 
           "XPL-191");
        
        CPPUNIT_ASSERT_MESSAGE("teardown root test dir", stat == OS_SUCCESS);
    }
}

void OsTestUtilities::initDummyBuffer(char *buff, int size)
{
    for (int i = 0; i < size; i++)
    {
        buff[i] = (char)(i % 256);
    }
}

UtlBoolean OsTestUtilities::testDummyBuffer(char *buff, unsigned long size, unsigned long position)
{
    for (unsigned long i = 0; i < size; i++)
    {
        char expected = (char)((position + i) % 256);
        if (buff[i] != expected)
        {
            printf("buff[%li] = %i, expected = %i\n", position, buff[i], expected);
            return false;
        }
    }

    return true;
}


OsStatus OsTestUtilities::createDummyFile(OsPath testFile, unsigned long size)
{
    OsStatus stat;
    char wbuff[10000];
    unsigned long wbuffsize = (unsigned long)sizeof(wbuff);

    OsTestUtilities::initDummyBuffer(wbuff, sizeof(wbuff));

    OsFile wfile(testFile);
    stat = wfile.open(OsFile::CREATE);
    if (stat == OS_SUCCESS)
    {
        unsigned long wposition = 0;
        for (int i = 0; stat == OS_SUCCESS && wposition < wbuffsize; i++)
        {
            unsigned long remaining = wbuffsize - wposition;
            unsigned long byteswritten = 0;
            stat = wfile.write(wbuff + wposition, remaining, byteswritten);
            wposition += byteswritten;
        }

        wfile.close();
    }

    return stat;
}

UtlBoolean OsTestUtilities::verifyDummyFile(OsPath testFile, unsigned long size)
{
    OsStatus stat;
    UtlBoolean ok = false;
    char rbuff[256];
    unsigned long rbuffsize = (unsigned long)sizeof(rbuff);
    OsFile rfile(testFile);
    stat = rfile.open();
    if (stat == OS_SUCCESS)
    {
        unsigned long rposition = 0;
        ok = true;
        for (int i = 0; ok && rposition < size; i++)
        {
            unsigned long remaining = (size - rposition);
            unsigned long readsize = remaining < rbuffsize ? remaining : rbuffsize;
            unsigned long bytesread = 0;
            stat = rfile.read(rbuff, readsize, bytesread);
            if (stat != OS_SUCCESS)
            {
                ok = false;
                printf("Error reading file, status = %i", stat);
            }
            else
            {
                ok = OsTestUtilities::testDummyBuffer(rbuff, bytesread, rposition);
                rposition += bytesread;
            }
        }

        rfile.close();
    }

    return ok;
}

