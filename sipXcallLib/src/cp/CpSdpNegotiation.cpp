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

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <os/OsDateTime.h>
#include <os/OsSysLog.h>
#include <sdp/SdpCodec.h>
#include <sdp/SdpCodecList.h>
#include <net/SdpBody.h>
#include <net/SipMessage.h>
#include <cp/CpSdpNegotiation.h>

// DEFINES
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// MACROS
// GLOBAL VARIABLES
// GLOBAL FUNCTIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

CpSdpNegotiation::CpSdpNegotiation()
: m_negotiationState(CpSdpNegotiation::SDP_NOT_NEGOTIATED)
, m_bSdpOfferFinished(FALSE)
, m_bSdpAnswerFinished(FALSE)
, m_bUnreliableSdpAnswerFinished(FALSE)
, m_bLocallyInitiated(FALSE)
, m_sdpOfferingMode(CpSdpNegotiation::SDP_OFFERING_IMMEDIATE)
, m_pOfferSipMessage(NULL)
, m_pAnswerSipMessage(NULL)
, m_pSecurity(NULL)
, m_bLocalHoldRequest(FALSE)
, m_cseqNum(-1)
{

}

CpSdpNegotiation::~CpSdpNegotiation()
{
   delete m_pSecurity;
   m_pSecurity = NULL;

   delete m_pOfferSipMessage;
   m_pOfferSipMessage = NULL;

   delete m_pAnswerSipMessage;
   m_pAnswerSipMessage = NULL;

   m_cseqNum = -1;
}

/* ============================ MANIPULATORS ============================== */

void CpSdpNegotiation::startSdpNegotiation(const SipMessage& sipMessage, UtlBoolean bLocalHoldRequest)
{
   m_negotiationState = CpSdpNegotiation::SDP_NEGOTIATION_IN_PROGRESS;
   m_bSdpOfferFinished = FALSE;
   m_bSdpAnswerFinished = FALSE;
   m_bUnreliableSdpAnswerFinished = FALSE;
   m_bLocallyInitiated = sipMessage.isFromThisSide();

   delete m_pOfferSipMessage;
   m_pOfferSipMessage = NULL;

   delete m_pAnswerSipMessage;
   m_pAnswerSipMessage = NULL;

   m_localSdpCodecList.clearCodecs();
   m_bLocalHoldRequest = bLocalHoldRequest;

   sipMessage.getCSeqField(&m_cseqNum, NULL); // save transaction number
}

UtlBoolean CpSdpNegotiation::handleInboundSdpOffer(const SipMessage& rOfferSipMessage)
{
   if ((m_bLocallyInitiated && rOfferSipMessage.isFromThisSide()) ||
      (!m_bLocallyInitiated && !rOfferSipMessage.isFromThisSide()))
   {
      m_bSdpOfferFinished = TRUE;

      delete m_pOfferSipMessage;
      m_pOfferSipMessage = new SipMessage(rOfferSipMessage); // keep copy of sdp offer

      return TRUE;
   }

   return FALSE;
}

UtlBoolean CpSdpNegotiation::handleInboundSdpAnswer(const SipMessage& rAnswerSipMessage)
{
   UtlBoolean bFinishSdpAnswer = FALSE;
   // SDP answer must come from other side than offer
   if ((m_bLocallyInitiated && !rAnswerSipMessage.isFromThisSide()) ||
      (!m_bLocallyInitiated && rAnswerSipMessage.isFromThisSide()))
   {
      if (m_bSdpOfferFinished)
      {
         if (rAnswerSipMessage.isResponse())
         {
            int seqNum;
            UtlString seqMethod;
            rAnswerSipMessage.getCSeqField(&seqNum, &seqMethod);
            UtlBoolean bIsReliable = rAnswerSipMessage.isRequireExtensionSet(SIP_PRACK_EXTENSION);
            int responseCode = rAnswerSipMessage.getResponseStatusCode();
            if (seqMethod.compareTo(SIP_INVITE_METHOD) == 0 &&
               responseCode > SIP_1XX_CLASS_CODE &&
               responseCode < SIP_2XX_CLASS_CODE &&
               !bIsReliable)
            {
               // unreliable provisional response
               m_bUnreliableSdpAnswerFinished = TRUE;
               delete m_pAnswerSipMessage;
               m_pAnswerSipMessage = new SipMessage(rAnswerSipMessage); // keep copy of sdp answer
            }
            else
            {
               bFinishSdpAnswer = TRUE;
            }
         }
         else
         {
            bFinishSdpAnswer = TRUE;
         }

         if (bFinishSdpAnswer)
         {
            delete m_pAnswerSipMessage;
            m_pAnswerSipMessage = new SipMessage(rAnswerSipMessage); // keep copy of sdp answer

            m_bSdpAnswerFinished = TRUE;
            m_negotiationState = CpSdpNegotiation::SDP_NEGOTIATION_COMPLETE;
         }

         return TRUE;
      }
   }

   return FALSE;
}

