# script gnuplot para graficar profiling (desglose de tiempos)

set datafile separator ","
set terminal pdf size 6,4 enhanced font 'Inter,11'

# Margen en los extremos del eje X para evitar que los puntos toquen los bordes
set offsets 0.2, 0.2, 0, 0

# - Desglose del Tiempo en Local Buffers (profiling)
set output 'draws/erosion_local_breakdown.pdf'
set title "Desglose de Tiempos en Erosión Local Buffers (4096x4096)" font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Tiempo de Ejecución (ms)"
#set logscale y
set grid
set key left top
plot "< grep ',dynamic,' ../benchmark_erosion_4096.csv" using 1:9 with linespoints lw 2 title "Asignación de Memoria (Alloc)", \
     "< grep ',dynamic,' ../benchmark_erosion_4096.csv" using 1:10 with linespoints lw 2 title "Simulación de Gotas (Sim)", \
     "< grep ',dynamic,' ../benchmark_erosion_4096.csv" using 1:11 with linespoints lw 2 title "Reducción de Buffers (Reduc)"
