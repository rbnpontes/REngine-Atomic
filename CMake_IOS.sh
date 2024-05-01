#!/usr/bin/env bash
SOURCE=$(cd ${0%/*}; pwd)
cmake -E make_directory ../REngine-XCode && cmake -E chdir ../REngine-XCode cmake "$SOURCE" -DIOS -G Xcode
echo "XCode project written to ../REngine-XCode"
