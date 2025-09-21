#!/bin/bash

./clean_dir.sh

if [ "$#" -eq 1 ]; then
    ./build.sh $1
else
    ./build.sh
fi

