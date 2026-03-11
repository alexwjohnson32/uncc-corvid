#!/bin/bash

SIF_FILE=$1

# 1. Configure (tells CMake to find all dependencies automatically)
apptainer exec --bind .:/mnt $SIF_FILE cmake -S /mnt -B /mnt/build

# 2. Compile
apptainer exec --bind .:/mnt $SIF_FILE make -C /mnt/build