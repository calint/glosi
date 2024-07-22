#!/usr/bin/gnuplot

set terminal png
set output "update.png"
set key autotitle columnhead
set xlabel "time (ms)"
plot for[col=5:5] "metrics.txt" using 1:col title columnheader(col) with lines
