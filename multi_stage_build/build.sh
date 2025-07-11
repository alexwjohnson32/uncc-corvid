#!/bin/bash

# Load modules for correct apptainer binary
module purge; module load modules

# Generate random string to prevent folder collisions
export random_string=`tr -dc A-Za-z0-9 < /dev/urandom | head -c 15; echo`

if [ "$#" -ne 1 ]; then
   echo "Usage $0 <build_option>"
   echo ""
   echo "Available build options are:"
   echo "'sys'              - rebuilds system and all dependencies (everything)"
   echo "                     + Use this option for an initial build as it will"
   echo "                     + generate the required *.sif files for subsquent"
   echo "                     + builds."
   echo "'helics'           - rebuilds helics and all dependencies"
   echo "'gridlab-gridpack' - builds Gridlab-D then GridPACK only"
   echo "'gridpack-gridlab' - builds GridPACK then Gridlab-D only"
   echo "'gridlab'          - builds Gridlab-D only"
   echo "'gridpack'         - builds GridPACK only"
   exit 1
fi

if [ $1 == "sys" ]; then
   declare -i start=0
   declare -a stages=("sys" "helics" "gridlab" "gridpack")
elif [ $1 == "helics" ]; then
   declare -i start=1
   declare -a stages=("sys" "helics" "gridlab" "gridpack")
elif [ $1 == "gridlab-gridpack" ]; then
   declare -i start=2
   declare -a stages=("sys" "helics" "gridlab" "gridpack")
elif [ $1 == "gridpack-gridlab" ]; then
   declare -i start=2
   declare -a stages=("sys" "helics" "gridpack" "gridlab")
elif [ $1 == "gridlab" ]; then
   declare -i start=3
   declare -a stages=("sys" "helics" "gridpack" "gridlab")
elif [ $1 == "gridpack" ]; then
   declare -i start=3
   declare -a stages=("sys" "helics" "gridlab" "gridpack")
else
   echo "Invalid option"
   echo ""
   echo "Available build options are:"
   echo "'sys'              - rebuilds system and all dependencies (everything)"
   echo "'helics'           - rebuilds helics and all dependencies (except sys)"
   echo "'gridlab-gridpack' - rebuilds Gridlab-D then GridPACK"
   echo "'gridpack-gridlab' - rebuilds GridPACK then Gridlab-D"
   echo "'gridlab'          - rebuilds Gridlab-D only"
   echo "'gridpack'         - rebuilds GridPACK only"
   exit 1
fi

# Check if necessary *.sif files exist
if [ $start -eq 0 ]; then
   rm rl8_uncc_${stages[0]}.sif
   rm rl8_uncc_${stages[0]}_${stages[1]}.sif
   rm rl8_uncc_${stages[0]}_${stages[1]}_${stages[2]}.sif
   rm rl8_uncc_${stages[0]}_${stages[1]}_${stages[2]}_${stages[3]}.sif
elif [ $start -eq 1 ]; then
   if [ ! -f rl8_uncc_${stages[0]}.sif ]; then
      echo rl8_uncc_${stages[0]}.sif does not exist.
      echo "You need to run with the " ${stages[0]} " build option."
      exit 1
   fi
   rm rl8_uncc_${stages[0]}_${stages[1]}.sif
   rm rl8_uncc_${stages[0]}_${stages[1]}_${stages[2]}.sif
   rm rl8_uncc_${stages[0]}_${stages[1]}_${stages[2]}_${stages[3]}.sif
elif [ $start -eq 2 ]; then
   if [ ! -f rl8_uncc_${stages[0]}_${stages[1]}.sif ]; then
      echo rl8_uncc_${stages[0]}_${stages[1]}.sif does not exist.
      echo "You need to run with the " ${stages[1]} " build option."
      exit 1
   fi
   rm rl8_uncc_${stages[0]}_${stages[1]}_${stages[2]}.sif
   rm rl8_uncc_${stages[0]}_${stages[1]}_${stages[2]}_${stages[3]}.sif
elif [ $start -eq 3 ]; then
   if [ ! -f rl8_uncc_${stages[0]}_${stages[1]}_${stages[2]}.sif ]; then
      echo rl8_uncc_${stages[0]}_${stages[1]}_${stages[2]}.sif does not exist.
      echo "You need to run with the " ${stages[2]}-${stages[3]} "build option."
      exit 1
   fi
   rm rl8_uncc_${stages[0]}_${stages[1]}_${stages[2]}_${stages[3]}.sif
fi

for i in $(seq $start 3);
do
   echo ""
   echo "Building Stage "${stages[i]}

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

   # Define apptainer sif and def file names
   if [ $i -eq 0 ]; then
      export apptainer_sif=rl8_uncc_${stages[0]}.sif
      export apptainer_def=rl8_uncc_${stages[0]}.def
   elif [ $i -eq 1 ]; then
      export apptainer_sif=rl8_uncc_${stages[0]}_${stages[1]}.sif
      export apptainer_def=rl8_uncc_${stages[0]}_${stages[1]}.def
   elif [ $i -eq 2 ]; then
      export apptainer_sif=rl8_uncc_${stages[0]}_${stages[1]}_${stages[2]}.sif
      export apptainer_def=rl8_uncc_${stages[0]}_${stages[1]}_${stages[2]}.def
   elif [ $i -eq 3 ]; then
      export apptainer_sif=rl8_uncc_${stages[0]}_${stages[1]}_${stages[2]}_${stages[3]}.sif
      export apptainer_def=rl8_uncc_${stages[0]}_${stages[1]}_${stages[2]}_${stages[3]}.def
   fi

   # Apptainer build command
   apptainer build --fakeroot --tmpdir $TMPDIR ${apptainer_sif} ${apptainer_def}

   echo "Finished Building Stage "$i
   echo ""
done

cp rl8_uncc_${stages[0]}_${stages[1]}_${stages[2]}_${stages[3]}.sif rl8_uncc_full_image.sif