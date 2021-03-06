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
#include <stdio.h>
#include <assert.h>
#if defined(_WIN32)
# include <io.h>
# define STDIN_FILENO 0 /* can't find where windows defines this */
#else
# include <unistd.h>
#endif

// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <sipXtapiDriver/CommandProcessor.h>
#include <net/SipUserAgent.h>
#include <sipXtapiDriver/ExitCommand.h>
#include <sipXtapiDriver/SleepCommand.h>

int SleepCommand::execute(int argc, char* argv[])
{
	int commandStatus = CommandProcessor::COMMAND_FAILED;
    if(argc != 2)
	{
		UtlString usage;
	    getUsage(argv[0], &usage);
        printf("%s", usage.data());
		commandStatus = CommandProcessor::COMMAND_BAD_SYNTAX;
	}
    else
	{
        int msecs = (atoi(argv[1])) * 1000;
        if(msecs > 0)
        {
			commandStatus = CommandProcessor::COMMAND_SUCCESS;
            OsTask::delay(msecs);
        }
    }
	return(commandStatus);
}

void SleepCommand::getUsage(const char* commandName, UtlString* usage) const
{
	Command::getUsage(commandName, usage);
	usage->append(" <seconds>\n");
}
