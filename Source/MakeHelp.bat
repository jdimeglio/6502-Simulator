@echo off
REM -- First make map file from Microsoft Visual C++ generated resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by 6502.HPJ. >"hlp\6502.hm"
echo. >>"hlp\6502.hm"
echo // Commands (ID_* and IDM_*) >>"hlp\6502.hm"
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>"hlp\6502.hm"
echo. >>"hlp\6502.hm"
echo // Prompts (IDP_*) >>"hlp\6502.hm"
makehm IDP_,HIDP_,0x30000 resource.h >>"hlp\6502.hm"
echo. >>"hlp\6502.hm"
echo // Resources (IDR_*) >>"hlp\6502.hm"
makehm IDR_,HIDR_,0x20000 resource.h >>"hlp\6502.hm"
echo. >>"hlp\6502.hm"
echo // Dialogs (IDD_*) >>"hlp\6502.hm"
makehm IDD_,HIDD_,0x20000 resource.h >>"hlp\6502.hm"
echo. >>"hlp\6502.hm"
echo // Frame Controls (IDW_*) >>"hlp\6502.hm"
makehm IDW_,HIDW_,0x50000 resource.h >>"hlp\6502.hm"
echo // Obiekty w oknach dialogowych (IDC_*) >>"hlp\6502.hm"
makehm IDC_,HIDC_,0xA0000 resource.h >>"hlp\6502.hm"
echo. >>"hlp\6502.hm"
REM -- Make help for Project 6502


echo Building Win32 Help files
start /wait hcrtf -x "hlp\6502.hpj"
echo.
if exist Debug\nul copy "hlp\6502.hlp" Debug
if exist Debug\nul copy "hlp\6502.cnt" Debug
if exist Release\nul copy "hlp\6502.hlp" Release
if exist Release\nul copy "hlp\6502.cnt" Release
if exist Static\nul copy "hlp\6502.hlp" Static
if exist Static\nul copy "hlp\6502.cnt" Static
echo.


