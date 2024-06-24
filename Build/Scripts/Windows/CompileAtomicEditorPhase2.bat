@echo OFF
REM this script is used by the build system
REM they call vs tools vars to start developer command
REM on any terminal.

set VS_TOOLS=%2
set last_path=%cd%
cd %VS_TOOLS%

call VsDevCmd.bat
cd %last_path%

msbuild /m Atomic.sln /t:AtomicEditor /t:AtomicPlayer /p:Configuration=%1 /p:Platform=x64