void CpSdpNegotiation::resetSdpNegotiation()
{
   m_negotiationState = CpSdpNegotiation::SDP_NOT_NEGOTIATED;

   m_bSdpOfferFinished = FALSE;
   m_bSdpAnswerFinished = FALSE;

   delete m_pOfferSipMessage;
   m_pOfferSipMessage = NULL;
   delete m_pAnswerSipMessage;
   m_pAnswerSipMessage = NULL;

   m_localSdpCodecList.clearCodecs();
   m_bLocalHoldRequest = FALSE;

   m_cseqNum = -1;
}

void CpSdpNegotiation::notifyAuthRetry(int newCseqNumber)
{
   m_cseqNum = newCseqNumber;

   // also patch offer and answer messages
   if (m_pOfferSipMessage)
   {
      int oldCseqNumber;
      UtlString cseqMethod;
      m_pOfferSipMessage->getCSeqField(oldCseqNumber, cseqMethod);
      m_pOfferSipMessage->setCSeqField(newCseqNumber, cseqMethod);
   }
   if (m_pAnswerSipMessage)
   {
      int oldCseqNumber;
      UtlString cseqMethod;
      m_pAnswerSipMessage->getCSeqField(oldCseqNumber, cseqMethod);
      m_pAnswerSipMessage->setCSeqField(newCseqNumber, cseqMethod);
   }
}

void CpSdpNegotiation::getCommonSdpCodecs(const SdpBody& rSdpBody, ///< inbound SDP body (offer or answer)
                                          const SdpCodecList& supportedCodecs,
                                          int& numCodecsInCommon, ///< how many codecs do we have in common
                                          SdpCodecList& commonCodecsForEncoder,
                                          SdpCodecList& commonCodecsForDecoder,
                                          UtlString& remoteRtpAddress,
                                          int& remoteRtpPort,
                                          int& remoteRtcpPort,
                                          int& remoteVideoRtpPort,
                                          int& remoteVideoRtcpPort,
                                          const SdpSrtpParameters& localSrtpParams,
                                          SdpSrtpParameters& matchingSrtpParams,
                                          int localVideoFramerate,
                                          int& matchingVideoFramerate)
{
   memset((void*)&matchingSrtpParams, 0, sizeof(SdpSrtpParameters));

   SdpCodec** encoderCodecs = NULL;
   SdpCodec** decoderCodecs = NULL;
   int matchingBandwidth; // ignored
   int localBandwidth = 0; // ignored

   rSdpBody.getBestAudioCodecs(supportedCodecs,
      numCodecsInCommon,
      encoderCodecs,
      decoderCodecs,
      remoteRtpAddress,
      remoteRtpPort,
      remoteRtcpPort,
      remoteVideoRtpPort,
      remoteVideoRtcpPort,
      localSrtpParams,
      matchingSrtpParams,
      localBandwidth,
      matchingBandwidth,
      localVideoFramerate,
      matchingVideoFramerate);

   // To be compliant with RFC 3264
   if(rSdpBody.findValueInField("a", "sendonly") || rSdpBody.findValueInField("a", "inactive"))
   {
      remoteRtpAddress = "0.0.0.0";
   }

   // convert array to SdpCodecList
   commonCodecsForEncoder.clearCodecs();
   commonCodecsForEncoder.addCodecs(numCodecsInCommon, encoderCodecs);
   commonCodecsForDecoder.clearCodecs();
   commonCodecsForDecoder.addCodecs(numCodecsInCommon, decoderCodecs);

   deleteSdpCodecArray(numCodecsInCommon, encoderCodecs);
   deleteSdpCodecArray(numCodecsInCommon, decoderCodecs);
}

