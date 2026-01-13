#!/bin/bash

mkdir -p ../build

cd ../build

cmake_args=(
    -DCMAKE_BUILD_TYPE=Debug
    -DCMAKE_INSTALL_PREFIX=bin
    ..
)

cmake "${cmake_args[@]}"

# make install -j16
make -j16

if [ "$#" -eq 1 ]; then
    cd ..
    ./export.sh $1
fi