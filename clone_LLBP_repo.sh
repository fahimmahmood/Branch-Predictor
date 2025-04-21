#!/bin/bash

REQUIRED_DIR="Branch-Predictor"
CURRENT_DIR=$(basename "$PWD")

if [ "$CURRENT_DIR" != "$REQUIRED_DIR" ]; then
    printf "Please run this script from within the '$REQUIRED_DIR' directory.\n"
    exit 1
fi

# Clone the repo @ https://github.com/dhschall/LLBP
git clone https://github.com/dhschall/LLBP.git

printf "LLBP cloned! Make the simulator by runnning:\n\tcd src\n\tmake\n"