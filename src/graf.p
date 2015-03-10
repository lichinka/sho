#set terminal postscript eps enhanced size 14cm, 10cm
#set terminal svg enhanced dynamic font "Vera,22" linewidth 1
set terminal wxt enhanced font "Vera,20"
set style line 1 linecolor rgb "red" linewidth 1
set style line 2 linecolor rgb "black" linewidth 1
set autoscale
set key right top Left
set xlabel "Število iteracij"
set ylabel "Sila v grafu {/CMMI10 \107}"
#set format y "10^{%01T}"
#set logscale y
plot "/tmp/solution.txt" using 2 title '|F_{/CMMI10 \107}|' with lines ls 2
