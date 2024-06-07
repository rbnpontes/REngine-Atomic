@echo off
REM for some reason on Windows
REM cmake execute_script can't execute 'yarn' command
REM so this script is a forward declaration just to call yarn

echo Args: %*
yarn --cwd %cd% %*