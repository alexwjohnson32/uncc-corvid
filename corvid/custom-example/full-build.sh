#!/bin/bash

./clean_dir.sh

rm -rf gridpack/build/*

cd gridpack/build
cmake ..
make -j10