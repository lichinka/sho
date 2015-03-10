#!/bin/bash

for np in 50; do
    #for cr in 0.7 0.8 0.9; do
    for cr in 0.8 0.9; do
        for f in 0.2 0.5 0.7 0.9; do
            GMAX=$(( 100000 / ${np} ))
            for run in $(seq 1 5); do
                OUTPUT="run-np_${np}-cr_${cr}-f_${f}-gmax_${GMAX}-${run}.txt"
                ./de -N ${np} -C ${cr} -F ${f} -G ${GMAX} > ${OUTPUT} 2>&1
            done
        done
    done
done

