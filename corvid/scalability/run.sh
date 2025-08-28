#!/bin/bash

mkdir -p build/deploy

cd build

./orchestrator/orchestrator_exec "$@"
