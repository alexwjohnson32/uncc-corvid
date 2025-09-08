#!/bin/bash

# Exit if no argument provided
if [ -z "$1" ]; then
  echo "Usage: $0 <container.sif>"
  exit 1
fi

SIF_FILE="$1"

clear

# Purging modules to avoid loaded options that are different
module purge

apptainer exec $SIF_FILE bash -s <<'EOF'

echo Starting Run

export UNCC_ROOT="$(cd "$(pwd)/.." && pwd)"

# Changing to the correct dir
cd $UNCC_ROOT/examples/simple-cosim/
mkdir -p outputs
rm -rf outputs/*

# Moving library dependencies around
g++ -o gridpack_federate gridpack_federate.cpp    -I/root/develop/helics/src    -I/root/develop/helics/build/include/helics_cxx    -L/root/develop/helics/build/lib    -lhelics    -lstdc++

# Linking HELICS correctly
export LD_LIBRARY_PATH=/root/develop/helics/build/lib:$LD_LIBRARY_PATH

# Source venv
source /opt/myenv/bin/activate

# Running example
helics run --path=switch_cosim_runner.json

echo Run Complete
EOF
