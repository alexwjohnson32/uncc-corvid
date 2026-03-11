#!/bin/bash

SIF_FILE=$1

echo "----HELICS: check helics is installed"

apptainer exec $SIF_FILE helics_broker --version

#echo "----HELICS: confirm custom build is being used"
#module load python/3.11
#apptainer exec $SIF_FILE /bin/bash -c "source /opt/myenv/bin/activate && python3 check_helics_source.py"

#echo "----HELICS: path check"
#module load python/3.11
#apptainer exec $SIF_FILE /bin/bash -c "source /opt/myenv/bin/activate && python3 check_helics_source.py"

echo "----HELICS: check core types"

apptainer exec $SIF_FILE python3.11 -c "import helics as h; print('MPI:', h.helicsIsCoreTypeAvailable('mpi')); print('ZMQ:', h.helicsIsCoreTypeAvailable('zmq'))"