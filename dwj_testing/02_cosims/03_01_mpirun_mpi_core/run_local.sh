#!/bin/bash
SIF=$1

# 1. Define and EXPORT Paths (Crucial for -x to work)
export HOST_MPI_ROOT="/opt/Software"
export HOST_MPI_BIN="/opt/Software/openmpi/gcc/13.2.0/4.1.6/bin"
export HOST_MPI_LIBS="/opt/Software/openmpi/gcc/13.2.0/4.1.6/lib:/opt/Software/openmpi/gcc/13.2.0/4.1.6_ucx/lib"

# 2. Setup Apptainer Binds and Paths
export APPTAINER_BIND="/opt/Software:/opt/Software"
export PATH="$HOST_MPI_BIN:$PATH"

# 3. Define and EXPORT MPI/HELICS Environment for the Container
export OPAL_PREFIX="/opt/Software/openmpi/gcc/13.2.0/4.1.6"
export LD_LIBRARY_PATH="/usr/local/helics/lib64:/usr/local/lib:$HOST_MPI_LIBS:$LD_LIBRARY_PATH"
export HELICS_CORE_TYPE="mpi"
export HELICS_CORE_INIT="--broker_rank=0"

# 4. Map these to the Apptainer internal environment
export APPTAINERENV_OPAL_PREFIX="$OPAL_PREFIX"
export APPTAINERENV_LD_LIBRARY_PATH="$LD_LIBRARY_PATH"
export APPTAINERENV_HELICS_CORE_TYPE="$HELICS_CORE_TYPE"
export APPTAINERENV_HELICS_CORE_INIT="$HELICS_CORE_INIT"
export APPTAINERENV_UCX_TLS="self,sm,tcp"
export APPTAINERENV_UCX_POSIX_USE_PROC_LINK=n

# 5. Cleanup
pkill -u $USER -f "helics_broker|gridlabd|gpk_fed" || true
sleep 1

# 6. Unified MPMD Launch
echo "Launching Massive-Scale MPI Simulation..."

# -tag-output identifies which rank is speaking
# -x exports the variables we just defined
mpirun --tag-output \
  -x LD_LIBRARY_PATH \
  -x OPAL_PREFIX \
  -x HELICS_CORE_TYPE \
  -x HELICS_CORE_INIT \
  -mca btl ^openib -mca btl_vader_single_copy_mechanism none \
  -n 1 apptainer exec $SIF helics_broker --coretype=mpi --federates=2 : \
  -n 1 apptainer exec $SIF gridlabd gld_model.glm : \
  -n 4 apptainer exec $SIF stdbuf -oL ./build/gpk_fed