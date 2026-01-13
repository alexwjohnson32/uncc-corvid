#!/bin/bash

mkdir -p build/deploy

cd build/deploy

helics run --path runnable_cosim.json
