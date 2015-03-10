gnuplot -p -e "set view map; set pm3d map; set palette defined (1 'black', 2 'blue', 3 'red', 4 'yellow', 5 'white'); splot '$1' with points ps 2 pt 5 palette"
