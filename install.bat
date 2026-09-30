@echo off
rem Installs the name mod. Run it again after every game update.
setlocal
set "HERE=%~dp0"
set "DLL=%HERE%namemod.dll"
if not exist "%DLL%" set "DLL=%HERE%build\namemod.dll"
if not exist "%DLL%" (echo namemod.dll is missing. Put it next to install.bat. & goto fail)

set "GAME="
if exist "%HERE%Atla.exe" set "GAME=%HERE%"
if exist "%HERE%..\Atla.exe" set "GAME=%HERE%..\"
if not defined GAME (
    echo Can't find the game. Put this folder inside the game folder:
    echo Steam, right-click Avatar Legends, Manage, Browse local files.
    goto fail
)
cd /d "%GAME%"
tasklist /fi "imagename eq Atla.exe" | "%SystemRoot%\System32\find.exe" /i "Atla.exe" >nul && (echo Close the game first. & goto fail)

"%SystemRoot%\System32\findstr.exe" /m /c:"AVATAR-LEGENDS-NAME-MOD" steam_api64.dll >nul 2>&1
if errorlevel 1 (
    rem steam_api64.dll is Valve's original, either a first install or a game update put it back
    copy /y steam_api64.dll steam_api64_o.dll >nul || goto fail
) else if not exist steam_api64_o.dll (
    echo steam_api64_o.dll is missing. Verify the game files in Steam, then run this again.
    goto fail
)
copy /y "%DLL%" steam_api64.dll >nul || goto fail
if not exist playername.txt type nul > playername.txt

echo Installed. Notepad opens playername.txt now. Type your name, save, start the game.
start "" notepad playername.txt
pause
exit /b 0

:fail
echo Install failed.
pause
exit /b 1
