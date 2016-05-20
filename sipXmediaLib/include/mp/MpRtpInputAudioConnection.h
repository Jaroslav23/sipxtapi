//  
// Copyright (C) 2006-2007 SIPez LLC. 
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


#ifndef _MpRtpInputAudioConnection_h_
#define _MpRtpInputAudioConnection_h_

// FORWARD DECLARATIONS
class MpFlowGraphBase;
class MpDecoderBase;
class MpResource;
class MprDecode;
class MprRecorder;
class SdpCodec;
class OsNotification;
class MprDtmfDetectorBase;

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/MpRtpInputConnection.h"
#include "mp/MpNotificationMsgDef.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
class SdpCodecList;

/**
*  @brief Connection container for audio part of call.
*/
class MpRtpInputAudioConnection : public MpRtpInputConnection
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
	friend class MprSimpleDtmfDetector;
   friend class MprDecode;


/* ============================ CREATORS ================================== */
///@name Creators
//@{

     /// Constructor
   MpRtpInputAudioConnection(const UtlString& resourceName,
                             MpConnectionID myID,
                             OsMsgQ* pConnectionNotificationQueue,
                             UtlBoolean bInBandDTMFEnabled,
                             UtlBoolean bRFC2833DTMFEnabled,
                             int samplesPerFrame, 
                             int samplesPerSec);

     /// Destructor
   virtual
   ~MpRtpInputAudioConnection();

//@}

/* ============================ MANIPULATORS ============================== */
///@name Manipulators
//@{
   /// Process one frame of audio
   UtlBoolean processFrame(void);

     /// Add an RTP payload type to decoder instance mapping table
   void addPayloadType(int payloadId, MpDecoderBase* pDecoder);

     /// Remove an RTP payload type from decoder instance map
   void deletePayloadType(int payloadId);

   /**
    * Observer notification callback.
    */
   virtual void onNotify(UtlObservable* subject, int code, intptr_t userData);
//@}

/* ============================ ACCESSORS ================================= */
///@name Accessors
//@{

   static void setConnectionIdleTimeout(long timeoutSeconds);

     /// Get decoder for this payload type
   MpDecoderBase* mapPayloadType(int payloadType);


   /// Queue a message to start receiving RTP and RTCP packets.
   static OsStatus startReceiveRtp(OsMsgQ& messageQueue,
                                   const UtlString& resourceName,
                                   const SdpCodecList& sdpCodecList,
                                   OsSocket& rRtpSocket, 
                                   OsSocket& rRtcpSocket);


   /// queue a message to stop receiving RTP and RTCP packets.
   static OsStatus stopReceiveRtp(OsMsgQ& messageQueue,
                                  const UtlString& resourceName);

//@}

/* ============================ INQUIRY =================================== */
///@name Inquiry
//@{

//@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   /// @brief Handles an incoming resource message for this media processing object.
   virtual UtlBoolean handleMessage(MpResourceMsg& rMsg);
   /**< @returns TRUE if the message was handled, otherwise FALSE. */

   /// @brief perform the enable operation specific to the MpRtpInputAudioConnection
   virtual UtlBoolean handleEnable();

   /// @brief perform the disable operation specific to the MpRtpInputAudioConnection
   virtual UtlBoolean handleDisable();

   /// @brief This method does the real work for the media processing resource and 
   /// must be defined in each class derived from this one.
   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                     MpBufPtr outBufs[],
                                     int inBufsSize,
                                     int outBufsSize,
                                     UtlBoolean isEnabled,
                                     int samplesPerFrame=80,
                                     int samplesPerSecond=8000);

   void sendConnectionNotification(MpNotificationMsgType type, intptr_t data);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

     /// Stops receiving RTP and RTCP packets.
   void handleStopReceiveRtp(void);

   /// Starts receiving RTP and RTCP packets.
   void handleStartReceiveRtp(UtlSList& codecList,// list of SdpCodec instances
                        OsSocket& rRtpSocket, OsSocket& rRtcpSocket);

     /// Default constructor
   MpRtpInputAudioConnection();

     /// Copy constructor (not implemented for this type)
   MpRtpInputAudioConnection(const MpRtpInputAudioConnection& rMpRtpInputAudioConnection);

     /// Assignment operator (not implemented for this type)
   MpRtpInputAudioConnection& operator=(const MpRtpInputAudioConnection& rhs);

   MprDecode*         mpDecode;        ///< Inbound component: Decoder
   MprDtmfDetectorBase* mpDtmfDetector; // InBand DTMF decoder

   MpDecoderBase*     mpPayloadMap[NUM_PAYLOAD_TYPES];
                                       ///< Map RTP payload types to our decoders

   UtlBoolean m_bInBandDTMFEnabled;
   UtlBoolean m_bRFC2833DTMFEnabled;
   int m_samplesPerFrame; 
   int m_samplesPerSec;
   long m_inactiveFrameCount; ///< count of frames we have seen without real sound
   static long ms_maxInactiveFrameCount; ///< maximum number of frames before we report inactivity
   UtlBoolean m_bAudioReceived; ///< true when at least 1 audio frame has been received
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpRtpInputAudioConnection_h_
