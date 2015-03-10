for i in $(seq 1 50);
do
    nice -n -3 ./start_optim.sh ../../data/cell_powers.txt params/f101.params
done

