; Script generated by the HM NIS Edit Script Wizard.

; HM NIS Edit Wizard helper defines
!define PRODUCT_NAME "TinyCAD"
!define PRODUCT_VERSION "2.70.00"
!define PRODUCT_PUBLISHER "TinyCAD"
!define PRODUCT_WEB_SITE "http://tinycad.sourceforge.net"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\TinyCad.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; MUI 1.67 compatible ------
!include "MUI.nsh"

; For the DLL installer
!include "UpgradeDll.nsh"

; MUI Settings
!define MUI_ABORTWARNING
!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

; Welcome page
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "licence.rtf"
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_RUN "$INSTDIR\TinyCad.exe"
!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!insertmacro MUI_UNPAGE_INSTFILES

; Language files
!insertmacro MUI_LANGUAGE "English"

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "Setup.exe"
InstallDir "$PROGRAMFILES\TinyCAD"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show

ShowUnInstDetails show

Section "MainSection" SEC01

; Download MDAC (if necessary)
  IfFileExists $SYSDIR\msjet40.dll skip_mdac_download

  MessageBox MB_OK "TinyCAD requires the Microsoft MDAC components, these will be downloaded now"
  CreateDirectory "$INSTDIR"
  NSISdl::download /TIMEOUT=30000 http://tinycad.sourceforge.net/installer/MDAC_TYP.EXE $INSTDIR/mdac_typ.exe
  Pop $R0 ;Get the return value
  StrCmp $R0 "success" +3
    MessageBox MB_OK "Download failed: $R0.  Please try to install again or download and run MDAC_TYPE.exe from Microsoft directly."
    Quit

  ExecWait '"$INSTDIR/MDAC_TYP.EXE" /Q'

skip_mdac_download:


  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File "..\src\Debug\TinyCad.exe"
  CreateDirectory "$SMPROGRAMS\TinyCAD"
  CreateShortCut "$SMPROGRAMS\TinyCAD\TinyCAD.lnk" "$INSTDIR\TinyCad.exe"
  CreateShortCut "$DESKTOP\TinyCAD.lnk" "$INSTDIR\TinyCad.exe"
  File "..\src\png\libpng13.dll"
  File "..\src\png\zlib1.dll"
  File "..\src\libiconv\iconv.dll"
  File "..\docs\TinyCAD.chm"
  
  CreateShortCut "$SMPROGRAMS\TinyCAD\Help.lnk" "$INSTDIR\TinyCAD.chm"
  SetOutPath "$INSTDIR\library"
  File "..\libs\74TTL.mdb"
  File "..\libs\AC connectors.mdb"
  File "..\libs\Analog.mdb"
  File "..\libs\Connectors.mdb"
  File "..\libs\DISCRETE.mdb"
  File "..\libs\IC-CMOS4000.mdb"
  File "..\libs\IC-OPAMP.mdb"
  File "..\libs\IC-VREG.mdb"
  File "..\libs\Microcontroller.mdb"
  File "..\libs\passive2.mdb"
  File "..\libs\passive.mdb"
  File "..\libs\semi.mdb"
  File "..\libs\switches.mdb"
  File "..\libs\valve.mdb"
  SetOutPath "$DOCUMENTS\TinyCAD Examples"
  File "..\examples\AMP.DSN"
  File "..\examples\AtTiny LED Flasher.dsn"
  File "..\examples\face.dsn"
  File "..\examples\INVERTER.DSN"
  File "..\examples\KEYSW.DSN"
  File "..\examples\nanocomp6802.dsn"
  File "..\examples\VCA.DSN"
  File "..\examples\VREG.DSN"

; Install the MSVC components
;  !insertmacro UpgradeDLL "c:\Windows\System32\msvcrt40.dll" "$SYSDIR\msvcrt40.dll" "$SYSDIR"
;  !insertmacro UpgradeDLL "c:\Windows\System32\msvcirt.dll" "$SYSDIR\msvcirt.dll" "$SYSDIR"


; Install the DAO components
  ReadRegStr $R0 HKLM "Software\Microsoft\Windows\CurrentVersion" "CommonFilesDir"
  Strcmp $R0 "" create_dao36_common copy_dao36_common

create_dao36_common:
; If the common files setting didn't exist, create the directory
; Set the value and write it
	StrCpy $R0 "$PROGRAMFILES\Common Files"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion" "CommonFilesDir" $R0
copy_dao36_common:

  ; Create the entire directory path to the shared DAO directory
  CreateDirectory "$R0"
  CreateDirectory "$R0\Microsoft Shared"
  CreateDirectory "$R0\Microsoft Shared\DAO"
  StrCpy $R9 "$R0\Microsoft Shared\DAO"

  SetOutPath $R9
  File "dao\DAO2535.TLB"
  !insertmacro UpgradeDLL "dao\dao360.dll" "$R9\dao360.dll" "$R9"
  !insertmacro UpgradeDLL "dao\dao350.dll" "$R9\dao350.dll" "$R9"



SectionEnd

Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\TinyCAD\TinyCAD Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\TinyCAD\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\TinyCad.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\TinyCad.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd


Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section Uninstall
  Delete "$INSTDIR\${PRODUCT_NAME}.url"
  Delete "$INSTDIR\uninst.exe"
  Delete "$DOCUMENTS\TinyCAD Examples\VREG.DSN"
  Delete "$DOCUMENTS\TinyCAD Examples\VCA.DSN"
  Delete "$DOCUMENTS\TinyCAD Examples\nanocomp6802.dsn"
  Delete "$DOCUMENTS\TinyCAD Examples\KEYSW.DSN"
  Delete "$DOCUMENTS\TinyCAD Examples\INVERTER.DSN"
  Delete "$DOCUMENTS\TinyCAD Examples\face.dsn"
  Delete "$DOCUMENTS\TinyCAD Examples\AtTiny LED Flasher.dsn"
  Delete "$DOCUMENTS\TinyCAD Examples\AMP.DSN"
  Delete "$INSTDIR\library\valve.mdb"
  Delete "$INSTDIR\library\switches.mdb"
  Delete "$INSTDIR\library\semi.mdb"
  Delete "$INSTDIR\library\passive.mdb"
  Delete "$INSTDIR\library\passive2.mdb"
  Delete "$INSTDIR\library\Microcontroller.mdb"
  Delete "$INSTDIR\library\IC-VREG.mdb"
  Delete "$INSTDIR\library\IC-OPAMP.mdb"
  Delete "$INSTDIR\library\IC-CMOS4000.mdb"
  Delete "$INSTDIR\library\DISCRETE.mdb"
  Delete "$INSTDIR\library\Connectors.mdb"
  Delete "$INSTDIR\library\Analog.mdb"
  Delete "$INSTDIR\library\AC connectors.mdb"
  Delete "$INSTDIR\library\74TTL.mdb"
  Delete "$INSTDIR\TinyCAD.chm"
  Delete "$INSTDIR\zlib1.dll"
  Delete "$INSTDIR\libpng13.dll"
  Delete "$INSTDIR\TinyCad.exe"

  Delete "$SMPROGRAMS\TinyCAD\Uninstall.lnk"
  Delete "$SMPROGRAMS\TinyCAD\TinyCAD Website.lnk"
  Delete "$SMPROGRAMS\TinyCAD\Help.lnk"
  Delete "$DESKTOP\TinyCAD.lnk"
  Delete "$SMPROGRAMS\TinyCAD\TinyCAD.lnk"

  RMDir "$SMPROGRAMS\TinyCAD"
  RMDir "$INSTDIR\library"
  RMDir "$INSTDIR"
  RMDir "$DOCUMENTS\TinyCAD Examples"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd
