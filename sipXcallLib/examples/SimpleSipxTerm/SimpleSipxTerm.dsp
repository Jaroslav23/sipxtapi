# Microsoft Developer Studio Project File - Name="SimpleSipxTerm" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=SimpleSipxTerm - Win32 Debug_LIB
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SimpleSipxTerm.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SimpleSipxTerm.mak" CFG="SimpleSipxTerm - Win32 Debug_LIB"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SimpleSipxTerm - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "SimpleSipxTerm - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "SimpleSipxTerm - Win32 Debug_LIB" (based on "Win32 (x86) Application")
!MESSAGE "SimpleSipxTerm - Win32 Release_LIB" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SimpleSipxTerm - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 Winmm.lib sipXtapi.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "SimpleSipxTerm - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "SIPXTAPI_STATIC" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Winmm.lib sipXtapid.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "SimpleSipxTerm - Win32 Debug_LIB"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "SimpleSipxTerm___Win32_Debug_LIB"
# PROP BASE Intermediate_Dir "SimpleSipxTerm___Win32_Debug_LIB"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "SimpleSipxTerm___Win32_Debug_LIB"
# PROP Intermediate_Dir "SimpleSipxTerm___Win32_Debug_LIB"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "SIPXTAPI_STATIC" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_LIB" /D "SIPXTAPI_STATIC" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 Winmm.lib sipXtapi.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Winmm.lib sipXtackLib.lib sipXsdpLib.lib sipXportLib.lib sipXmediaLib.lib sipXmediaAdapterLib.lib sipXcallLib.lib pcre.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "SimpleSipxTerm - Win32 Release_LIB"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "SimpleSipxTerm___Win32_Release_LIB"
# PROP BASE Intermediate_Dir "SimpleSipxTerm___Win32_Release_LIB"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "SimpleSipxTerm___Win32_Release_LIB"
# PROP Intermediate_Dir "SimpleSipxTerm___Win32_Release_LIB"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "SIPXTAPI_STATIC" /D "PCRE_STATIC" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 Winmm.lib sipXtapi.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 Winmm.lib sipXtackLib.lib sipXsdpLib.lib sipXportLib.lib sipXmediaLib.lib sipXmediaAdapterLib.lib sipXcallLib.lib pcre.lib /nologo /subsystem:windows /machine:I386

!ENDIF 

# Begin Target

# Name "SimpleSipxTerm - Win32 Release"
# Name "SimpleSipxTerm - Win32 Debug"
# Name "SimpleSipxTerm - Win32 Debug_LIB"
# Name "SimpleSipxTerm - Win32 Release_LIB"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\SimpleSipxTerm.cpp
# End Source File
# Begin Source File

SOURCE=.\SimpleSipxTerm.rc
# End Source File
# Begin Source File

SOURCE=.\SimpleSipxTermDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\SipPhoneDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StartupSettingsDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=C:\_VSS_\ServWare\Components\External\sipXtapi\Include\tapi\GipsDefs.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\SimpleSipxTerm.h
# End Source File
# Begin Source File

SOURCE=.\SimpleSipxTermDlg.h
# End Source File
# Begin Source File

SOURCE=.\SipPhoneDlg.h
# End Source File
# Begin Source File

SOURCE=C:\_VSS_\ServWare\Components\External\sipXtapi\Include\tapi\SipXEventDispatcher.h
# End Source File
# Begin Source File

SOURCE=C:\_VSS_\ServWare\Components\External\sipXtapi\Include\tapi\SipXHandleMap.h
# End Source File
# Begin Source File

SOURCE=C:\_VSS_\ServWare\Components\External\sipXtapi\Include\tapi\SipXMessageObserver.h
# End Source File
# Begin Source File

SOURCE=C:\_VSS_\ServWare\Components\External\sipXtapi\Include\tapi\sipXtapi.h
# End Source File
# Begin Source File

SOURCE=C:\_VSS_\ServWare\Components\External\sipXtapi\Include\tapi\sipXtapiEvents.h
# End Source File
# Begin Source File

SOURCE=C:\_VSS_\ServWare\Components\External\sipXtapi\Include\tapi\sipXtapiInternal.h
# End Source File
# Begin Source File

SOURCE=.\StartupSettingsDlg.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\SimpleSipxTerm.ico
# End Source File
# Begin Source File

SOURCE=.\res\SimpleSipxTerm.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
