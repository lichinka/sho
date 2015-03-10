#!/bin/bash

iter=0
for evap in 0.1 0.2 0.4; do
    for ants in 5 10 20; do
        for inc in 1.02 1.05 1.10; do
            for dec in 0.90 0.95 0.99; do
                for base in 2 5 10; do
                    iter=$(( ${iter} + 100 ))
                    for run in $(seq 1 10); do
                        IT=$(( ${iter} + ${run} ))
                        cat DASA-template.ini | sed -e "s/@evap/${evap}/g" \
                                              | sed -e "s/@ants/${ants}/g" \
                                              | sed -e "s/@inc/${inc}/g" \
                                              | sed -e "s/@dec/${dec}/g" \
                                              | sed -e "s/@base/${base}/g" \
                                              | sed -e "s/@it/${IT}/g" \
                                              > DASA.ini
                        ./start_optim.sh ../../data/cell_powers.txt params/f101.params
                    done
                done
            done
        done
    done
done

