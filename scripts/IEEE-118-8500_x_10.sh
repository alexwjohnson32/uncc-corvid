#!/bin/bash

# Exit if no argument provided
#if [ -z "$1" ]; then
#  echo "Usage: $0 <container.sif>"
#  exit 1
#fi

#SIF_FILE="$1"
#SIF_FILE="/home/shelf1/compile/uncc_dir/uncc_images/rl8_uncc_full_image.sif"
#SIF_FILE="/beegfs/users/djacobson/programs/uncc_electric/installs/20250815_roi-uncc/containers/apptainer/dwj_helics_gridlab_gridpack.sif"
#SIF_FILE="/beegfs/users/djacobson/programs/uncc_electric/installs/20250825_uncc_corvid/multi_stage_build/rl8_uncc_full_image.sif"
#SIF_FILE="/beegfs/users/djacobson/programs/uncc_electric/installs/20250815_roi-uncc/containers/apptainer/dwj_precursor.sif"
SIF_FILE="/beegfs/users/djacobson/programs/uncc_electric/installs/20250917_uncc_corvid/multi_stage_build/rl8_uncc_full_image.sif"

clear

# Purging modules to avoid loaded options that are different
module purge

/opt/Software/bin/apptainer exec $SIF_FILE bash -s <<'EOF'


echo Starting IEEE-118-8500_x_10 Run

export UNCC_ROOT="$(cd "$(pwd)/.." && pwd)"

# Linking HELICS correctly
#export LD_LIBRARY_PATH=/root/develop/helics/build/lib:$LD_LIBRARY_PATH
#export LD_LIBRARY_PATH=/usr/local/helics/lib64/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/root/develop/helics/build/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/lib64/openmpi/bin:$LD_LIBRARY_PATH

# Changing to the correct dir
#cd $UNCC_ROOT/examples/IEEE-118-8500/cosim_118_8500
cd ../examples/glds_10_118_1_bus/
mkdir -p outputs
rm -rf outputs/*

#cd $UNCC_ROOT/examples/
mkdir -p build
rm -rf build/*
cd build

cmake ..
make -j10

cd $UNCC_ROOT/examples/glds_10_118_1_bus/

# Linking HELICS correctly
export LD_LIBRARY_PATH=/root/develop/helics/build/lib:$LD_LIBRARY_PATH

# Running example
helics run --path=ieee8500_10_gpk_1_cosim_runner.json

echo Run Complete
EOF
