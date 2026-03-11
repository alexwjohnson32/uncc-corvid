

#--build
SIF_FILE=$1

/opt/Software/bin/apptainer exec $SIF_FILE bash -s <<'EOF'
rm -rf build/
mkdir build && cd build
#cmake .. -DGridPACK_DIR=/usr/local/gridpack/lib/cmake/gridpack
cmake .. -DGridPACK_DIR=/usr/local/GridPACK/
make

#--run
#mpiexec -np 4 ./hw_hello
EOF