#!/bin/bash
SIF_FILE=$1

# 1. Clean environment
unset MPI_HOME
unset OPAL_PREFIX

# 2. Define the correct versions based on your confirmation
MPI_PATH="/usr/local/openmpi-4.1.6"
GP_PATH="/usr/local/GridPACK/include/gridpack"
BOOST_PATH="/usr/local/boost-1.78.0"

# 3. Export Paths for the container
export APPTAINERENV_PATH="$MPI_PATH/bin:GP_PATH:$PATH"
export APPTAINERENV_LD_LIBRARY_PATH="$GP_PATH/lib:$MPI_PATH/lib:$LD_LIBRARY_PATH"

echo "--- Compiling with GridPACK and OpenMPI 4.2.6 ---"

apptainer exec --bind .:/mnt $SIF_FILE \
  mpicxx -O3 /mnt/check_gridpack.cpp \
  -I$GP_PATH/include \
  -I$GP_PATH/parallel \
  -L$MPI_PATH/lib \
  -lgridpack -lga -lpetsc -lmpi \
  -o /mnt/check_gridpack.x