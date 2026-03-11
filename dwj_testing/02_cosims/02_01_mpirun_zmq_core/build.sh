#!/bin/bash
# Exit on any error
set -e

SIF=$1

echo "--- Starting Build Process ---"

# 1. Create a clean build directory
rm -rf build
mkdir -p build
cd build

# 2. Run CMake and Make inside the container
# We pass the C++17 standard explicitly to be safe
apptainer exec ../$SIF cmake .. -DCMAKE_CXX_STANDARD=17
apptainer exec ../$SIF make

echo "--- Build Complete! Binary is in build/gpk_fed ---"