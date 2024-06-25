@echo off
REM this script is used by the build system
REM they call vs tools vars to start developer command
REM on any terminal.

set VS_TOOLS=%2

echo Initializing Vs Dev Command
call %VS_TOOLS%\VsDevCmd.bat

:: Note, we're building LibCpuId as it uses masm as getting XamlFactory load errors if delayed
msbuild /m Atomic.sln /t:LibCpuId /t:AtomicNETNative /p:Configuration=%1 /p:Platform=x64