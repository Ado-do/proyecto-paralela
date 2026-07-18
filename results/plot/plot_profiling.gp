# script gnuplot para graficar profiling (desglose de tiempos)

# Configuración de argumentos dinámicos
if (ARGC >= 1) {
    results_dir = ARG1
} else {
    results_dir = 'results'
}

if (ARGC >= 2) {
    platform_name = ARG2
} else {
    platform_name = 'Laptop'
}

set datafile separator ","
set terminal pdf size 6,4 enhanced font 'Inter,11'

# Margen en los extremos del eje X para evitar que los puntos toquen los bordes
set offsets 0.2, 0.2, 0, 0

# Suffix para títulos
title_suffix = " - " . platform_name

# - Desglose del Tiempo en Local Buffers (profiling)
set output results_dir . '/plot/draws/erosion_local_breakdown.pdf'
set title "Desglose de Tiempos en Erosión Local Buffers (4096x4096)" . title_suffix font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Tiempo de Ejecución (ms)"
#set logscale y
set grid
set key left top
plot sprintf("< grep ',dynamic,' %s/benchmark_erosion_4096.csv", results_dir) using 1:9 with linespoints lw 2 title "Asignación de Memoria (Alloc)", \
     sprintf("< grep ',dynamic,' %s/benchmark_erosion_4096.csv", results_dir) using 1:10 with linespoints lw 2 title "Simulación de Gotas (Sim)", \
     sprintf("< grep ',dynamic,' %s/benchmark_erosion_4096.csv", results_dir) using 1:11 with linespoints lw 2 title "Reducción de Buffers (Reduc)"
