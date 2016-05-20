//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
//
// Copyright (C) 2005-2007 SIPez LLC.
// Licensed to SIPfoundry under a Contributor Agreement.
// 
// Copyright (C) 2004-2007 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef SipXConfig_h__
#define SipXConfig_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
#define MIN_VIDEO_BITRATE 5
#define MAX_VIDEO_BITRATE 400

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
class SIPX_INSTANCE_DATA;

// STRUCTS
// TYPEDEFS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

OsStatus initLogger();
void freeAudioCodecs(SIPX_INSTANCE_DATA& pInst);
void freeVideoCodecs(SIPX_INSTANCE_DATA& pInst);

/**
* Utility function for setting allowed methods on a 
* instance's user-agent.
*/
SIPXTAPI_API SIPX_RESULT sipxConfigAllowMethod(const SIPX_INST hInst, const char* method, const bool bAllow = true);


#endif // SipXConfig_h__
