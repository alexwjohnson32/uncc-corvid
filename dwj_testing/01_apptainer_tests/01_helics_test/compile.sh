#!/bin/bash

module purge
module load modules

SIF=$1


/opt/Software/bin/apptainer exec $SIF bash -s <<'EOF'

# CLEAN PREVIOUS RUN
rm -rf *.log
rm -rf *.csv

# Linking HELICS correctly
export LD_LIBRARY_PATH=/root/develop/helics/build/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/lib/openmpi-4.1.6/bin:$LD_LIBRARY_PATH

mkdir -p build
rm -rf build/*
cd build

cmake .. \
    -DHELICS_DIR=/usr/local/helics/lib64/cmake/HELICS \
    -DCMAKE_CXX_STANDARD=17
make

EOF