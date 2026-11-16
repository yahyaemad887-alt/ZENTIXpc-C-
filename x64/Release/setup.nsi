; --- Zentix PC Ultimate Final Installer ---
Unicode true
!define APP_NAME "Zentix PC"
!define COMP_NAME "Zentix Suite"
!define PRODUCT_VERSION "1.0.0"

Name "${APP_NAME}"
OutFile "Zentix_PC_Installer.exe"
InstallDir "$PROGRAMFILES64\ZentixPC"
RequestExecutionLevel admin

; استخدام واجهة Modern UI 2 ومكتبة الحوارات
!include "MUI2.nsh"
!include "nsDialogs.nsh"

; فرض الأيقونات على واجهة Modern UI 2
!define MUI_ICON "zentix.ico"
!define MUI_UNICON "zentix.ico"

; 1. ترتيب صفحات التثبيت الأساسية (وإدراج صفحة الـ RTSS بينهم)
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
Page custom RTSSPageCreate RTSSPageLeave
!insertmacro MUI_PAGE_INSTFILES

; صفحة النهاية مع تشغيل التطبيق تلقائياً
!define MUI_FINISHPAGE_RUN "$INSTDIR\zentix pc.exe"
!define MUI_FINISHPAGE_RUN_TEXT "تشغيل Zentix PC فوراً بعد الانتهاء"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; 2. تعريف اللغات بنفس ترتيب الـ C الخاص بك (0 إلى 8)
!insertmacro MUI_LANGUAGE "English"     ; Index 0
!insertmacro MUI_LANGUAGE "Arabic"      ; Index 1
!insertmacro MUI_LANGUAGE "German"      ; Index 2
!insertmacro MUI_LANGUAGE "French"      ; Index 3
!insertmacro MUI_LANGUAGE "Japanese"    ; Index 4
!insertmacro MUI_LANGUAGE "Korean"      ; Index 5
!insertmacro MUI_LANGUAGE "Russian"     ; Index 6
!insertmacro MUI_LANGUAGE "Turkish"     ; Index 7
!insertmacro MUI_LANGUAGE "SimpChinese" ; Index 8

; 3. إظهار نافذة اختيار اللغات عند بدء السيت اب
Function .onInit
  !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

; ============================================================================
; تصميم صفحة الـ RTSS المستقلة (بالإنجليزية لضمان المظهر الاحترافي)
; ============================================================================
Var RTSS_Dialog
Var RTSS_LinkBtn

Function RTSSPageCreate
    nsDialogs::Create 1018
    Pop $RTSS_Dialog
    
    ${If} $RTSS_Dialog == error
        Abort
    ${EndIf}

    ; 1. Title
    ${NSD_CreateLabel} 0 10 100% 20 "RivaTuner Statistics Server (RTSS) Requirement"
    Pop $R0
    SetCtlColors $R0 0x0000FF Transparent

    ; 2. Description lines
    ${NSD_CreateLabel} 0 38 100% 20 "For proper FPS and Hardware Telemetry monitoring inside games,"
    Pop $R1

    ${NSD_CreateLabel} 0 60 100% 20 "Zentix PC requires RivaTuner Statistics Server (RTSS) to be installed."
    Pop $R2

    ${NSD_CreateLabel} 0 88 100% 20 "If it is not installed on your system, please download it below:"
    Pop $R3

    ; 3. Download Button
    ${NSD_CreateButton} 0 122 240 32 "Download RTSS 7.3.7 (Official Site)"
    Pop $RTSS_LinkBtn
    ${NSD_OnClick} $RTSS_LinkBtn OnClickRTSSDownload

    nsDialogs::Show
FunctionEnd

Function OnClickRTSSDownload
    ExecShell "open" "https://share.google/hqQrJYwFQ7WsZdY1f"
FunctionEnd

Function RTSSPageLeave
FunctionEnd
; ============================================================================

Section "CoreFiles" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite on

  ; أ. نسخ التطبيق، مجلد الأصوات raw، وكل ملفات الـ DLL
  File "zentix pc.exe"
  File "*.dll"
  File /r "raw"

  ; ب. إخفاء ملفات الـ DLL لتنظيف المجلد أمام المستخدم
  nsExec::Exec 'attrib +h "$INSTDIR\*.dll"'

  ; ج. مطابقة لغة التثبيت بأرقام الـ Index الصريحة الخاصة بتطبيقك
  StrCmp $LANGUAGE ${LANG_ENGLISH} set_en
  StrCmp $LANGUAGE ${LANG_ARABIC} set_ar
  StrCmp $LANGUAGE ${LANG_GERMAN} set_de
  StrCmp $LANGUAGE ${LANG_FRENCH} set_fr
  StrCmp $LANGUAGE ${LANG_JAPANESE} set_ja
  StrCmp $LANGUAGE ${LANG_KOREAN} set_ko
  StrCmp $LANGUAGE ${LANG_RUSSIAN} set_ru
  StrCmp $LANGUAGE ${LANG_TURKISH} set_tr
  StrCmp $LANGUAGE ${LANG_SIMPCHINESE} set_zh
  Goto set_en ; الافتراضي

  set_en:
    StrCpy $R0 "0"
    Goto DoWrite
  set_ar:
    StrCpy $R0 "1"
    Goto DoWrite
  set_de:
    StrCpy $R0 "2"
    Goto DoWrite
  set_fr:
    StrCpy $R0 "3"
    Goto DoWrite
  set_ja:
    StrCpy $R0 "4"
    Goto DoWrite
  set_ko:
    StrCpy $R0 "5"
    Goto DoWrite
  set_ru:
    StrCpy $R0 "6"
    Goto DoWrite
  set_tr:
    StrCpy $R0 "7"
    Goto DoWrite
  set_zh:
    StrCpy $R0 "8"
    Goto DoWrite

  DoWrite:
    FileOpen $0 "$INSTDIR\config.ini" w
    FileWrite $0 $R0
    FileClose $0

  ; د. إنشاء الاختصارات على سطح المكتب وفي قائمة ابدأ
  CreateShortCut "$DESKTOP\Zentix PC.lnk" "$INSTDIR\zentix pc.exe"
  CreateDirectory "$SMPROGRAMS\ZentixPC"
  CreateShortCut "$SMPROGRAMS\ZentixPC\Zentix PC.lnk" "$INSTDIR\zentix pc.exe"
  CreateShortCut "$SMPROGRAMS\ZentixPC\Uninstall.lnk" "$INSTDIR\uninstall.exe"

  ; هـ. تسجيل أداة الإزالة في النظام
  WriteUninstaller "$INSTDIR\uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ZentixPC" "DisplayName" "Zentix PC"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ZentixPC" "UninstallString" "$INSTDIR\uninstall.exe"
SectionEnd

Section "Uninstall"
  nsExec::Exec 'attrib -h "$INSTDIR\*.dll"'

  Delete "$INSTDIR\zentix pc.exe"
  Delete "$INSTDIR\*.dll"
  Delete "$INSTDIR\config.ini"
  Delete "$INSTDIR\uninstall.exe"
  RMDir /r "$INSTDIR\raw"
  RMDir "$INSTDIR"

  Delete "$DESKTOP\Zentix PC.lnk"
  Delete "$SMPROGRAMS\ZentixPC\Zentix PC.lnk"
  Delete "$SMPROGRAMS\ZentixPC\Uninstall.lnk"
  RMDir "$SMPROGRAMS\ZentixPC"
  
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\ZentixPC"
SectionEnd