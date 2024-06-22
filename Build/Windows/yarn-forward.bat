@echo off
REM for some reason on Windows
REM cmake execute_script can't execute 'yarn' command
REM so this script is a forward declaration just to call yarn

set initial_path=%cd%

REM navigate to build directory
cd %~dp0
cd ..

REM acquire build directory
set curr_path=%cd%

REM go back to initial directory
cd %initial_path%

echo Args: %*
yarn --cwd %curr_path% %*