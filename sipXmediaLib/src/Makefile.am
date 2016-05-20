## Process this file with automake to produce Makefile.in

SUBDIRS = test

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../sipXportLib/include

lib_LTLIBRARIES = libsipXmedia.la


## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

libsipXmedia_la_LDFLAGS = \
	-version-info ${version_Current}:${version_Revision}:${version_Age} \
	@GSM_LDFLAGS@

if DISABLE_LOCAL_AUDIO
LOCAL_AUDIO_CXXFLAGS = -DDISABLE_LOCAL_AUDIO
endif

libsipXmedia_la_CXXFLAGS = \
	-D_REALTIME_LINUX_AUDIO_THREADS \
	$(LOCAL_AUDIO_CXXFLAGS) \
	@GSM_CXXFLAGS@

# Disabled SPEEX_CFLAGS, broken
# SPEEX_CFLAGS

libsipXmedia_la_LIBADD = \
	@SIPXTACK_LIBS@ \
	@SIPXPORT_LIBS@ \
	@CA_LIBS@ \
	@GSM_LIBS@ \
	-lstdc++

# Disabled SPEEX_LIB, broken
# SPEEX_LIBS@

libsipXmedia_la_SOURCES =  \
    ../config/sipxmedialib-buildstamp.h \
    ../config/sipxmedialib-buildstamp.cpp \
    mp/DSPlib.cpp \
    mp/MpArrayBuf.cpp \
    mp/MpAsyncPortAudioStream.cpp \
    mp/MpAudioAbstract.cpp \
    mp/MpAudioBuf.cpp \
    mp/MpAudioDeviceInfo.cpp \
    mp/MpAudioDriverBase.cpp \
    mp/MpAudioDriverFactory.cpp \
    mp/MpAudioDriverManager.cpp \
    mp/MpAudioFileDecompress.cpp \
    mp/MpAudioFileOpen.cpp \
    mp/MpAudioFileUtils.cpp \
    mp/MpAudioMixerBase.cpp \
    mp/MpAudioResource.cpp \
    mp/MpAudioStreamInfo.cpp \
    mp/MpAudioStreamParameters.cpp \
    mp/MpAudioUtils.cpp \
    mp/MpAudioWaveFileRead.cpp \
    mp/MpBridgeAlgLinear.cpp \
    mp/MpBridgeAlgSimple.cpp \
    mp/MpBuf.cpp \
    mp/MpBufPool.cpp \
    mp/MpBufferMsg.cpp \
    mp/MpCallFlowGraph.cpp \
    mp/MpCodecFactory.cpp \
    mp/MpCodecInfo.cpp \
    mp/MpDataBuf.cpp \
    mp/MpDecodeBuffer.cpp \
    mp/MpDecoderBase.cpp \
    mp/MpDspUtils.cpp \
    mp/MpEncoderBase.cpp \
    mp/MpFlowGraphBase.cpp \
    mp/MpFlowGraphMsg.cpp \
    mp/MpHostAudioApiInfo.cpp \
    mp/MpJitterBufferBase.cpp \
    mp/MpJitterBufferDefault.cpp \
    mp/MpJitterBufferSpeex.cpp \
    mp/MpMMTimer.cpp \
    mp/MpMediaTask.cpp \
    mp/MpMediaTaskHelper.cpp \
    mp/MpMediaTaskMsg.cpp \
    mp/MpMisc.cpp \
    mp/MpNoiseGeneratorBase.cpp \
    mp/MpNoiseGeneratorDefault.cpp \
    mp/MpNoiseGeneratorFactory.cpp \
    mp/MpNoiseGeneratorSpanDsp.cpp \
    mp/MpNotificationDispatcher.cpp \
    mp/MpPortAudioDriver.cpp \
    mp/MpPortAudioMixer.cpp \
    mp/MpPortAudioStreamBase.cpp \
    mp/MpResamplerBase.cpp \
    mp/MpResamplerDefault.cpp \
    mp/MpResamplerFactory.cpp \
    mp/MpResamplerSpeex.cpp \
    mp/MpResource.cpp \
    mp/MpResourceMsg.cpp \
    mp/MpResourceSortAlg.cpp \
    mp/MpRtpBuf.cpp \
    mp/MpRtpInputAudioConnection.cpp \
    mp/MpRtpInputConnection.cpp \
    mp/MpRtpOutputAudioConnection.cpp \
    mp/MpRtpOutputConnection.cpp \
    mp/MpSipxDecoders.cpp \
    mp/MpStopDTMFTimerMsg.cpp \
    mp/MpSyncPortAudioStream.cpp \
    mp/MpTimerMsg.cpp \
    mp/MpTypes.cpp \
    mp/MpUdpBuf.cpp \
    mp/MpdIPPG7221.cpp \
    mp/MpdIPPG7231.cpp \
    mp/MpdIPPG728.cpp \
    mp/MpdIPPG729.cpp \
    mp/MpdIPPG7291.cpp \
    mp/MpdIPPG729i.cpp \
    mp/MpdIPPGAmr.cpp \
    mp/MpdIPPGAmrWb.cpp \
    mp/MpdIPPGSM.cpp \
    mp/MpdIPPPcma.cpp \
    mp/MpdIPPPcmu.cpp \
    mp/MpdPtAVT.cpp \
    mp/MpdSipxG722.cpp \
    mp/MpdSipxG726.cpp \
    mp/MpdSipxGSM.cpp \
    mp/MpdSipxILBC.cpp \
    mp/MpdSipxL16.cpp \
    mp/MpdSipxPcma.cpp \
    mp/MpdSipxPcmu.cpp \
    mp/MpdSipxSpeex.cpp \
    mp/MpdSipxSpeexUWb \
    mp/MpdSipxSpeexWb.cpp \
    mp/MpeIPPG7221.cpp \
    mp/MpeIPPG7231.cpp \
    mp/MpeIPPG728.cpp \
    mp/MpeIPPG729.cpp \
    mp/MpeIPPG7291.cpp \
    mp/MpeIPPG729i.cpp \
    mp/MpeIPPGAmr.cpp \
    mp/MpeIPPGAmrWb.cpp \
    mp/MpeIPPGSM.cpp \
    mp/MpeIPPPcma.cpp \
    mp/MpeIPPPcmu.cpp \
    mp/MpePtAVT.cpp \
    mp/MpeSipxG722.cpp \
    mp/MpeSipxG726.cpp \
    mp/MpeSipxGSM.cpp \
    mp/MpeSipxILBC.cpp \
    mp/MpeSipxL16.cpp \
    mp/MpeSipxPcma.cpp \
    mp/MpeSipxPcmu.cpp \
    mp/MpeSipxSpeex.cpp \
    mp/MpeSipxSpeexUWb.cpp \
    mp/MpeSipxSpeexWb.cpp \
    mp/MprBridge.cpp \
    mp/MprDecode.cpp \
    mp/MprSimpleDtmfDetector.cpp \
    mp/MprSpanDspDtmfDetector.cpp \
    mp/MprDejitter.cpp \
    mp/MprDtmfDetectorFactory.cpp \
    mp/MprEncode.cpp \
    mp/MprFromFile.cpp \
    mp/MprFromMic.cpp \
    mp/MprFromNet.cpp \
    mp/MprMixer.cpp \
    mp/MprNull.cpp \
    mp/MprRecorder.cpp \
    mp/MprSpeexEchoCancel.cpp \
    mp/MprSpeexPreProcess.cpp \
    mp/MprSplitter.cpp \
    mp/MprToNet.cpp \
    mp/MprToSpkr.cpp \
    mp/MprToneGen.cpp \
    mp/NetInTask.cpp \
    mp/dtmflib.cpp \
    mp/mpG711.cpp \
    mp/mpau.cpp \
    mp/resamplingby2.cpp \
    rtcp/BaseClass.cpp \
    rtcp/ByeReport.cpp \
    rtcp/Message.cpp \
    rtcp/MsgQueue.cpp \
    rtcp/NetworkChannel.cpp \
    rtcp/RTCManager.cpp \
    rtcp/RTCPConnection.cpp \
    rtcp/RTCPHeader.cpp \
    rtcp/RTCPRender.cpp \
    rtcp/RTCPSession.cpp \
    rtcp/RTCPSource.cpp \
    rtcp/RTCPTimer.cpp \
    rtcp/RTPHeader.cpp \
    rtcp/ReceiverReport.cpp \
    rtcp/SenderReport.cpp \
    rtcp/SourceDescription.cpp
