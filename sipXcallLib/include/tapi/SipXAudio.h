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

#ifndef SipXAudio_h__
#define SipXAudio_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// FORWARD DECLARATIONS
class SIPX_INSTANCE_DATA;
class MIC_SETTING;
class SPEAKER_SETTING;

// STRUCTS
// TYPEDEFS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

void initAudioDevices(SIPX_INSTANCE_DATA& pInst);
void freeAudioDevices(SIPX_INSTANCE_DATA& pInst);
void initMicSettings(MIC_SETTING& pMicSetting);
void initSpeakerSettings(SPEAKER_SETTING& pSpeakerSetting);

#endif // SipXAudio_h__
