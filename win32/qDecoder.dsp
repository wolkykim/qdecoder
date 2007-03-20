# Microsoft Developer Studio Project File - Name="qDecoder" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=qDecoder - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "qDecoder.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "qDecoder.mak" CFG="qDecoder - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "qDecoder - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "qDecoder - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "qDecoder - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x412 /d "NDEBUG"
# ADD RSC /l 0x412 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "qDecoder - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x412 /d "_DEBUG"
# ADD RSC /l 0x412 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "qDecoder - Win32 Release"
# Name "qDecoder - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\md5\md5c.c
# End Source File
# Begin Source File

SOURCE=..\src\qArg.c
# End Source File
# Begin Source File

SOURCE=..\src\qAwk.c
# End Source File
# Begin Source File

SOURCE=..\src\qCount.c
# End Source File
# Begin Source File

SOURCE=..\src\qDecoder.c
# End Source File
# Begin Source File

SOURCE=..\src\qDownload.c
# End Source File
# Begin Source File

SOURCE=..\src\qEncode.c
# End Source File
# Begin Source File

SOURCE=..\src\qEnv.c
# End Source File
# Begin Source File

SOURCE=..\src\qError.c
# End Source File
# Begin Source File

SOURCE=..\src\qfDecoder.c
# End Source File
# Begin Source File

SOURCE=..\src\qFile.c
# End Source File
# Begin Source File

SOURCE=..\src\qHttpHeader.c
# End Source File
# Begin Source File

SOURCE=..\src\qInternalCommon.c
# End Source File
# Begin Source File

SOURCE=..\src\qInternalEntry.c
# End Source File
# Begin Source File

SOURCE=..\src\qMisc.c
# End Source File
# Begin Source File

SOURCE=..\src\qsDecoder.c
# End Source File
# Begin Source File

SOURCE=..\src\qSed.c
# End Source File
# Begin Source File

SOURCE=..\src\qSession.c
# End Source File
# Begin Source File

SOURCE=..\src\qString.c
# End Source File
# Begin Source File

SOURCE=..\src\qTime.c
# End Source File
# Begin Source File

SOURCE=..\src\qValid.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\md5\md5.h
# End Source File
# Begin Source File

SOURCE=..\src\md5\md5_global.h
# End Source File
# Begin Source File

SOURCE=..\src\qDecoder.h
# End Source File
# Begin Source File

SOURCE=..\src\qInternal.h
# End Source File
# End Group
# End Target
# End Project
