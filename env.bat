@echo off
rem env.bat - puts the Qt toolchain on PATH for this console only.
rem The other .bat files call this first. Nothing is installed or changed system-wide.
set "QT_KIT=C:\Qt\6.11.2\mingw_64"
set "PATH=C:\Qt\Tools\CMake_64\bin;C:\Qt\Tools\Ninja;C:\Qt\Tools\mingw1310_64\bin;%QT_KIT%\bin;%PATH%"
