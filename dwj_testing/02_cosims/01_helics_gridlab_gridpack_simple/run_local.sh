#!/bin/bash
SIF=$1

# 1. Kill everything
pkill -u $USER -f helics_broker || true
pkill -u $USER -f gridlabd || true
pkill -u $USER -f gpk_fed || true
fuser -k 23404/tcp 23405/tcp 23500/tcp
sleep 1

# 2. Setup Environment - FORCE GridLAB-D to see these
export APPTAINERENV_HELICS_BROKER="localhost"
export APPTAINERENV_HELICS_PORT="23404"
export APPTAINERENV_HELICS_CORE_TYPE="zmq"
# Ensure libraries are found
export APPTAINERENV_LD_LIBRARY_PATH="/usr/local/helics/lib64:/usr/local/lib:$LD_LIBRARY_PATH"

# 3. Launch Broker manually
echo "Launching Broker manually..."
apptainer exec $SIF helics_broker --coretype=zmq --federates=2 --port=23404 --loglevel=debug &
sleep 2

# 4. Launch Federates
echo "Launching Federates..."
apptainer exec $SIF /opt/myenv/bin/helics run --path=runner.json