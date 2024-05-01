#!/usr/bin/env bash
SOURCE=$(cd ${0%/*}; pwd)
cmake -E make_directory ../REngine-XCode-IOS && cmake -E chdir ../REngine-XCode-IOS cmake "$SOURCE" -G Xcode -DIOS=ON
echo "XCode project written to ../REngine-XCode-IOS"
