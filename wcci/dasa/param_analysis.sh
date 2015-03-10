#!/bin/bash

iter=0
for evap in 0.1 0.2 0.4; do
    for ants in 5 10 20; do
        for inc in 1.02 1.05 1.10; do
            for dec in 0.90 0.95 0.99; do
                for base in 2 5 10; do
                    iter=$(( ${iter} + 100 ))
                    all="stats/DASA-f101-25-${iter}.stats"
                    rm -f "${all}"
                    score=0
                    score_count=0
                    for run in $(seq 1 10); do
                        IT=$(( ${iter} + ${run} ))
                        stats="stats/DASA-f101-25-${IT}.stats"
                        if [ -f "${stats}" ]; then
                            echo "${stats}" >> ${all}
                            echo -en "\t" >> ${all}
                            grep -F "Best score" ${stats} >> ${all}
                            best_score="$(grep -F "Best score" ${stats} | cut -d'=' -f 2)"
                            score="$( echo "${score} + ${best_score}" | bc )"
                            score_count=$(( ${score_count} + 1 ))
                        fi
                    done
                    if [ ${score_count} -gt 0 ]; then
                        avg_score="$( echo "scale=6; ${score} / ${score_count}" | bc )"
                        echo -e "\nAverage score = ${avg_score}\tfrom ${score_count} runs" >> ${all}
                        echo -e "\n[Params]" >> ${all}
                        echo "Evaporation = ${evap}" >> ${all}
                        echo "Number of ants = ${ants}" >> ${all}
                        echo "Cauchy inc = ${inc}" >> ${all}
                        echo "Caucy dec = ${dec}" >> ${all}
                        echo "Discrete base = ${base}" >> ${all}
                    fi
                done
            done
        done
    done
done

for i in $(seq 100 100 24300);
do
    all="stats/DASA-f101-25-${i}.stats"
    if [ -f "${all}" ]; then
        grep -F "Average score" ${all}
    fi
done
