# preserve header timestamps to preserve incremental builds across
# projects
nobase_includeHEADERS_INSTALL = $(INSTALL) -D -p -c -m 644

## If we use windows, make these conditional includes
EXTRA_DIST = \
    resparse/wnt/arpa/inet.h \
    resparse/wnt/arpa/nameser.h \
    resparse/wnt/crypt.h \
    resparse/wnt/inet_aton.h \
    resparse/wnt/netinet/in.h \
    resparse/wnt/nterrno.h \
    resparse/wnt/resolv/resolv.h \
    resparse/wnt/res_signal.h \
    resparse/wnt/sys/param.h \
    resparse/wnt/sys/uio.h \
    resparse/wnt/sysdep.h \
    resparse/wnt/utilNT.h

nobase_include_HEADERS =  \
    net/HttpBody.h \
    net/HttpConnection.h \
    net/HttpConnectionMap.h \
    net/HttpMessage.h \
    net/HttpRequestContext.h \
    net/HttpServer.h \
    net/HttpService.h \
    net/MailAttachment.h \
    net/MailMessage.h \
    net/MimeBodyPart.h \
    net/NameValuePair.h \
    net/NameValuePairInsensitive.h \
    net/NameValueTokenizer.h \
    net/NetAttributeTokenizer.h \
    net/NetBase64Codec.h \
    net/NetMd5Codec.h \
    net/PidfBody.h \
    net/QoS.h \
    net/SdpBody.h \
    net/SipCallIdGenerator.h \
    net/SipClient.h \
    net/SipContact.h \
    net/SipContactDb.h \
    net/SipContactSelector.h \
    net/SipDialog.h \
    net/SipDialogEvent.h \
    net/SipDialogMgr.h \
    net/SipDialogMonitor.h \
    net/SipInfoEventListener.h \
    net/SipInfoStatusEventListener.h \
    net/SipLine.h \
    net/SipLineAlias.h \
    net/SipLineCredential.h \
    net/SipLineList.h \
    net/SipLineMgr.h \
    net/SipLineProvider.h \
    net/SipLineStateEventListener.h \
    net/SipMessage.h \
    net/SipMessageEvent.h \
    net/SipMessageList.h \
    net/SipNonceDb.h \
    net/SipNotifyStateTask.h \
    net/SipObserverCriteria.h \
    net/SipPimClient.h \
    net/SipPresenceEvent.h \
    net/SipProtocolServerBase.h \
    net/SipPublishContentMgr.h \
    net/SipPublishServer.h \
    net/SipPublishServerEventStateCompositor.h \
    net/SipPublishServerEventStateMgr.h \
    net/SipRefreshManager.h \
    net/SipRefreshMgr.h \
    net/SipRequestContext.h \
    net/SipResourceList.h \
    net/SipSecurityEventListener.h \
    net/SipServerBase.h \
    net/SipServerBroker.h \
    net/SipServerBrokerListener.h \
    net/SipSrvLookup.h \
    net/SipSubscribeClient.h \
    net/SipSubscribeServer.h \
    net/SipSubscribeServerEventHandler.h \
    net/SipSubscriptionMgr.h \
    net/SipTagGenerator.h \
    net/SipTcpServer.h \
    net/msg/SipTimerMsg.h \
    net/SipTlsServer.h \
    net/SipTransaction.h \
    net/SipTransactionList.h \
    net/SipTransport.h \
    net/SipUdpServer.h \
    net/SipUserAgent.h \
    net/SipUserAgentBase.h \
    net/SmimeBody.h \
    net/StateChangeNotifier.h \
    net/Url.h \
    net/pk12wrapper.h \
    net/version.h \
    resparse/bzero.h \
    resparse/ns_name.h \
    resparse/poll.h \
    resparse/res_config.h \
    resparse/res_info.h \
    resparse/rr.h \
    resparse/types.h \
    tapi/SipXHandleMap.h \
    tapi/SipXMessageObserver.h \
    tapi/SipXEventDispatcher.h \
    tapi/doxyfile \
    tapi/sipXtapi.h \
    tapi/sipXtapiEvents.h

net/version.h: .FORCE
	@test -d net || mkdir net
	@echo '#ifndef SIP_STACK_VERSION'       > net/version_new.h
	@echo -n '#define SIP_STACK_VERSION "' >> net/version_new.h
	@echo -n @VERSION@                     >> net/version_new.h
	@echo    '"'                           >> net/version_new.h
	@echo '#endif'                         >> net/version_new.h
	@if test ! -r net/version.h || ! diff net/version.h net/version_new.h > /dev/null  \
	;then \
	   echo sipXtack version is @VERSION@ \
	;  mv net/version_new.h net/version.h \
	;else \
	   rm -f net/version_new.h \
	;fi

.FORCE:
