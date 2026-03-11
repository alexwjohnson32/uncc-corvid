#!/bin/bash
SIF=$1

# 1. MPI Paths & Bind Logic
HOST_MPI_ROOT="/opt/Software"
HOST_MPI_BIN="/opt/Software/openmpi/gcc/13.2.0/4.1.6/bin"
export APPTAINER_BIND="/opt/Software:/opt/Software"
export PATH="$HOST_MPI_BIN:$PATH"

# 2. Fix UCX/MPI Container Permissions
export APPTAINERENV_UCX_TLS="self,sm,tcp"
export APPTAINERENV_UCX_POSIX_USE_PROC_LINK=n

# 3. HELICS Environment
export APPTAINERENV_LD_LIBRARY_PATH="/usr/local/helics/lib64:/usr/local/lib:/opt/Software/openmpi/gcc/13.2.0/4.1.6/lib:/opt/Software/openmpi/gcc/13.2.0/4.1.6_ucx/lib"
export APPTAINERENV_HELICS_BROKER="localhost"
export APPTAINERENV_HELICS_PORT="23404"

# 4. Cleanup
echo "Cleaning up..."
pkill -u $USER -f "helics_broker|gridlabd|gpk_fed" || true
fuser -k 23404/tcp 2>/dev/null
sleep 2

# 5. Launch Broker (with 30s timeout to allow for setup/teardown)
echo "Launching Broker..."
apptainer exec $SIF helics_broker --coretype=zmq --federates=2 --port=23404 --timeout=30s &
sleep 5

# 6. Launch GridLAB-D
echo "Launching GridLAB-D..."
apptainer exec $SIF gridlabd gld_model.glm &
sleep 2

# 7. Launch GridPACK via Host MPI
# stdbuf -oL ensures output is printed line-by-line rather than buffered
echo "Launching GridPACK via Host MPI..."
mpirun -np 4 \
  -mca btl ^openib \
  -mca btl_vader_single_copy_mechanism none \
  -x UCX_TLS=self,sm,tcp \
  -x UCX_POSIX_USE_PROC_LINK=n \
  stdbuf -oL apptainer exec $SIF ./build/gpk_fed &

# 8. Wait for all background processes
echo "Co-simulation running..."
wait
echo "Co-simulation Finished."