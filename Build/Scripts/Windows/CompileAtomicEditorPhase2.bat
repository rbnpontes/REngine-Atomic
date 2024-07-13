@echo OFF
REM this script is used by the build system
REM they call vs tools vars to start developer command
REM on any terminal.

set VS_TOOLS=%2
set SOLUTION_NAME=%3
set EDITOR_NAME=%4

echo Initializing Vs Dev Command
call %VS_TOOLS%\VsDevCmd.bat
echo Start Building
msbuild /m %SOLUTION_NAME% "/t:Engine Code\%EDITOR_NAME%" /t:AtomicPlayer /p:Configuration=%1 /p:Platform=x64
