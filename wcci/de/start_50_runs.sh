#!/bin/bash

for np in 100; do
    for cr in 0.8; do
        for f in 0.5; do
            GMAX=$(( 100000 / ${np} ))
            for run in 2 3 7 8 16 17 20 24 25 26 30 49 50; do
                OUTPUT="run-np_${np}-cr_${cr}-f_${f}-gmax_${GMAX}-${run}.txt"
                nice -n -3 ./de -N ${np} -C ${cr} -F ${f} -G ${GMAX} > ${OUTPUT} 2>&1
                sleep 120
            done
        done
    done
done

