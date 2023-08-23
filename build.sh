#!/bin/zsh

echo 'Building kernel...'
export PATH="${1}:${PATH}"
cmake -S . -B cmake_build
cmake --build cmake_build
cmake --install cmake_build