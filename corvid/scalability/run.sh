#!/bin/bash

mkdir -p build/deploy

cd build

# ./orchestrator/orchestrator_exec
./orchestrator/orchestrator_exec --existing
