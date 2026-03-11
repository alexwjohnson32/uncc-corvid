#!/bin/bash

SIF_FILE=$1

/opt/Software/bin/apptainer exec $SIF_FILE /usr/local/openmpi-4.1.6/bin/mpirun -n 2 ./build/check_gridpack.x