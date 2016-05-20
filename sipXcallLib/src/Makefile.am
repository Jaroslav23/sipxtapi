## Process this file with automake to produce Makefile.in


EXTRA_DIST = tapi

SUBDIRS = .

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXmediaAdapterLib/interface/include

lib_LTLIBRARIES = libsipXcall.la

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

libsipXcall_la_LDFLAGS = -version-info ${version_Current}:${version_Revision}:${version_Age}

libsipXcall_la_LIBADD = \
	@SIPXMEDIA_MP_LIBS@ -lstdc++

libsipXcall_la_CXXFLAGS = \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\" \
    @VIDEO_DEFINE@

libsipXcall_la_SOURCES = \
    ../config/sipxcalllib-buildstamp.h \
    ../config/sipxcalllib-buildstamp.cpp \
    cp/CpLoopDetector.cpp \
    cp/CpNotificationRegister.cpp \
    cp/Cp100RelTracker.cpp \
    cp/CpSdpNegotiation.cpp \
    cp/CpSessionTimerProperties.cpp \
    cp/CpSipTransactionManager.cpp \
    cp/XCpAbstractCall.cpp \
    cp/XCpCall.cpp \
    cp/XCpCallIdUtil.cpp \
    cp/XCpCallManager.cpp \
    cp/XCpConference.cpp \
    cp/XCpCallStack.cpp \
    cp/XSipConnection.cpp \
    cp/XSipConnectionContext.cpp \
    cp/msg/AcAcceptConnectionMsg.cpp \
    cp/msg/AcAcceptTransferMsg.cpp \
    cp/msg/AcAnswerConnectionMsg.cpp \
    cp/msg/AcAudioBufferPlayMsg.cpp \
    cp/msg/AcAudioFilePlayMsg.cpp \
    cp/msg/AcAudioPausePlaybackMsg.cpp \
    cp/msg/AcAudioRecordStartMsg.cpp \
    cp/msg/AcAudioRecordStopMsg.cpp \
    cp/msg/AcAudioResumePlaybackMsg.cpp \
    cp/msg/AcAudioStopPlaybackMsg.cpp \
    cp/msg/AcAudioToneStartMsg.cpp \
    cp/msg/AcAudioToneStopMsg.cpp \
    cp/msg/AcCommandMsg.cpp \
    cp/msg/AcConferenceJoinMsg.cpp \
    cp/msg/AcConferenceSplitMsg.cpp \
    cp/msg/AcConnectMsg.cpp \
    cp/msg/AcDestroyConnectionMsg.cpp \
    cp/msg/AcDropAllConnectionsMsg.cpp \
    cp/msg/AcDropConnectionMsg.cpp \
    cp/msg/AcGainFocusMsg.cpp \
    cp/msg/AcHoldAllConnectionsMsg.cpp \
    cp/msg/AcHoldConnectionMsg.cpp \
    cp/msg/AcLimitCodecPreferencesMsg.cpp \
    cp/msg/AcMuteInputConnectionMsg.cpp \
    cp/msg/AcNotificationMsg.cpp \
    cp/msg/AcRedirectConnectionMsg.cpp \
    cp/msg/AcRejectConnectionMsg.cpp \
    cp/msg/AcRejectTransferMsg.cpp \
    cp/msg/AcRenegotiateCodecsAllMsg.cpp \
    cp/msg/AcRenegotiateCodecsMsg.cpp \
    cp/msg/AcSendInfoMsg.cpp \
    cp/msg/AcStartedMsg.cpp \
    cp/msg/AcStartRtpRedirectMsg.cpp \
    cp/msg/AcStopRtpRedirectMsg.cpp \
    cp/msg/AcSubscribeMsg.cpp \
    cp/msg/AcTransferBlindMsg.cpp \
    cp/msg/AcTransferConsultativeMsg.cpp \
    cp/msg/AcTunneledNotificationMsg.cpp \
    cp/msg/AcUnholdAllConnectionsMsg.cpp \
    cp/msg/AcUnholdConnectionMsg.cpp \
    cp/msg/AcUnmuteInputConnectionMsg.cpp \
    cp/msg/AcUnsubscribeMsg.cpp \
    cp/msg/AcYieldFocusMsg.cpp \
    cp/msg/CmCommandMsg.cpp \
    cp/msg/CmDestroyAbstractCallMsg.cpp \
    cp/msg/CmGainFocusMsg.cpp \
    cp/msg/CmNotificationMsg.cpp \
    cp/msg/CmYieldFocusMsg.cpp \
    cp/msg/CpTimerMsg.cpp \
    cp/msg/Sc100RelTimerMsg.cpp \
    cp/msg/Sc2xxTimerMsg.cpp \
    cp/msg/ScByeRetryTimerMsg.cpp \
    cp/msg/ScCommandMsg.cpp \
    cp/msg/ScDelayedAnswerTimerMsg.cpp \
    cp/msg/ScDisconnectTimerMsg.cpp \
    cp/msg/ScInviteExpirationTimerMsg.cpp \
    cp/msg/ScNotificationMsg.cpp \
    cp/msg/ScReestablishRtpRedirectCommandMsg.cpp \
    cp/msg/ScReInviteTimerMsg.cpp \
    cp/msg/ScRtpRedirectCompleteNotificationMsg.cpp \
    cp/msg/ScRtpRedirectErrorNotificationMsg.cpp \
    cp/msg/ScSessionTimeoutTimerMsg.cpp \
    cp/msg/ScStartRtpRedirectAckNotificationMsg.cpp \
    cp/msg/ScStartRtpRedirectCommandMsg.cpp \
    cp/msg/ScStopRtpRedirectCommandMsg.cpp \
    cp/msg/ScTimerMsg.cpp \
    cp/state/AlertingSipConnectionState.cpp \
    cp/state/BaseSipConnectionState.cpp \
    cp/state/DialingSipConnectionState.cpp \
    cp/state/DisconnectedSipConnectionState.cpp \
    cp/state/EstablishedSipConnectionState.cpp \
    cp/state/GeneralTransitionMemory.cpp \
    cp/state/IdleSipConnectionState.cpp \
    cp/state/NewCallSipConnectionState.cpp \
    cp/state/OfferingSipConnectionState.cpp \
    cp/state/QueuedSipConnectionState.cpp \
    cp/state/RemoteAlertingSipConnectionState.cpp \
    cp/state/RemoteOfferingSipConnectionState.cpp \
    cp/state/RemoteQueuedSipConnectionState.cpp \
    cp/state/SipConnectionStateContext.cpp \
    cp/state/SipConnectionStateMachine.cpp \
    cp/state/SipConnectionStateTransition.cpp \
    cp/state/SipEventTransitionMemory.cpp \
    cp/state/SipResponseTransitionMemory.cpp \
    cp/state/StateTransitionEventDispatcher.cpp \
    cp/state/UnknownSipConnectionState.cpp \
    tapi/CallStateEventMsg.cpp \
    tapi/ConferenceEventMsg.cpp \
    tapi/InfoEventMsg.cpp \
    tapi/InfoStatusEventMsg.cpp \
    tapi/KeepaliveEventMsg.cpp \
    tapi/LineStateEventMsg.cpp \
    tapi/MediaEventMsg.cpp \
    tapi/RtpRedirectEventMsg.cpp \
    tapi/SecurityEventMsg.cpp \
    tapi/SecurityHelper.cpp \
    tapi/SipXAudio.cpp \
    tapi/SipXCall.cpp \
    tapi/SipXCallEventListener.cpp \
    tapi/SipXConference.cpp \
    tapi/SipXConferenceEventListener.cpp \
    tapi/SipXConfig.cpp \
    tapi/SipXCore.cpp \
    tapi/SipXEventDispatcher.cpp \
    tapi/SipXEvents.cpp \
    tapi/SipXHandleMap.cpp \
    tapi/SipXInfo.cpp \
    tapi/SipXInfoEventListener.cpp \
    tapi/SipXInfoStatusEventListener.cpp \
    tapi/SipXKeepaliveEventListener.cpp \
    tapi/SipXLine.cpp \
    tapi/SipXLineEventListener.cpp \
    tapi/SipXMediaEventListener.cpp \
    tapi/SipXMessageObserver.cpp \
    tapi/SipXPim.cpp \
    tapi/SipXPublish.cpp \
    tapi/SipXRtpRedirectEventListener.cpp \
    tapi/SipXSecurityEventListener.cpp \
    tapi/SipXSubscribe.cpp \
    tapi/sipXtapi.cpp \
    tapi/SipXUtil.cpp

DISTCLEANFILES = Makefile.in
