#!/bin/bash

module purge
module load modules python/3.11 openmpi/4.1.6

# --- LOCAL CONFIGURATION (Emulating SBATCH) ---
TOTAL_ROWS=400
NUM_WORKERS=30
ROWS_PER_WORKER=$((TOTAL_ROWS / NUM_WORKERS))
ROW_LEN=400
DENSITY=0.25
STEPS=2000
CONTAINER=$1 # Expects the .sif or image path as the first argument
HELICS_PORT=23404

# Check if container argument is provided
if [ -z "$CONTAINER" ]; then
    echo "Usage: ./run_local.sh <path_to_container>"
    exit 1
fi

rm -rf *.txt
rm -rf *.log

# Use localhost for local execution
BROKER_IP="127.0.0.1"
# Use PID or timestamp since we don't have a SLURM_JOB_ID
LOCAL_ID=$$
APP_FILE="local_mpirun_${LOCAL_ID}.app"

# Clean up previous runs
rm -f temp_worker_*_row_*.csv full_history.txt "$APP_FILE"

echo "--- Local System Status ---"
echo "Broker IP: $BROKER_IP (Localhost)"
echo "Generating MPI Appfile: $APP_FILE"

# BUILD MPI APPFILE
# Syntax for --app: -np [count] executable args
# Task 0: The Broker (1 instance)
echo "-np 1 apptainer exec --bind $(pwd) --pwd $(pwd) $CONTAINER helics_broker --coretype=zmq --federates=$NUM_WORKERS --localinterface=$BROKER_IP --port=$HELICS_PORT" > "$APP_FILE"

# Tasks 1 to NUM_WORKERS: The Workers
for i in $(seq 1 $NUM_WORKERS); do
    W_ID=$((i-1))
    # Calculate rows for the last worker
    if [ $i -eq $NUM_WORKERS ]; then
        ACTUAL_ROWS=$(( TOTAL_ROWS - (W_ID * ROWS_PER_WORKER) ))
    else
        ACTUAL_ROWS=$ROWS_PER_WORKER
    fi
    
    # Append to appfile (1 instance per worker)
    echo "-np 1 apptainer exec --bind $(pwd) --pwd $(pwd) $CONTAINER python3.11 gol_federate.py $W_ID $NUM_WORKERS $ACTUAL_ROWS $ROW_LEN $DENSITY $BROKER_IP $STEPS" >> "$APP_FILE"
done

echo "Launching Broker and $NUM_WORKERS Workers via mpirun..."

# --- MONITORING (Background) ---
(
    sleep 3
    echo "Checking connectivity to $BROKER_IP:$HELICS_PORT..."
    # Check if port is open locally
    nc -z "$BROKER_IP" "$HELICS_PORT" > /dev/null 2>&1
    if [ $? -eq 0 ]; then
        echo "Network path to HELICS Broker is OPEN."
    else
        echo "WARNING: Local Broker port appears closed. Check container logs."
    fi
) &

# --- START THE SIMULATION ---
# --oversubscribe is often needed for local runs if NUM_WORKERS > physical CPU cores
mpirun --oversubscribe --app "$APP_FILE"

echo "Simulation finished. Aggregating..."
python3 aggregate_gol.py

# Cleanup the temporary appfile
rm -f "$APP_FILE"