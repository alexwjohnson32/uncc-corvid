#!/bin/bash

SIF_FILE="./rl8_uncc.sif"

clear

# Purging modules to avoid loaded options that are different
module purge

apptainer exec $SIF_FILE bash -s <<'EOF'

echo Starting Run

export UNCC_ROOT=$(pwd)

# Changing to the correct dir
cd $UNCC_ROOT/examples/simple-cosim/
mkdir -p outputs
rm -rf outputs/*

# Moving library dependencies around
g++ -o gridpack_federate gridpack_federate.cpp    -I/root/develop/helics/src    -I/root/develop/helics/build/include/helics_cxx    -L/root/develop/helics/build/lib    -lhelics    -lstdc++

# Linking HELICS correctly
export LD_LIBRARY_PATH=/root/develop/helics/build/lib:$LD_LIBRARY_PATH

# Running example
helics run --path=switch_cosim_runner.json

echo Run Complete
EOF
