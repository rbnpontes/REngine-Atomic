@echo off
REM this script is used by the build system
REM they call vs tools vars to start developer command
REM on any terminal.

set VS_TOOLS=%2
set SOLUTION_NAME=%3

echo Initializing Vs Dev Command
call %VS_TOOLS%\VsDevCmd.bat
echo Start Building
:: Note, we're building LibCpuId as it uses masm as getting XamlFactory load errors if delayed
msbuild /m %SOLUTION_NAME% /t:ThirdParty\LibCpuId "/t:Engine Code\AtomicNETNative" /p:Configuration=%1 /p:Platform=x64