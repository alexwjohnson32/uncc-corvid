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

echo Starting 2bus-13bus Run

export UNCC_ROOT="$(cd "$(pwd)/.." && pwd)"

# Linking HELICS correctly
export LD_LIBRARY_PATH=/root/develop/helics/build/lib:$LD_LIBRARY_PATH

# Changing to the correct dir
cd $UNCC_ROOT/examples/2bus-13bus
mkdir -p outputs
rm -rf outputs/*

mkdir -p build
rm -rf build/*
cd build

# Build
cmake ..
make -j10

cd $UNCC_ROOT/examples/2bus-13bus

# Source venv
source /opt/myenv/bin/activate

# Running example
helics run --path=gpk-gld-cosim.json

echo Run Complete
EOF
