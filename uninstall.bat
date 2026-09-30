@echo off
rem Puts Valve's original steam_api64.dll back.
setlocal
set "HERE=%~dp0"
set "GAME="
if exist "%HERE%Atla.exe" set "GAME=%HERE%"
if exist "%HERE%..\Atla.exe" set "GAME=%HERE%..\"
if not defined GAME (echo Can't find the game folder. & pause & exit /b 1)
cd /d "%GAME%"
tasklist /fi "imagename eq Atla.exe" | "%SystemRoot%\System32\find.exe" /i "Atla.exe" >nul && (echo Close the game first. & pause & exit /b 1)

"%SystemRoot%\System32\findstr.exe" /m /c:"AVATAR-LEGENDS-NAME-MOD" steam_api64.dll >nul 2>&1
if errorlevel 1 (
    rem a game update already put the original back, the backup is outdated
    if exist steam_api64_o.dll del steam_api64_o.dll
    echo The mod wasn't active. Cleaned up.
) else (
    if not exist steam_api64_o.dll (echo Backup missing. Verify the game files in Steam instead. & pause & exit /b 1)
    move /y steam_api64_o.dll steam_api64.dll >nul
    echo Removed. The game uses your Steam name again.
)
pause