void CpSdpNegotiation::addSdpOffer(SipMessage& rSipMessage,
                                   int nRtpContacts,
                                   UtlString hostAddresses[],
                                   int rtpAudioPorts[],
                                   int rtcpAudiopPorts[],
                                   int rtpVideoPorts[],
                                   int rtcpVideoPorts[],
                                   RTP_TRANSPORT transportTypes[],
                                   const SdpCodecList& sdpCodecList,
                                   const SdpSrtpParameters& srtpParams,
                                   int videoBandwidth,
                                   int videoFramerate,
                                   RTP_TRANSPORT rtpTransportOptions)
{
   addSdpBody(rSipMessage, NULL, nRtpContacts, hostAddresses, rtpAudioPorts, rtcpAudiopPorts, rtpVideoPorts, rtcpVideoPorts,
      transportTypes, sdpCodecList, srtpParams, videoBandwidth, videoFramerate, rtpTransportOptions);

   delete m_pOfferSipMessage;
   m_pOfferSipMessage = new SipMessage(rSipMessage);

   m_bSdpOfferFinished = TRUE;
}

void CpSdpNegotiation::addSdpAnswer(SipMessage& rSipMessage,
                                    int nRtpContacts,
                                    UtlString hostAddresses[],
                                    int rtpAudioPorts[],
                                    int rtcpAudiopPorts[],
                                    int rtpVideoPorts[],
                                    int rtcpVideoPorts[],
                                    RTP_TRANSPORT transportTypes[],
                                    const SdpCodecList& sdpCodecList,
                                    const SdpSrtpParameters& srtpParams,
                                    int videoBandwidth,
                                    int videoFramerate,
                                    RTP_TRANSPORT rtpTransportOptions)
{
   UtlBoolean bFinishSdpAnswer = FALSE;

   if (m_pOfferSipMessage)
   {
      addSdpBody(rSipMessage, m_pOfferSipMessage, nRtpContacts, hostAddresses, rtpAudioPorts, rtcpAudiopPorts,
         rtpVideoPorts, rtcpVideoPorts, transportTypes, sdpCodecList, srtpParams, videoBandwidth,
         videoFramerate, rtpTransportOptions);

      if (rSipMessage.isResponse())
      {
         int seqNum;
         UtlString seqMethod;
         rSipMessage.getCSeqField(&seqNum, &seqMethod);
         UtlBoolean bIsReliable = rSipMessage.isRequireExtensionSet(SIP_PRACK_EXTENSION);
         int responseCode = rSipMessage.getResponseStatusCode();
         if (seqMethod.compareTo(SIP_INVITE_METHOD) == 0 &&
            responseCode > SIP_1XX_CLASS_CODE &&
            responseCode < SIP_2XX_CLASS_CODE &&
            !bIsReliable)
         {
            // unreliable provisional response
            m_bUnreliableSdpAnswerFinished = TRUE;
            delete m_pAnswerSipMessage;
            m_pAnswerSipMessage = new SipMessage(rSipMessage); // keep copy of sdp answer
         }
         else
         {
            bFinishSdpAnswer = TRUE;
         }
      }
      else
      {
         bFinishSdpAnswer = TRUE;
      }

      if (bFinishSdpAnswer)
      {
         delete m_pAnswerSipMessage;
         m_pAnswerSipMessage = new SipMessage(rSipMessage); // keep copy of sdp answer

         m_bSdpAnswerFinished = TRUE;
         m_negotiationState = CpSdpNegotiation::SDP_NEGOTIATION_COMPLETE;
      }
   }
   else
   {
      OsSysLog::add(FAC_CP, PRI_ERR, "Cannot add SDP answer, because SDP offer was never seen.\n");
   }
}

