#!/bin/bash

if [ $# -gt 1 ]; then
    MOCRETA="$1"
    #
    # Prepare data for the DASA algorithm
    #
    OUT_FILES=$(./setup_dasa.py ${MOCRETA} $2 $3)

    #
    # Get the number of cells (parameters) being optimized
    #
    PARAMS_FILE=$(echo "${OUT_FILES}" | grep 'DASA params' | egrep -o '\[.+\]' | tr -d '[' | tr -d ']')
    PARAM_COUNT=$(head -n1 ${PARAMS_FILE} | cut -d' ' -f2)
    
    #
    # Run the DASA algorithm
    #
    DASA="./dasa -f 101 -D ${PARAM_COUNT} -cells ${MOCRETA}"
    echo -e "Executing\t${DASA}"
    ${DASA}
else
    ./setup_dasa.py
fi
