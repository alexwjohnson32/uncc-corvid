#!/bin/bash

# clerical tracking information
#SBATCH --job-name="z005--spaced_plates"
#SBATCH --comment="4.201_sph_decoup_8t1"
#SBATCH --account="1771-010"
#SBATCH --time=4:00
#SBATCH --partition=37
#SBATCH --nodes=2
module purge
module load modules python/3.11 openmpi/4.1.6

# --- CONFIGURATION ---
TOTAL_ROWS=400
NUM_WORKERS=30
ROWS_PER_WORKER=$((TOTAL_ROWS / NUM_WORKERS))
ROW_LEN=400
DENSITY=0.25
STEPS=2000
CONTAINER=$1
HELICS_PORT=23404
# ---------------------

rm -rf *.txt
rm -rf *.log
rm -rf *.out
rm -rf *.conf
CONF_FILE="slurm_gol_${SLURM_JOB_ID}.conf"
rm -f temp_worker_*_row_*.csv full_history.txt $CONF_FILE

# DYNAMIC IP DISCOVERY (Using srun for internal node discovery)
BROKER_IP=$(srun --ntasks=1 --nodes=1 --relative=0 hostname -I | awk '{print $1}')
echo "--- Slurm System Status ---"
echo "Broker IP: $BROKER_IP"

# BUILD CONFIG
# Task 0: Broker
echo "0 apptainer exec --bind $(pwd) --pwd $(pwd) $CONTAINER helics_broker --coretype=zmq --federates=$NUM_WORKERS --localinterface=$BROKER_IP --port=$HELICS_PORT" > $CONF_FILE

# Tasks 1 to NUM_WORKERS: Workers
for i in $(seq 1 $NUM_WORKERS); do
    W_ID=$((i-1))
    [ $i -eq $NUM_WORKERS ] && ACTUAL_ROWS=$(( TOTAL_ROWS - (W_ID * ROWS_PER_WORKER) )) || ACTUAL_ROWS=$ROWS_PER_WORKER
    echo "$i apptainer exec --bind $(pwd) --pwd $(pwd) $CONTAINER python3.11 gol_federate.py $W_ID $NUM_WORKERS $ACTUAL_ROWS $ROW_LEN $DENSITY $BROKER_IP $STEPS" >> $CONF_FILE
done

# 1. LAUNCH THE ENTIRE ALLOCATION
# Instead of backgrounding the broker, we launch everything at once.
# HELICS Federates are designed to wait for the broker to appear.
echo "Launching Broker and Workers via Multi-Prog..."

# 2. FIREWALL CHECK (Python Version)
# We run this in the background to monitor the broker's startup
(
    sleep 5
    echo "Checking connectivity to $BROKER_IP:$HELICS_PORT via Python..."
    python3 -c "import socket; s = socket.socket(); s.settimeout(5); exit(0 if s.connect_ex(('$BROKER_IP', $HELICS_PORT)) == 0 else 1)"
    if [ $? -eq 0 ]; then
        echo "Network path is OPEN."
    else
        echo "WARNING: Path to Broker appears blocked or Broker failed to start."
    fi
) &

# 3. START THE SIMULATION
srun --multi-prog $CONF_FILE

echo "Simulation finished. Aggregating..."
python3 aggregate_gol.py

# Cleanup
#rm -f $CONF_FILE
