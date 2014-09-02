set terminal png
set output "abstract.png"
set style data lines

set xlabel "Generations"
set ylabel "Average Evolvability"
set key bottom right
plot for [i=3:6] "abstract-model.dat" u 2:i w l title columnheader(i)

#plot "abstract-model.dat" u 2:3,'' u 2:4, '' u 2:5, '' u 2:6
