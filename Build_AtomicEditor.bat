@echo OFF

REM Install Dependencies
yarn --cwd Build
REM Build Editor
yarn --cwd Build editor:build %*
