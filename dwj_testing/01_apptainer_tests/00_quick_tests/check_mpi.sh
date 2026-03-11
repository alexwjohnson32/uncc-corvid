#!/bin/bash

if [ -z "$1" ]; then
    echo "❌ Usage: $0 <path_to_apptainer_image.sif>"
    exit 1
fi

SIF_FILE=$1

echo "NOTE: THE MPI APPTAINER DOESN'T NEED TO RUN WRITE, IT JUST NEEDS TO HELP BUILD THE APPLICATIONS CORRECTLY"
exit

apptainer exec --writable-tmpfs "$SIF_FILE" bash -s <<'EOF'
    # --- Paths ---
    # Where the binaries live (Install Dir)
    MPI_INSTALL="/usr/local/openmpi-4.1.6"
    # Where the example source code lives (Source Dir)
    MPI_SOURCE="/root/develop/openmpi-4.1.6"
    # Where we will build our test binaries
    TEST_DIR="/root/develop/mpi_diagnostic_suite"
    
    mkdir -p $TEST_DIR

    # 0. the basic troubleshooting
    $MPI_INSTALL/bin/mpirun -np 2 echo "hello"


    # 1. PATH & VERSION CHECK
    echo -e "\n[Step 1/5] Verifying Environment..."
    $MPI_INSTALL/bin/mpirun --version | head -n 1
    
    # 2. BASIC PROCESS LAUNCH (Using installed mpirun)
    #echo -e "\n[Step 2/5] Testing basic 'mpirun' launch..."
    #$MPI_INSTALL/bin/mpirun --allow-run-as-root \
    #    --mca iof_base_nonblocking 1 \
    #    --mca btl tcp,self \
    #    -np 2 hostname || echo "⚠️ Warning: Step 2 had non-zero exit, moving on..."
    #$MPI_INSTALL/bin/mpirun hostname
    #$MPI_INSTALL/bin/mpirun --allow-run-as-root -np 2 echo "hello"
    wait 2

    # 3. COMPILATION (Using installed mpicc to compile source from /root/develop)
    echo -e "\n[Step 3/5] Compiling Native Examples..."
    $MPI_INSTALL/bin/mpicc $MPI_SOURCE/examples/hello_c.c -o $TEST_DIR/hello_diag
    $MPI_INSTALL/bin/mpicc $MPI_SOURCE/examples/ring_c.c -o $TEST_DIR/ring_diag
    
    if [[ -x "$TEST_DIR/hello_diag" ]]; then 
        echo "✅ Compilation successful (Source from $MPI_SOURCE)."
    else 
        echo "❌ ERROR: Could not find hello_c.c in $MPI_SOURCE/examples/"; exit 1; 
    fi

    # 4. MPI INITIALIZATION (Hello World)
    echo -e "\n[Step 4/5] Testing MPI_Init (Hello World)..."
    $MPI_INSTALL/bin/mpirun --allow-run-as-root \
        --mca iof_base_nonblocking 1 \
        --mca btl tcp,self \
        --mca pml ob1 \
        -np 2 $TEST_DIR/hello_diag

    # 5. FULL RING COMMUNICATION (Data Transfer)
    echo -e "\n[Step 5/5] Testing Ring Communication..."
    $MPI_INSTALL/bin/mpirun --allow-run-as-root \
        --mca iof_base_nonblocking 1 \
        --mca btl tcp,self \
        --mca pml ob1 \
        -np 4 $TEST_DIR/ring_diag

    echo -e "\n✨ ALL FIVE STEPS COMPLETE"
    rm -rf $TEST_DIR
EOF