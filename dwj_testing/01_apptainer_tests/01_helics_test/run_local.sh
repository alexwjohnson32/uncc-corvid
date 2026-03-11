#!/bin/bash

module purge
module load modules

SIF=$1

if false; then 
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
fi


# Running example
#--METHOD 1: TIME
#cd ..
echo ls
#mpirun -n 4 openmpi-run.conf

module load openmpi/4.1.6


#HOST_MPI="/opt/Software/openmpi/gcc/13.2.0/4.1.6"
#export APPTAINER_BIND="$HOST_MPI:$HOST_MPI"
#export APPTAINERENV_LD_LIBRARY_PATH="$HOST_MPI/lib:$HOST_MPI/lib/openmpi:/usr/local/helics/lib64:$LD_LIBRARY_PATH"
#mpirun -np 1 apptainer exec $SIF /usr/local/helics/bin/helics_broker --coretype=mpi --federates=2 --loglevel=4 : \
#    -np 1 apptainer exec $SIF ./build/messageFed-cpp --name=fed1 --target=fed2 --coretype=mpi --coreinit="--broker_address=0:0" : \
#    -np 1 apptainer exec $SIF ./build/messageFed-cpp --name=fed2 --target=fed1 --coretype=mpi --coreinit="--broker_address=0:0"

# Host MPI Path Setup
HOST_MPI="/opt/Software/openmpi/gcc/13.2.0/4.1.6"
export APPTAINER_BIND="$HOST_MPI:$HOST_MPI"
export APPTAINERENV_LD_LIBRARY_PATH="$HOST_MPI/lib:$HOST_MPI/lib/openmpi:/usr/local/helics/lib64:$LD_LIBRARY_PATH"

# Run the co-simulation
# Changed --loglevel=4 to --loglevel=debug (or warning/summary)
mpirun -np 1 apptainer exec $SIF /usr/local/helics/bin/helics_broker --coretype=mpi --federates=2 --loglevel=debug : \
    -np 1 apptainer exec $SIF ./build/messageFed-cpp --name=fed1 --target=fed2 --coretype=mpi --coreinit="--broker_address=0:0" : \
    -np 1 apptainer exec $SIF ./build/messageFed-cpp --name=fed2 --target=fed1 --coretype=mpi --coreinit="--broker_address=0:0"


echo Run Complete
