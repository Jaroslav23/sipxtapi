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

#ifndef AcCommandMsg_h__
#define AcCommandMsg_h__

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <cp/CpMessageTypes.h>

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
class AcCommandMsg : public OsMsg
{
   /* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   typedef enum
   {
      AC_GAIN_FOCUS = 0,///< gain local focus (mic, speaker)
      AC_YIELD_FOCUS, ///< loose local focus
      AC_CONNECT, ///< connects existing free call shell
      AC_START_RTP_REDIRECT, ///< starts RTP redirect operation on 2 calls
      AC_STOP_RTP_REDIRECT, ///< stops RTP redirect operation on 2 calls
      AC_ACCEPT_CONNECTION, ///< accepts inbound call
      AC_REJECT_CONNECTION, ///< rejects inbound call
      AC_REDIRECT_CONNECTION, ///< redirects inbound call
      AC_ANSWER_CONNECTION, ///< answers inbound call
      AC_DESTROY_CONNECTION, ///< destroys connection. Connection is progressed into destroyed state and deleted.
      AC_DROP_CONNECTION, ///< drops connection
      AC_DROP_ALL_CONNECTIONS, ///< drops all connections
      AC_TRANSFER_BLIND, ///< initiates blind call transfer
      AC_TRANSFER_CONSULTATIVE, ///< initiates consultative call transfer
      AC_HOLD_CONNECTION, ///< holds connection
      AC_HOLD_ALL_CONNECTIONS, ///< holds all connections
      AC_UNHOLD_CONNECTION, ///< unholds connection
      AC_UNHOLD_ALL_CONNECTIONS, ///< unholds all connections
      AC_LIMIT_CODEC_PREFERENCES, ///< limits codec preferences for future calls
      AC_RENEGOTIATE_CODECS, ///< renegotiates connection codecs
      AC_RENEGOTIATE_CODECS_ALL, ///< renegotiates codecs on all connections
      AC_SEND_INFO, ///< send SIP INFO message
      AC_MUTE_INPUT_CONNECTION, ///< mutes inbound RTP connection on bridge
      AC_UNMUTE_INPUT_CONNECTION, ///< unmutes inbound RTP connection on bridge
      AC_AUDIO_BUFFER_PLAY, ///< starts playing a sound buffer on call
      AC_AUDIO_FILE_PLAY, ///< starts playing a file on call
      AC_AUDIO_STOP_PLAYBACK, ///< stops file/buffer playback
      AC_AUDIO_PAUSE_PLAYBACK, ///< pauses buffer/file playback
      AC_AUDIO_RESUME_PLAYBACK, ///< resumes buffer/file playback
      AC_AUDIO_RECORD_START, ///< starts call/conference recording
      AC_AUDIO_RECORD_STOP, ///< stops call/conference recording
      AC_AUDIO_TONE_START, ///< sends DTMF tone (in-band or rfc2833)
      AC_AUDIO_TONE_STOP, ///< stops sending DTMF tone
      AC_SUBSCRIBE, ///< subscribe to notifications
      AC_UNSUBSCRIBE, ///< unsubscribe from notifications
      AC_ACCEPT_TRANSFER, ///< accept transfer request
      AC_REJECT_TRANSFER, ///< reject transfer request
      AC_CONFERENCE_SPLIT, ///< split connection from conference into new call
      AC_CONFERENCE_JOIN, ///< join connection from a call into conference
   } SubTypeEnum;

   /* ============================ CREATORS ================================== */

   AcCommandMsg(SubTypeEnum subType);

   virtual ~AcCommandMsg();

   virtual OsMsg* createCopy(void) const;

   /* ============================ MANIPULATORS ============================== */

   /* ============================ ACCESSORS ================================= */

   /* ============================ INQUIRY =================================== */

   /* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /** Private copy constructor */
   AcCommandMsg(const AcCommandMsg& rMsg);

   /** Private assignment operator */
   AcCommandMsg& operator=(const AcCommandMsg& rhs);
};

#endif // AcCommandMsg_h__
