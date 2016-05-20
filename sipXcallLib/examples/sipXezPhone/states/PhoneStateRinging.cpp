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

// APPLICATION INCLUDES
#include "../stdwx.h"
#include "../sipXmgr.h"
#include "../sipXezPhoneApp.h"
#include "PhoneStateIdle.h"
#include "PhoneStateDialing.h"
#include "PhoneStateRinging.h"
#include "PhoneStateConnected.h"
#include "PhoneStateDisconnectRequested.h"
#include "PhoneStateAccepted.h"
#include "../sipXezPhoneSettings.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern sipXezPhoneApp* thePhoneApp;
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACRO CALLS

PhoneStateRinging::PhoneStateRinging(SIPX_CALL hCall) : 
    mbPlayingTone(false)
{
   mhCall = hCall;
}

PhoneStateRinging::~PhoneStateRinging(void)
{
#ifdef VOICE_ENGINE
    if (mbPlayingTone)
        sipxCallAudioPlayFileStop(sipXmgr::getInstance().getCurrentCall());
#endif
}

PhoneState* PhoneStateRinging::OnDisconnected(const SIPX_CALL hCall)
{
   return (new PhoneStateDisconnectRequested());
}

PhoneState* PhoneStateRinging::OnFlashButton()
{
    return (new PhoneStateAccepted(mhCall));
}

PhoneState* PhoneStateRinging::OnConnected()
{
   return (new PhoneStateConnected());
}

PhoneState* PhoneStateRinging::Execute()
{
    sipXmgr::getInstance().setCurrentCall(mhCall); 
    
    char szIncomingNumber[256];
    sipxCallGetRemoteField(mhCall, szIncomingNumber, 256);
    wxString incomingNumber(szIncomingNumber);

    thePhoneApp->setStatusMessage(incomingNumber);

    if (sipXezPhoneSettings::getInstance().getAutoAnswer() == true)
    {
        sipxCallAudioPlayFileStop(sipXmgr::getInstance().getCurrentCall()) ;
        // if in Auto Answer mode, go ahead and answer
        sipxCallAnswer(sipXmgr::getInstance().getCurrentCall());
    }
    return this;
}
