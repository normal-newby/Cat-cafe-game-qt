@echo off
rem run.bat - builds if needed, then starts the game.
call "%~dp0build.bat"
if errorlevel 1 (
    echo.
    echo The build failed. Scroll up for the first error.
    pause
    exit /b 1
)
start "" "%~dp0build\CatCafe.exe"
