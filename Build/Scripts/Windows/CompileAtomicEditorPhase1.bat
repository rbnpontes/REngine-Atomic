@echo off
REM this script is used by the build system
REM they call vs tools vars to start developer command
REM on any terminal.

set VS_TOOLS=%2

echo VS_TOOLS: %VS_TOOLS%

set last_path=%cd%

echo Switching to VS TOOLS
cd %VS_TOOLS%
echo Error Level: %errorlevel%

call VsDevCmd.bat
cd %last_path%

:: Note, we're building LibCpuId as it uses masm as getting XamlFactory load errors if delayed
msbuild /m Atomic.sln /t:LibCpuId /t:AtomicNETNative /p:Configuration=%1 /p:Platform=x64