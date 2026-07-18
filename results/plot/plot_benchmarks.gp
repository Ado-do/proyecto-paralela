# script gnuplot para graficar benchmarks en tableros consolidados (dashboards)

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

# Funciones auxiliares para construir rutas e instrucciones de grep
noise_csv(sz) = sprintf("%s/benchmark_noise_%d.csv", results_dir, sz)
erosion_csv(sz) = sprintf("%s/benchmark_erosion_%d.csv", results_dir, sz)

noise_grep(sched, sz) = sprintf("< grep ',%s,' %s", sched, noise_csv(sz))
erosion_grep(sched, sz) = sprintf("< grep ',%s,' %s", sched, erosion_csv(sz))

set datafile separator ","

# Margen en los extremos del eje X para evitar que los puntos toquen los bordes
set offsets 0.2, 0.2, 0, 0
set grid

# Suffix para títulos
title_suffix = " - " . platform_name

# =============================================================================
# 1. TABLERO DE RENDIMIENTO: RUIDO
# =============================================================================
set terminal pdf size 10,7.5 enhanced font 'Inter,11'
set output results_dir . '/plot/draws/noise_dashboard.pdf'

set multiplot layout 2,2 rowsfirst title "Tablero de Rendimiento: Generación de Ruido" . title_suffix font 'Inter-Bold,16'

# Subplot 1: Speedup (Static, comparando tamaños)
set title "Speedup de Ruido (Planificador: Static)" font 'Inter-Bold,12'
set xlabel "Cantidad de Hilos"
set ylabel "Speedup"
set key left top
plot noise_grep('static', 256) using 1:6 with linespoints lw 2 title "256x256", \
     noise_grep('static', 512) using 1:6 with linespoints lw 2 title "512x512", \
     noise_grep('static', 1024) using 1:6 with linespoints lw 2 title "1024x1024", \
     noise_grep('static', 2048) using 1:6 with linespoints lw 2 title "2048x2048", \
     noise_grep('static', 4096) using 1:6 with linespoints lw 2 title "4096x4096"

# Subplot 2: Eficiencia (Static, comparando tamaños)
set title "Eficiencia de Ruido (Planificador: Static)" font 'Inter-Bold,12'
set xlabel "Cantidad de Hilos"
set ylabel "Eficiencia (%)"
set key left bottom
plot noise_grep('static', 256) using 1:7 with linespoints lw 2 title "256x256", \
     noise_grep('static', 512) using 1:7 with linespoints lw 2 title "512x512", \
     noise_grep('static', 1024) using 1:7 with linespoints lw 2 title "1024x1024", \
     noise_grep('static', 2048) using 1:7 with linespoints lw 2 title "2048x2048", \
     noise_grep('static', 4096) using 1:7 with linespoints lw 2 title "4096x4096"

# Subplot 3: Comparación de Planificadores (4096x4096)
set title "Ruido 4096x4096: Static vs Dynamic" font 'Inter-Bold,12'
set xlabel "Cantidad de Hilos"
set ylabel "Speedup"
set key left top
plot noise_grep('static', 4096) using 1:6 with linespoints lw 2 title "Static", \
     noise_grep('dynamic', 4096) using 1:6 with linespoints lw 2 title "Dynamic"

# Subplot 4: Tiempo absoluto de ejecución (Static, comparando tamaños)
set title "Tiempo de Ejecución de Ruido (Static)" font 'Inter-Bold,12'
set xlabel "Cantidad de Hilos"
set ylabel "Tiempo de Ejecución (ms)"
set logscale y
set key right top
plot noise_grep('static', 256) using 1:5 with linespoints lw 2 title "256x256", \
     noise_grep('static', 512) using 1:5 with linespoints lw 2 title "512x512", \
     noise_grep('static', 1024) using 1:5 with linespoints lw 2 title "1024x1024", \
     noise_grep('static', 2048) using 1:5 with linespoints lw 2 title "2048x2048", \
     noise_grep('static', 4096) using 1:5 with linespoints lw 2 title "4096x4096"
unset logscale y

unset multiplot

# =============================================================================
# 2. TABLERO DE RENDIMIENTO: EROSIÓN HIDRÁULICA
# =============================================================================
set output results_dir . '/plot/draws/erosion_dashboard.pdf'

set multiplot layout 2,2 rowsfirst title "Tablero de Rendimiento: Erosión Hidráulica" . title_suffix font 'Inter-Bold,16'

# Subplot 1: Speedup Comparación Planificadores y Modos (4096x4096)
set title "Speedup de Erosión (4096x4096)" font 'Inter-Bold,12'
set xlabel "Cantidad de Hilos"
set ylabel "Speedup (vs Secuencial)"
set key left top
plot erosion_grep('static', 4096) using 1:7 with linespoints lw 2 title "Atómico (Static)", \
     erosion_grep('dynamic', 4096) using 1:7 with linespoints lw 2 title "Atómico (Dynamic)", \
     erosion_grep('static', 4096) using 1:8 with linespoints lw 2 title "Local Buffers (Static)", \
     erosion_grep('dynamic', 4096) using 1:8 with linespoints lw 2 title "Local Buffers (Dynamic)"

# Subplot 2: Eficiencia Comparación Planificadores y Modos (4096x4096)
set title "Eficiencia de Erosión (4096x4096)" font 'Inter-Bold,12'
set xlabel "Cantidad de Hilos"
set ylabel "Eficiencia (%)"
set key left bottom
plot erosion_grep('static', 4096) using 1:(($7 / $1) * 100.0) with linespoints lw 2 title "Atómico (Static)", \
     erosion_grep('dynamic', 4096) using 1:(($7 / $1) * 100.0) with linespoints lw 2 title "Atómico (Dynamic)", \
     erosion_grep('static', 4096) using 1:(($8 / $1) * 100.0) with linespoints lw 2 title "Local Buffers (Static)", \
     erosion_grep('dynamic', 4096) using 1:(($8 / $1) * 100.0) with linespoints lw 2 title "Local Buffers (Dynamic)"

# Subplot 3: Escalabilidad por tamaño (Local Buffers, Dynamic)
set title "Erosión Local Buffers: Escalabilidad por Tamaño (Dynamic)" font 'Inter-Bold,12'
set xlabel "Cantidad de Hilos"
set ylabel "Speedup"
set key left top
plot erosion_grep('dynamic', 256) using 1:8 with linespoints lw 2 title "256x256", \
     erosion_grep('dynamic', 512) using 1:8 with linespoints lw 2 title "512x512", \
     erosion_grep('dynamic', 1024) using 1:8 with linespoints lw 2 title "1024x1024", \
     erosion_grep('dynamic', 2048) using 1:8 with linespoints lw 2 title "2048x2048", \
     erosion_grep('dynamic', 4096) using 1:8 with linespoints lw 2 title "4096x4096"

# Subplot 4: Tiempos de ejecución absolutos (4096x4096, Dynamic)
set title "Tiempo de Ejecución de Erosión (4096x4096, Dynamic)" font 'Inter-Bold,12'
set xlabel "Cantidad de Hilos"
set ylabel "Tiempo de Ejecución (ms)"
set logscale y
set key right top
plot erosion_grep('dynamic', 4096) using 1:4 with linespoints lw 2 title "Secuencial (Base)", \
     erosion_grep('dynamic', 4096) using 1:5 with linespoints lw 2 title "Paralelo (Atómico)", \
     erosion_grep('dynamic', 4096) using 1:6 with linespoints lw 2 title "Paralelo (Local Buffers)"
unset logscale y

unset multiplot
