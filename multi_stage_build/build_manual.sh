#!/bin/bash

# Load modules for correct apptainer binary
module purge; module load modules

# Generate random string to prevent folder collisions
export random_string=`tr -dc A-Za-z0-9 < /dev/urandom | head -c 15; echo`

# Define $TMPDIR
export TMPDIR=/dev/shm/${USER}/${random_string}/${stages[i]}
echo "Temporary directory is: " $TMPDIR

# Delete $TMPDIR if it exists
if [ -d "$TMPDIR" ]; then
   chmod u+w -R $TMPDIR # apptainer sets funny folder permission
   command rm -rf $TMPDIR
fi

# Create $TMPDIR
mkdir -p $TMPDIR

# Apptainer build command
apptainer build --fakeroot --tmpdir $TMPDIR build.sif rl8_uncc_sys.def

