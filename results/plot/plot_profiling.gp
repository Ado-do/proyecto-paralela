# script gnuplot para graficar perfilamiento (desglose de tiempos)

set datafile separator ","
set terminal pdf size 6,4 enhanced font 'Inter,11'

# 1. Desglose del Tiempo en Local Buffers (Perfilamiento)
set output 'draws/erosion_local_breakdown.pdf'
set title "Desglose de Tiempos en Erosion Local Buffers (4096x4096)" font 'Inter-Bold,14'
set xlabel "Numero de Hilos"
set ylabel "Tiempo de Ejecucion (ms)"
set grid
set key left top
plot '../benchmark_erosion_4096.csv' using 1:8 with linespoints lw 2 title "Asignacion de Memoria (Alloc)", \
     '../benchmark_erosion_4096.csv' using 1:9 with linespoints lw 2 title "Simulacion de Gotas (Sim)", \
     '../benchmark_erosion_4096.csv' using 1:10 with linespoints lw 2 title "Reduccion de Buffers (Reduc)"