/* ============================ ACCESSORS ================================= */

void CpSdpNegotiation::setSecurity(const SIPXTACK_SECURITY_ATTRIBUTES* val)
{
   delete m_pSecurity;
   m_pSecurity = NULL;

   if (val)
   {
      m_pSecurity = new SIPXTACK_SECURITY_ATTRIBUTES(*val);
   }
}

UtlBoolean CpSdpNegotiation::getRemoteSdpBody(SdpBody& sdpBody)
{
   if (m_pOfferSipMessage && !m_pOfferSipMessage->isFromThisSide())
   {
      // offer is from remote side, use its SDP body
      const SdpBody* pSdpBody = m_pOfferSipMessage->getSdpBody(m_pSecurity);
      if (pSdpBody)
      {
         sdpBody = *pSdpBody;
         return TRUE;
      }
   }
   else if (m_pAnswerSipMessage && !m_pAnswerSipMessage->isFromThisSide())
   {
      // answer is from remote side, use its SDP body
      const SdpBody* pSdpBody = m_pAnswerSipMessage->getSdpBody(m_pSecurity);
      if (pSdpBody)
      {
         sdpBody = *pSdpBody;
         return TRUE;
      }
   }

   return FALSE;
}

/* ============================ INQUIRY =================================== */

UtlBoolean CpSdpNegotiation::isNewSdpNegotiationAllowed() const
{
   return m_negotiationState != CpSdpNegotiation::SDP_NEGOTIATION_IN_PROGRESS;
}

UtlBoolean CpSdpNegotiation::isSdpNegotiationInProgress() const
{
   return m_negotiationState == CpSdpNegotiation::SDP_NEGOTIATION_IN_PROGRESS;
}

UtlBoolean CpSdpNegotiation::isSdpNegotiationComplete() const
{
   return m_negotiationState == CpSdpNegotiation::SDP_NEGOTIATION_COMPLETE;
}

UtlBoolean CpSdpNegotiation::isSdpNegotiationUnreliablyComplete() const
{
   return m_negotiationState == CpSdpNegotiation::SDP_NEGOTIATION_IN_PROGRESS &&
      m_bUnreliableSdpAnswerFinished;
}

CpSdpNegotiation::SdpBodyType CpSdpNegotiation::getSdpBodyType(const SipMessage& sipMessage) const
{
   if (m_pOfferSipMessage)
   {
      UtlBoolean bMatch = compareSipMessages(*m_pOfferSipMessage, sipMessage);
      if (bMatch)
      {
         return CpSdpNegotiation::SDP_BODY_OFFER; // this is an offer
      }
   }
   if (m_pAnswerSipMessage)
   {
      UtlBoolean bMatch = compareSipMessages(*m_pAnswerSipMessage, sipMessage);
      if (bMatch)
      {
         return CpSdpNegotiation::SDP_BODY_ANSWER; // this is an answer
      }
   }

   // get pointer to internal sdp body
   const SdpBody* pSdpBody = sipMessage.getSdpBody(m_pSecurity);
   if (pSdpBody)
   {
      // message has SDP body, but we are unable to tell if its offer or answer
      return CpSdpNegotiation::SDP_BODY_UNKNOWN;
   }

   // there is no sdp body
   return CpSdpNegotiation::SDP_BODY_NONE;
}

