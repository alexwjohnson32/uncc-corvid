#!/bin/bash

SIF_FILE=$1
BUILD_DIR=$2

echo "----GRIDPACK: check installed"
apptainer exec --writable-tmpfs $SIF_FILE bash -s <<'EOF'
    #cd $2
    #ctest
    cd /root/develop/GridPACK/src/build/
    #cd /build/GridPACK/src/build/
    export OMPI_MCA_btl=self,vader,tcp
    export OMPI_MCA_pml=ob1
    export OMPI_MCA_rmaps_base_oversubscribe=1
    #ctest -I 1,50 -R "_serial$" -j 1
    ctest -R "_serial$" -j 1
EOF 

#    cd /root/develop/GridPACK/src/build/
#    ctest -R hello_world
#    ctest -R greetings_serial
#    ctest -R greetings_parallel
#    ctest -R task_test_serial
#    ctest -R task_test_parallel
#    ctest -R mpi_test_serial
#    ctest -R mpi_test_parallel
#    ctest -R shuffle_serial
#    ctest -R shuffle_parallel
#    ctest -R hash_test_serial
#    ctest -R hash_test_parallel