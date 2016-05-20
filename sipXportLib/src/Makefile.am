## Process this file with automake to produce Makefile.in
include $(top_builddir)/config/sipXcommon.mak

SUBDIRS = . test

lib_LTLIBRARIES = libsipXport.la

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/../sipXcallLib/include

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=2
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0

libsipXport_la_LDFLAGS = \
	-version-info ${version_Current}:${version_Revision}:${version_Age} \
	@PCRE_LDFLAGS@ \
	@SSL_LDFLAGS@

libsipXport_la_LIBADD = \
	@PCRE_LIBS@ \
	@SSL_LIBS@ \
	@RT_LIBS@ \
	-ldl \
	-lpthread  \
	-lstdc++

# this is helpful if you're debugging something with lots of inlines (like the UtlLink code)
# 		-fno-gcse -O0 -fno-implement-inlines -fno-default-inline

libsipXport_la_CXXFLAGS = \
	@PCRE_CFLAGS@ \
	@SSL_CXXFLAGS@ \
	-DSIPX_TMPDIR=\"@SIPX_TMPDIR@\" \
	-DSIPX_CONFDIR=\"@SIPX_CONFDIR@\"

libsipXport_la_SOURCES =  \
    ../config/sipxportlib-buildstamp.h \
    ../config/sipxportlib-buildstamp.cpp \
    hmac-sha1/hmac-sha1.c \
    hmac-sha1/sha1.c \
    hmac-sha1/sha1.h \
    os/NatMsg.cpp \
    os/OsAssert.c \
    os/OsCSemBase.cpp \
    os/OsCallback.cpp \
    os/OsConfigDb.cpp \
    os/OsConnectionSocket.cpp \
    os/OsContact.cpp \
    os/OsContactList.cpp \
    os/OsDatagramSocket.cpp \
    os/OsDateTime.cpp \
    os/OsDirBase.cpp \
    os/OsEncryption.cpp \
    os/OsEvent.cpp \
    os/OsEventMsg.cpp \
    os/OsExcept.cpp \
    os/OsFileBase.cpp \
    os/OsFileInfoBase.cpp \
    os/OsFileIteratorBase.cpp \
    os/OsFileSystem.cpp \
    os/OsIntPtrMsg.cpp \
    os/OsLockingList.cpp \
    os/OsMsg.cpp \
    os/OsMsgDispatcher.cpp \
    os/OsMsgPool.cpp \
    os/OsMsgQ.cpp \
    os/OsMulticastSocket.cpp \
    os/OsNameDb.cpp \
    os/OsNameDbInit.cpp \
    os/OsNatAgentTask.cpp \
    os/OsNatDatagramSocket.cpp \
    os/OsNatSocketBaseImpl.cpp \
    os/OsNetworkAdapterInfo.cpp \
    os/OsPathBase.cpp \
    os/OsProcessBase.cpp \
    os/OsProcessIteratorBase.cpp \
    os/OsProcessMgr.cpp \
    os/OsProtectEvent.cpp \
    os/OsProtectEventMgr.cpp \
    os/OsProtectEventMgrInit.cpp \
    os/OsPtrMsg.cpp \
    os/OsQueuedEvent.cpp \
    os/OsQueuedNotification.cpp \
    os/OsRWMutex.cpp \
    os/OsRpcMsg.cpp \
    os/OsSSL.cpp \
    os/OsSSLConnectionSocket.cpp \
    os/OsSSLServerSocket.cpp \
    os/OsStunResultFailureMsg.cpp \
    os/OsStunResultMsg.cpp \
    os/OsStunResultSuccessMsg.cpp \
    os/OsServerSocket.cpp \
    os/OsServerTask.cpp \
    os/OsSharedLibMgr.cpp \
    os/OsSharedServerTask.cpp \
    os/OsSharedServerTaskMgr.cpp \
    os/OsSharedServerTaskMsg.cpp \
    os/OsSharedServerTaskWorker.cpp \
    os/OsSharedTaskInfo.cpp \
    os/OsSocket.cpp \
    os/OsSysLog.cpp \
    os/OsSysLogFacilities.cpp \
    os/OsSysLogMsg.cpp \
    os/OsSysLogTask.cpp \
    os/OsTask.cpp \
    os/OsTime.cpp \
    os/OsTimeLog.cpp \
    os/OsTimer.cpp \
    os/OsTimerMsg.cpp \
    os/OsTimerNotification.cpp \
    os/OsTimerTaskCommandMsg.cpp \
    os/OsTimerTask.cpp \
    os/OsTokenizer.cpp \
    os/OsUtil.cpp \
    os/StunMessage.cpp \
    os/StunUtils.cpp \
    os/TurnMessage.cpp \
    os/linux/OsBSemLinux.cpp \
    os/linux/OsCSemLinux.cpp \
    os/linux/OsDateTimeLinux.cpp \
    os/linux/OsDirLinux.cpp \
    os/linux/OsFileInfoLinux.cpp \
    os/linux/OsFileIteratorLinux.cpp \
    os/linux/OsFileLinux.cpp \
    os/linux/OsMutexLinux.cpp \
    os/linux/OsMutexLinuxC.c \
    os/linux/OsNetworkLinux.cpp \
    os/linux/OsPathLinux.cpp \
    os/linux/OsProcessIteratorLinux.cpp \
    os/linux/OsProcessLinux.cpp \
    os/linux/OsSharedLibMgrLinux.cpp \
    os/linux/OsTaskLinux.cpp \
    os/linux/OsUtilLinux.cpp \
    os/linux/clock_gettime.c \
    os/linux/host_address.c \
    os/linux/pt_csem.c \
    os/linux/pt_mutex.c \
    os/osPrintf.cpp \
    os/shared/OsMsgQShared.cpp \
    os/shared/OsTimerMessage.cpp \
    utl/PluginHooks.cpp \
    utl/UtlBool.cpp \
    utl/UtlChainPool.cpp \
    utl/UtlContainable.cpp \
    utl/UtlContainableAtomic.cpp \
    utl/UtlContainer.cpp \
    utl/UtlCopyableSList.cpp \
    utl/UtlCopyableSortedList.cpp \
    utl/UtlCrc32.cpp \
    utl/UtlDList.cpp \
    utl/UtlDListIterator.cpp \
    utl/UtlDateTime.cpp \
    utl/UtlHashBag.cpp \
    utl/UtlHashBagIterator.cpp \
    utl/UtlHashMap.cpp \
    utl/UtlHashMapIterator.cpp \
    utl/UtlHistogram.cpp \
    utl/UtlInit.cpp \
    utl/UtlInt.cpp \
    utl/UtlIterator.cpp \
    utl/UtlLink.cpp \
    utl/UtlList.cpp \
    utl/UtlListIterator.cpp \
    utl/UtlLongLongInt.cpp \
    utl/UtlObservableImpl.cpp \
    utl/UtlRandom.cpp \
    utl/UtlRegex.cpp \
    utl/UtlRscStore.cpp \
    utl/UtlRscTrace.cpp \
    utl/UtlSList.cpp \
    utl/UtlSListIterator.cpp \
    utl/UtlSortedList.cpp \
    utl/UtlSortedListIterator.cpp \
    utl/UtlString.cpp \
    utl/UtlTokenizer.cpp \
    utl/UtlVoidPtr.cpp \
    utl/XmlContent.cpp \
    xmlparser/ExtractContent.cpp \
    xmlparser/TiXmlIterator.cpp \
    xmlparser/tinystr.cpp \
    xmlparser/tinyxml.cpp \
    xmlparser/tinyxmlerror.cpp \
    xmlparser/tinyxmlparser.cpp

EXTRA_DIST= \
    os/Wnt/getWindowsDNSServers.cpp \
    os/Wnt/OsBSemWnt.cpp \
    os/Wnt/OsCSemWnt.cpp \
    os/Wnt/OsDateTimeWnt.cpp \
    os/Wnt/OsDirWnt.cpp \
    os/Wnt/OsFileInfoWnt.cpp \
    os/Wnt/OsFileIteratorWnt.cpp \
    os/Wnt/OsFileSystemWnt.cpp \
    os/Wnt/OsFileWnt.cpp \
    os/Wnt/OsMutexWnt.cpp \
    os/Wnt/OsPathWnt.cpp \
    os/Wnt/OsProcessIteratorWnt.cpp \
    os/Wnt/OsProcessWnt.cpp \
    os/Wnt/OsSharedLibMgrWnt.cpp \
    os/Wnt/OsTaskWnt.cpp \
    os/Wnt/OsTimerMessage.cpp \
    os/Wnt/OsUtilWnt.cpp \
    os/Wnt/WindowsAdapterInfo.cpp 