UtlBoolean CpSdpNegotiation::isInSdpNegotiation(const SipMessage& sipMessage) const
{
   int tmpSeqNum = -1;
   sipMessage.getCSeqField(&tmpSeqNum, NULL);

   if (m_cseqNum == tmpSeqNum &&
       (m_negotiationState == CpSdpNegotiation::SDP_NEGOTIATION_COMPLETE ||
        m_negotiationState == CpSdpNegotiation::SDP_NEGOTIATION_IN_PROGRESS))
   {
      return TRUE;
   }

   return FALSE;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

void CpSdpNegotiation::deleteSdpCodecArray(int arraySize, SdpCodec**& sdpCodecArray)
{
   // Free up the codec copies and array
   for(int codecIndex = 0; codecIndex < arraySize; codecIndex++)
   {
      delete sdpCodecArray[codecIndex];
      sdpCodecArray[codecIndex] = NULL;
   }
   if(sdpCodecArray) delete[] sdpCodecArray;
   sdpCodecArray = NULL;
}

UtlBoolean CpSdpNegotiation::compareSipMessages(const SipMessage& baseSipMessage, const SipMessage& receivedSipMessage) const
{
   int receivedSeqNum;
   UtlString receivedSeqMethod;
   UtlBoolean bReceivedIsRequest;
   receivedSipMessage.getCSeqField(&receivedSeqNum, &receivedSeqMethod);
   bReceivedIsRequest = receivedSipMessage.isRequest();

   int baseSeqNum;
   UtlString baseSeqMethod;
   UtlBoolean bBaseIsRequest;
   baseSipMessage.getCSeqField(&baseSeqNum, &baseSeqMethod);
   bBaseIsRequest = baseSipMessage.isRequest();

   const SdpBody* pBaseSdpBody = baseSipMessage.getSdpBody(m_pSecurity);
   const SdpBody* pReceivedSdpBody = receivedSipMessage.getSdpBody(m_pSecurity);

   if (pBaseSdpBody && pReceivedSdpBody)
   {
      // both messages have SDP body
      if (baseSeqNum == receivedSeqNum && baseSeqMethod.compareTo(receivedSeqMethod) == 0)
      {
         if (bReceivedIsRequest && bBaseIsRequest)
         {
            // both are requests
            return TRUE;
         }
         else if (!bReceivedIsRequest && !bBaseIsRequest)
         {
            // both are responses, also response codes must match
            int receivedResponseCode = receivedSipMessage.getResponseStatusCode();
            int baseResponseCode = baseSipMessage.getResponseStatusCode();
            if (receivedResponseCode == baseResponseCode)
            {
               return TRUE;
            }
         }
      }
   }

   return FALSE;
}

void CpSdpNegotiation::addSdpBody(SipMessage& rSipMessage,///< sip message where we want to add SDP body
                                  SipMessage* pSdpOfferSipMessage,
                                  int nRtpContacts,
                                  UtlString hostAddresses[],
                                  int rtpAudioPorts[],
                                  int rtcpAudiopPorts[],
                                  int rtpVideoPorts[],
                                  int rtcpVideoPorts[],
                                  RTP_TRANSPORT transportTypes[],
                                  const SdpCodecList& sdpCodecList,
                                  const SdpSrtpParameters& srtpParams,
                                  int videoBandwidth,
                                  int videoFramerate,
                                  RTP_TRANSPORT rtpTransportOptions)
{
   int codecCount = sdpCodecList.getCodecCount();
   SdpCodec** pCodecArray = new SdpCodec*[codecCount];
   sdpCodecList.getCodecs(codecCount, pCodecArray); // fill array with codec copies

   // if m_pOfferSipMessage is known, then SDP answer is added, otherwise SDP offer
   rSipMessage.addSdpBody(nRtpContacts,
      hostAddresses,
      rtpAudioPorts,
      rtcpAudiopPorts,
      rtpVideoPorts,
      rtcpVideoPorts,
      transportTypes,
      codecCount,
      pCodecArray,
      srtpParams,
      videoBandwidth,
      videoFramerate,
      pSdpOfferSipMessage, // original SIP message with offer if available
      rtpTransportOptions,
      m_bLocalHoldRequest);

   // remember local codec list, needed for starting local audio decoding
   m_localSdpCodecList = sdpCodecList;

   deleteSdpCodecArray(codecCount, pCodecArray);
}

/* ============================ FUNCTIONS ================================= */
