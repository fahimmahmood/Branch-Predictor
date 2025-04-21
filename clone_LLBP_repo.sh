#!/bin/bash

REQUIRED_DIR="Branch-Predictor"
CURRENT_DIR=$(basename "$PWD")

if [ "$CURRENT_DIR" != "$REQUIRED_DIR" ]; then
    echo "Please run this script from within the '$REQUIRED_DIR' directory."
    exit 1
fi

# Clone the repo @ https://github.com/dhschall/LLBP
git clone https://github.com/dhschall/LLBP.git