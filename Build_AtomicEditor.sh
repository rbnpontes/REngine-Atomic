#!/bin/bash

# Install Dependencies
yarn --cwd Build
# Build Editor
yarn --cwd Build editor:build "$@"