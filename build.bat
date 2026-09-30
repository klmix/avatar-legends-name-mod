@echo off
rem Builds build\namemod.dll. Needs Visual Studio 2022 (or Build Tools) with the C++ workload.
setlocal
cd /d "%~dp0"
set "VSW=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSW%" for /f "usebackq delims=" %%i in (`"%VSW%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VS=%%i"
if not defined VS set "VS=C:\Program Files\Microsoft Visual Studio\2022\Community"
if not exist "%VS%\VC\Auxiliary\Build\vcvars64.bat" (echo Visual Studio C++ tools not found. & exit /b 1)
call "%VS%\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
if not exist build mkdir build
cl /nologo /LD /O2 /MT /W3 /D_CRT_SECURE_NO_WARNINGS src\namemod.cpp /Fobuild\ /Febuild\namemod.dll /link /IMPLIB:build\namemod.lib || exit /b 1
