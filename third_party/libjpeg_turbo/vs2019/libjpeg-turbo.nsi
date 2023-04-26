!include x64.nsh
Name "libjpeg-turbo SDK for Visual C++ 64-bit"
OutFile "I:/git/libjpeg-turbo-1.2.90/vs2019\${BUILDDIR}libjpeg-turbo-1.2.90-vc64.exe"
InstallDir c:\libjpeg-turbo64

SetCompressor bzip2

Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "libjpeg-turbo SDK for Visual C++ 64-bit (required)"
!ifdef WIN64
	${If} ${RunningX64}
	${DisableX64FSRedirection}
	${Endif}
!endif
	SectionIn RO
!ifdef GCC
	IfFileExists $SYSDIR/libturbojpeg.dll exists 0
!else
	IfFileExists $SYSDIR/turbojpeg.dll exists 0
!endif
	goto notexists
	exists:
!ifdef GCC
	MessageBox MB_OK "An existing version of the libjpeg-turbo SDK for Visual C++ 64-bit is already installed.  Please uninstall it first."
!else
	MessageBox MB_OK "An existing version of the libjpeg-turbo SDK for Visual C++ 64-bit or the TurboJPEG SDK is already installed.  Please uninstall it first."
!endif
	quit

	notexists:
	SetOutPath $SYSDIR
!ifdef GCC
	File "I:/git/libjpeg-turbo-1.2.90/vs2019\libturbojpeg.dll"
!else
	File "I:/git/libjpeg-turbo-1.2.90/vs2019\${BUILDDIR}turbojpeg.dll"
!endif
	SetOutPath $INSTDIR\bin
!ifdef GCC
	File "/oname=libjpeg-62.dll" "I:/git/libjpeg-turbo-1.2.90/vs2019\sharedlib\libjpeg-*.dll" 
!else
	File "I:/git/libjpeg-turbo-1.2.90/vs2019\sharedlib\${BUILDDIR}jpeg62.dll"
!endif
	File "I:/git/libjpeg-turbo-1.2.90/vs2019\sharedlib\${BUILDDIR}cjpeg.exe"
	File "I:/git/libjpeg-turbo-1.2.90/vs2019\sharedlib\${BUILDDIR}djpeg.exe"
	File "I:/git/libjpeg-turbo-1.2.90/vs2019\sharedlib\${BUILDDIR}jpegtran.exe"
	File "I:/git/libjpeg-turbo-1.2.90/vs2019\${BUILDDIR}tjbench.exe"
	File "I:/git/libjpeg-turbo-1.2.90/vs2019\${BUILDDIR}rdjpgcom.exe"
	File "I:/git/libjpeg-turbo-1.2.90/vs2019\${BUILDDIR}wrjpgcom.exe"
	SetOutPath $INSTDIR\lib
!ifdef GCC
	File "I:/git/libjpeg-turbo-1.2.90/vs2019\libturbojpeg.dll.a"
	File "I:/git/libjpeg-turbo-1.2.90/vs2019\libturbojpeg.a"
	File "I:/git/libjpeg-turbo-1.2.90/vs2019\sharedlib\libjpeg.dll.a"
	File "I:/git/libjpeg-turbo-1.2.90/vs2019\libjpeg.a"
!else
	File "I:/git/libjpeg-turbo-1.2.90/vs2019\${BUILDDIR}turbojpeg.lib"
	File "I:/git/libjpeg-turbo-1.2.90/vs2019\${BUILDDIR}turbojpeg-static.lib"
	File "I:/git/libjpeg-turbo-1.2.90/vs2019\sharedlib\${BUILDDIR}jpeg.lib"
	File "I:/git/libjpeg-turbo-1.2.90/vs2019\${BUILDDIR}jpeg-static.lib"
!endif
!ifdef JAVA
	SetOutPath $INSTDIR\classes
	File "I:/git/libjpeg-turbo-1.2.90/vs2019\java\${BUILDDIR}turbojpeg.jar"
