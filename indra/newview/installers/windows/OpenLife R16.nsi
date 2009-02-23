Name "OpenLife R16"

Page directory
Page instfiles

InstallDir "$PROGRAMFILES\OpenLife R16"
VIProductVersion "0.1.1.16"

OutFile "OpenLife R16.exe"

XPStyle on

RequestExecutionLevel admin

Function .onInit
  #Show a nice splashcreen on init
  InitPluginsDir
  File /oname=$PLUGINSDIR\3dxviewer.bmp ".\3dxviewer.bmp"
  advsplash::show 1000 600 400 -1 $PLUGINSDIR\3dxviewer
  Pop $0

  Delete $PLUGINSDIR\3dxviewer.bmp
FunctionEnd
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; Variables
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

Var INSTFLAGS

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Delete files in Documents and Settings\<user>\OpenLife
; Delete files in Documents and Settings\All Users\OpenLife
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
Function DocumentsAndSettingsFolder

; Delete files in Documents and Settings\<user>\OpenLife
Push $0
Push $1
Push $2

  DetailPrint "Deleting files in Documents and Settings folder"

  StrCpy $0 0 ; Index number used to iterate via EnumRegKey

  LOOP:
    EnumRegKey $1 HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList" $0
    StrCmp $1 "" DONE               ; no more users

    ReadRegStr $2 HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList\$1" "ProfileImagePath" 
    StrCmp $2 "" CONTINUE 0         ; "ProfileImagePath" value is missing

    ; Required since ProfileImagePath is of type REG_EXPAND_SZ
    ExpandEnvStrings $2 $2

	; If uninstalling a normal install remove everything
	; Otherwise (preview/dmz etc) just remove cache
    StrCmp $INSTFLAGS "" RM_ALL RM_CACHE
      RM_ALL:
        RMDir /r "$2\Application Data\OpenLife"
        GoTo CONTINUE
      RM_CACHE:
        RMDir /r "$2\Application Data\OpenLife\Cache"
        Delete "$2\Application Data\OpenLife\user_settings\settings_windlight.xml"

  CONTINUE:
    IntOp $0 $0 + 1
    Goto LOOP
  DONE:

Pop $2
Pop $1
Pop $0

; Delete files in Documents and Settings\All Users\OpenLife
Push $0
  ReadRegStr $0 HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" "Common AppData"
  StrCmp $0 "" +2
  RMDir /r "$0\OpenLife"
Pop $0

; Delete filse in C:\Windows\Application Data\OpenLife
; If the user is running on a pre-NT system, Application Data lives here instead of
; in Documents and Settings.
RMDir /r "$WINDIR\Application Data\OpenLife"

FunctionEnd

Section ""
  SetOutPath $INSTDIR
  File /r OpenLifeR16\*.*
  
  call DocumentsAndSettingsFolder

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenLife" \
                   "DisplayName" "OpenLife"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenLife" \
                   "DisplayVersion" "0.1.1.16"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenLife" \
                   "UninstallString" "$INSTDIR\uninstaller.exe"

                   WriteUninstaller "$INSTDIR\uninstaller.exe"
		   
		   
SectionEnd



Section "Start Menu Shortcuts"
  SetShellVarContext all
   
   CreateDirectory "$SMPROGRAMS\OpenLife R16"
   CreateShortCut "$SMPROGRAMS\OpenLife R16\OpenLifeR16.lnk" "$INSTDIR\OpenLife R16.exe"
   CreateShortCut "$SMPROGRAMS\OpenLife R16\Uninstall.lnk" "$INSTDIR\uninstaller.exe"
   CreateShortCut "$DESKTOP\OpenLife R16.lnk" "$INSTDIR\OpenLife R16.exe"
   
   
SectionEnd

Section "Uninstall"
  RMDir /r $INSTDIR
  
  #removing the APPDATA\roaming\OpenLife doesn't seem to work properly on Vista. (maybe incorrect user information is given through add/remove programs)
  SetShellVarContext all
  ;;RMDir /r "$APPDATA\Roaming\OpenLife"
 ;;RMDir /r "$2\Application Data\OpenLife"
  RMDir /r "$SMPROGRAMS\OpenLife R16"
  Delete "$DESKTOP\OpenLife R16.lnk"
  SetShellVarContext current
 ;; RMDir /r "$APPDATA\Roaming\OpenLife"
 ;;RMDir /r "$2\Application Data\OpenLife"
  RMDir /r "$SMPROGRAMS\OpenLife R16"
  Delete "$DESKTOP\OpenLife R16.lnk"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenLife"
   
SectionEnd