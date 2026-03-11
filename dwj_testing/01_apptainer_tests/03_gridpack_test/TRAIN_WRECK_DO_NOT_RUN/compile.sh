#!/bin/bash
# Run this inside the container: apptainer shell apptainer_image.sif

# Define paths (matching your /usr/local structure)
GRIDPACK_DIR="/usr/local/gridpack"
GA_DIR="/usr/local/ga-5.8"
BOOST_DIR="/usr/local/boost-1.82"
PETSC_DIR="/usr/local/petsc-3.20"

mpicxx -O3 check_gridpack.cpp \
    -I${GRIDPACK_DIR}/include \
    -L${GRIDPACK_DIR}/lib -lgridpack \
    -L${GA_DIR}/lib -lga -lcomex \
    -L${BOOST_DIR}/lib -lboost_serialization -lboost_mpi -lboost_system \
    -L${PETSC_DIR}/lib -lpetsc \
    -DHAS_HELICS \
    -o check_gridpack.x
