@echo off

REM Generate Android Projects
yarn --cwd Build android:gen
REM Build Android Projects
yarn --cwd Build android:build
REM Copy Compiled Artifacts into Lib directory
yarn --cwd Build android:cpylibs
