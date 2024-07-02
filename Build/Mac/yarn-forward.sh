#!/usr/bin/env bash
export initial_path=$(pwd)
cd $(dirname ${BASH_SOURCE[0]})
cd ..

export build_path=$(pwd)
cd $initial_path

yarn --cwd $build_path $@