#!/bin/bash

cd build

cmake_args=(
    -DCMAKE_BUILD_TYPE=Debug
    ..
)

cmake "${cmake_args[@]}"

make -j16