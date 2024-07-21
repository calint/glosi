#!/usr/bin/gnuplot

set terminal png
set output "fps.png"
set key autotitle columnhead
set xlabel "time (ms)"
plot for[col=3:3] "metrics.txt" using 1:col title columnheader(col) with lines
