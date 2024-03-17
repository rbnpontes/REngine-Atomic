@echo off

set WORKING_DIR=%~1\Build
set BUILD_ARGS=%~2

echo Generating Script Bindings
echo Working Dir: %WORKING_DIR%
echo Build Args: %BUILD_ARGS%
yarn --cwd %WORKING_DIR% build %BUILD_ARGS%
echo Finished