!endif
	SetOutPath $INSTDIR\include
	File "I:/git/libjpeg-turbo-1.2.90/vs2019\jconfig.h"
	File "I:/git/libjpeg-turbo-1.2.90\jerror.h"
	File "I:/git/libjpeg-turbo-1.2.90\jmorecfg.h"
	File "I:/git/libjpeg-turbo-1.2.90\jpeglib.h"
	File "I:/git/libjpeg-turbo-1.2.90\turbojpeg.h"
	SetOutPath $INSTDIR\doc
	File "I:/git/libjpeg-turbo-1.2.90\README"
	File "I:/git/libjpeg-turbo-1.2.90\README-turbo.txt"
	File "I:/git/libjpeg-turbo-1.2.90\example.c"
	File "I:/git/libjpeg-turbo-1.2.90\libjpeg.txt"
	File "I:/git/libjpeg-turbo-1.2.90\structure.txt"
	File "I:/git/libjpeg-turbo-1.2.90\usage.txt"
	File "I:/git/libjpeg-turbo-1.2.90\wizard.txt"

	WriteRegStr HKLM "SOFTWARE\libjpeg-turbo64 1.2.90" "Install_Dir" "$INSTDIR"

	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\libjpeg-turbo64 1.2.90" "DisplayName" "libjpeg-turbo SDK v1.2.90 for Visual C++ 64-bit"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\libjpeg-turbo64 1.2.90" "UninstallString" '"$INSTDIR\uninstall_1.2.90.exe"'
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\libjpeg-turbo64 1.2.90" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\libjpeg-turbo64 1.2.90" "NoRepair" 1
	WriteUninstaller "uninstall_1.2.90.exe"
SectionEnd

Section "Uninstall"
!ifdef WIN64
	${If} ${RunningX64}
	${DisableX64FSRedirection}
	${Endif}
!endif

	SetShellVarContext all

	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\libjpeg-turbo64 1.2.90"
	DeleteRegKey HKLM "SOFTWARE\libjpeg-turbo64 1.2.90"

!ifdef GCC
	Delete $INSTDIR\bin\libjpeg-62.dll
	Delete $SYSDIR\libturbojpeg.dll
	Delete $INSTDIR\lib\libturbojpeg.dll.a"
	Delete $INSTDIR\lib\libturbojpeg.a"
	Delete $INSTDIR\lib\libjpeg.dll.a"
	Delete $INSTDIR\lib\libjpeg.a"
!else
	Delete $INSTDIR\bin\jpeg62.dll
	Delete $SYSDIR\turbojpeg.dll
	Delete $INSTDIR\lib\jpeg.lib
	Delete $INSTDIR\lib\jpeg-static.lib
	Delete $INSTDIR\lib\turbojpeg.lib
	Delete $INSTDIR\lib\turbojpeg-static.lib
!endif
!ifdef JAVA
	Delete $INSTDIR\classes\turbojpeg.jar
!endif
	Delete $INSTDIR\bin\cjpeg.exe
	Delete $INSTDIR\bin\djpeg.exe
	Delete $INSTDIR\bin\jpegtran.exe
	Delete $INSTDIR\bin\tjbench.exe
	Delete $INSTDIR\bin\rdjpgcom.exe
	Delete $INSTDIR\bin\wrjpgcom.exe
	Delete $INSTDIR\include\jconfig.h"
	Delete $INSTDIR\include\jerror.h"
	Delete $INSTDIR\include\jmorecfg.h"
	Delete $INSTDIR\include\jpeglib.h"
	Delete $INSTDIR\include\turbojpeg.h"
	Delete $INSTDIR\uninstall_1.2.90.exe
	Delete $INSTDIR\doc\README
	Delete $INSTDIR\doc\README-turbo.txt
	Delete $INSTDIR\doc\example.c
	Delete $INSTDIR\doc\libjpeg.txt
	Delete $INSTDIR\doc\structure.txt
	Delete $INSTDIR\doc\usage.txt
	Delete $INSTDIR\doc\wizard.txt

	RMDir "$INSTDIR\include"
	RMDir "$INSTDIR\lib"
	RMDir "$INSTDIR\doc"
!ifdef JAVA
	RMDir "$INSTDIR\classes"
!endif
	RMDir "$INSTDIR\bin"
	RMDir "$INSTDIR"

SectionEnd
