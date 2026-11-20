@echo off
rem test.bat        - builds, then runs every test
rem test.bat items  - builds, then runs only the tests whose name contains "items"
call "%~dp0build.bat"
if errorlevel 1 exit /b 1

if "%~1"=="" (
    ctest --test-dir "%~dp0build" --output-on-failure
) else (
    ctest --test-dir "%~dp0build" --output-on-failure -R "%~1"
)
if not errorlevel 1 exit /b 0

rem A virus scanner can hold a freshly built .exe for a moment, which shows up as
rem "resource busy or locked". Wait a few seconds and run just the failures once more.
ping -n 5 127.0.0.1 >nul
ctest --test-dir "%~dp0build" --output-on-failure --rerun-failed
