@echo OFF
REM this script is used by the build system
REM they call vs tools vars to start developer command
REM on any terminal.

set VS_TOOLS=%2

echo Initializing Vs Dev Command
call %VS_TOOL%\VsDevCmd.bat
echo Start Building
msbuild /m Atomic.sln /t:AtomicEditor /t:AtomicPlayer /p:Configuration=%1 /p:Platform=x64
