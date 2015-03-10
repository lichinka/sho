#!/bin/bash

for temp in 20 50 100 200 400 800 1500; do
    for run in $(seq 1 10); do
        OUTPUT="run-temp_${temp}-${run}.txt"
        ./start_optim.sh ../../data/cell_powers.txt ../../data/dl_path_loss/ ../../data/ul_path_loss ../../data/dl_path_loss/best_server.GRD 370 661 ${temp} 100000 > ${OUTPUT} 2>&1
    done
done

