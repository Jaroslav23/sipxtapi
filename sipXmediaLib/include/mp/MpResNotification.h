//
// Copyright (C) 2007 Jaroslav Libak
//
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef _MpResNotification_h_
#define _MpResNotification_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsStatus.h>
#include <os/OsIntTypes.h>
#include <os/OsNotification.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

typedef enum
{
   MP_RES_DTMF_2833_NOTIFICATION = 0, // used to notify decoder from MpdPtAVT
   MP_RES_DTMF_INBAND_NOTIFICATION
}MpResNotificationType;

// FORWARD DECLARATIONS

#endif _MpResNotification_h_