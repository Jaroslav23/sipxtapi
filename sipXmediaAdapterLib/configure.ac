#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(sipXmediaAdapterLib, 3.3, sipx-dev@list.sipfoundry.org)
AC_CONFIG_AUX_DIR(config)
AC_PREFIX_DEFAULT([/usr/local/sipx])
AM_INIT_AUTOMAKE(foreign)
SFAC_AUTOMAKE_VERSION([1.6])
CHECK_AUTOCONF

# Configure pkg-config
PKG_PROG_PKG_CONFIG

AC_CONFIG_SRCDIR([sipXmediaMediaProcessing/src/sipXmediaFactoryImpl.cpp])
AM_CONFIG_HEADER([config.h])
AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
CHECK_LIBRT
CHECK_COREAUDIO
CHECK_CARBON

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([arpa/inet.h fcntl.h malloc.h netdb.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UID_T
AC_C_VOLATILE

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_FORK
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_CHECK_FUNCS([clock_gettime getcwd gethostbyname gethostname gettimeofday inet_ntoa localtime_r memchr memset mkdir putenv rmdir select setenv socket strchr strdup strerror strrchr strstr strtol strtoul tzset])

# Look for CPPUnit libraries
CHECK_CPPUNIT

# Look for GSM and Speex codecs
CHECK_GSM
#CHECK_SPEEX

CHECK_PCRE
CHECK_SSL

# sipX dependencies
SFAC_INIT_FLAGS
SFAC_LIB_MEDIA

# Conditionally use the GIPS audio libraries
CHECK_GIPSNEQ
CHECK_GIPSVE
CHECK_GIPSCE
CHECK_VIDEO

ENABLE_DOXYGEN
AM_CONDITIONAL(DOC, test x$enable_doc = xyes)
AM_CONDITIONAL(USE_BLDNO, test x$enable_buildnumber = xyes)

# Enable local audio support (microphone / speaker)
AC_ARG_ENABLE(local-audio,
[  --disable-local-audio    Disable local audio (speaker and microphone)],
[ case "${enableval}" in
  no) disable_local_audio=true ;;
  yes) disable_local_audio=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --disable-local-audio) ;;
esac],[disable_local_audio=false])
AM_CONDITIONAL(DISABLE_LOCAL_AUDIO, test x$disable_local_audio = xtrue)

AC_CONFIG_FILES([
  Makefile 
  config/sipXcommon.mak
  interface/Makefile
  sipXmediaMediaProcessing/Makefile
  sipXmediaMediaProcessing/src/Makefile
  sipXmediaMediaProcessing/src/test/Makefile
  sipXmediaMediaProcessing/include/Makefile
])

AC_OUTPUT
