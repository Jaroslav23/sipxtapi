## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/../sipXcallLib/include -I$(top_srcdir)/interface/include

nobase_includeHEADERS_INSTALL = $(INSTALL) -D -p -c -m 644

nobase_include_HEADERS =  \
    include/mi/CpAudioDeviceInfo.h \
    include/mi/CpMediaInterface.h \
    include/mi/CpMediaInterfaceFactoryFactory.h \
    include/mi/CpMediaInterfaceFactory.h
    
noinst_LTLIBRARIES = libmediainterface.la

libmediainterface_la_LIBADD =  \
	@SIPXMEDIA_LIBS@

libmediainterface_la_SOURCES =  \
    src/mi/CpAudioDeviceInfo.cpp \
    src/mi/CpMediaInterface.cpp \
    src/mi/CpMediaInterfaceFactory.cpp
