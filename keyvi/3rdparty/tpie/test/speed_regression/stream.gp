set out "stream.ps"
set terminal postscript

set autoscale
set title "Speed of operations trough time"
set xlabel "Revision"
set ylabel "Time"
plot "stream.dat" using 1:2 title "writeitem" with linespoint, \
     "stream.dat" using 1:3 title "readitem" with linespoint, \
     "stream.dat" using 1:4 title "writearray" with linespoint, \
     "stream.dat" using 1:5 title "readarray" with linespoint

