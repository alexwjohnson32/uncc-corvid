#!/bin/bash

SIF_FILE=$1

echo "----HELICS CHECK"
./check_helics.sh $SIF_FILE 
echo "\n\n"

echo "----GRIDLAB CHECK"
./check_gridlab.sh $SIF_FILE
echo "\n\n"

echo "----GRIDPACK CHECK"
./check_gridpack.sh $SIF_FILE
echo "\n\n"