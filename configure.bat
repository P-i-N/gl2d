@echo off

if [%1]==[] goto autodetect

premake5 %1
goto done

:autodetect
set GENERATOR=""

rem Detect Visual Studio 2013
rem -------------------------
if exist "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\bin" (
set GENERATOR="vs2013"
echo Visual Studio 2013 detected!
) else (
set GENERATOR=%GENERATOR%
)

rem Detect Visual Studio 2015
rem -------------------------
if exist "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin" (
set GENERATOR="vs2015"
echo Visual Studio 2015 detected!
) else (
set GENERATOR=%GENERATOR%
)

if %GENERATOR%=="" (
echo Visual Studio not detected! You must specify a premake generator manually.
) else (
premake5 %GENERATOR%
)

:done
