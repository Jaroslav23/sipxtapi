//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// Copyright (C) 2007 Jaroslav Libak
// Licensed under the LGPL license.
// $$
///////////////////////////////////////////////////////////////////////////////

#ifndef AcAudioPausePlaybackMsg_h__
#define AcAudioPausePlaybackMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <utl/UtlString.h>
#include <cp/CpMessageTypes.h>
#include <cp/msg/AcCommandMsg.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS
// MACROS
// FORWARD DECLARATIONS

/**
* Abstract call command message. Instructs call to carry out some action.
*/
class AcAudioPausePlaybackMsg : public AcCommandMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   /* ============================ CREATORS ================================== */

   AcAudioPausePlaybackMsg();

   virtual ~AcAudioPausePlaybackMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private copy constructor */
   AcAudioPausePlaybackMsg(const AcAudioPausePlaybackMsg& rMsg);

   /** Private assignment operator */
   AcAudioPausePlaybackMsg& operator=(const AcAudioPausePlaybackMsg& rhs);
};

#endif // AcAudioPausePlaybackMsg_h__
