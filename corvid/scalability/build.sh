#!/bin/bash

cd build

cmake_args=(
    -DCMAKE_BUILD_TYPE=Debug
    -DCMAKE_INSTALL_PREFIX=bin
    ..
)

cmake "${cmake_args[@]}"

make install -j16