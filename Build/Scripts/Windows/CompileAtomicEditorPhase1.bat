@echo off
REM this script is used by the build system
REM they call vs tools vars to start developer command
REM on any terminal.

set VS_TOOLS=%2
set last_path=%cd%
cd %VS_TOOLS%

call VsDevCmd.bat
cd %last_path%

:: Note, we're building LibCpuId as it uses masm as getting XamlFactory load errors if delayed
msbuild /m Atomic.sln /t:LibCpuId /t:AtomicNETNative /p:Configuration=%1 /p:Platform=x64