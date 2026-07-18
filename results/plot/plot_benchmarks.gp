# script gnuplot para graficar benchmarks

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
set terminal pdf size 6,4 enhanced font 'Inter,11'

# Margen en los extremos del eje X para evitar que los puntos toquen los bordes
set offsets 0.2, 0.2, 0, 0
set grid

# Suffix para títulos
title_suffix = " - " . platform_name

# -------------------------------------------------------------
# 1. RUIDO: SPEEDUP COMPARANDO TAMAÑOS (Planificador: static)
# -------------------------------------------------------------
set output results_dir . '/plot/draws/noise_speedup.pdf'
set title "Speedup de Generación de Ruido (Planificador: Static)" . title_suffix font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Speedup"
set key left top
plot noise_grep('static', 256) using 1:6 with linespoints lw 2 title "256x256", \
     noise_grep('static', 512) using 1:6 with linespoints lw 2 title "512x512", \
     noise_grep('static', 1024) using 1:6 with linespoints lw 2 title "1024x1024", \
     noise_grep('static', 2048) using 1:6 with linespoints lw 2 title "2048x2048", \
     noise_grep('static', 4096) using 1:6 with linespoints lw 2 title "4096x4096"

# -------------------------------------------------------------
# 2. RUIDO: COMPARACIÓN DE PLANIFICADORES (4096x4096)
# -------------------------------------------------------------
set output results_dir . '/plot/draws/noise_speedup_schedulers.pdf'
set title "Ruido 4096x4096: Speedup (Static vs Dynamic)" . title_suffix font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Speedup"
set key left top
plot noise_grep('static', 4096) using 1:6 with linespoints lw 2 title "Static", \
     noise_grep('dynamic', 4096) using 1:6 with linespoints lw 2 title "Dynamic"

# -------------------------------------------------------------
# 3. RUIDO: EFICIENCIA COMPARANDO TAMAÑOS (Planificador: static)
# -------------------------------------------------------------
set output results_dir . '/plot/draws/noise_efficiency.pdf'
set title "Eficiencia de Generación de Ruido (Planificador: Static)" . title_suffix font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Eficiencia (%)"
set key left bottom
plot noise_grep('static', 256) using 1:7 with linespoints lw 2 title "256x256", \
     noise_grep('static', 512) using 1:7 with linespoints lw 2 title "512x512", \
     noise_grep('static', 1024) using 1:7 with linespoints lw 2 title "1024x1024", \
     noise_grep('static', 2048) using 1:7 with linespoints lw 2 title "2048x2048", \
     noise_grep('static', 4096) using 1:7 with linespoints lw 2 title "4096x4096"

# -------------------------------------------------------------
# 4. EROSIÓN: SPEEDUP COMPARANDO PLANIFICADORES (4096x4096)
# -------------------------------------------------------------
set output results_dir . '/plot/draws/erosion_speedup.pdf'
set title "Speedup de Erosión Hidráulica (4096x4096)" . title_suffix font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Speedup (vs Secuencial)"
set key left top
plot erosion_grep('static', 4096) using 1:7 with linespoints lw 2 title "Atómico (Static)", \
     erosion_grep('dynamic', 4096) using 1:7 with linespoints lw 2 title "Atómico (Dynamic)", \
     erosion_grep('static', 4096) using 1:8 with linespoints lw 2 title "Local Buffers (Static)", \
     erosion_grep('dynamic', 4096) using 1:8 with linespoints lw 2 title "Local Buffers (Dynamic)"

# -------------------------------------------------------------
# 5. EROSIÓN: EFICIENCIA COMPARANDO PLANIFICADORES (4096x4096)
# -------------------------------------------------------------
set output results_dir . '/plot/draws/erosion_efficiency.pdf'
set title "Eficiencia de Erosión Hidráulica (4096x4096)" . title_suffix font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Eficiencia (%)"
set key left bottom
plot erosion_grep('static', 4096) using 1:(($7 / $1) * 100.0) with linespoints lw 2 title "Atómico (Static)", \
     erosion_grep('dynamic', 4096) using 1:(($7 / $1) * 100.0) with linespoints lw 2 title "Atómico (Dynamic)", \
     erosion_grep('static', 4096) using 1:(($8 / $1) * 100.0) with linespoints lw 2 title "Local Buffers (Static)", \
     erosion_grep('dynamic', 4096) using 1:(($8 / $1) * 100.0) with linespoints lw 2 title "Local Buffers (Dynamic)"

# -------------------------------------------------------------
# 6. EROSIÓN: COMPARATIVA DE TAMAÑOS (Local Buffers, Planificador: dynamic)
# -------------------------------------------------------------
set output results_dir . '/plot/draws/erosion_speedup_sizes.pdf'
set title "Erosión Local Buffers: Escalabilidad por Tamaño (Dynamic)" . title_suffix font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Speedup"
set key left top
plot erosion_grep('dynamic', 256) using 1:8 with linespoints lw 2 title "256x256", \
     erosion_grep('dynamic', 512) using 1:8 with linespoints lw 2 title "512x512", \
     erosion_grep('dynamic', 1024) using 1:8 with linespoints lw 2 title "1024x1024", \
     erosion_grep('dynamic', 2048) using 1:8 with linespoints lw 2 title "2048x2048", \
     erosion_grep('dynamic', 4096) using 1:8 with linespoints lw 2 title "4096x4096"

# -------------------------------------------------------------
# 7. EROSIÓN: TIEMPO DE EJECUCIÓN COMPARACIÓN (4096x4096, Planificador: dynamic)
# -------------------------------------------------------------
set output results_dir . '/plot/draws/erosion_time.pdf'
set title "Tiempo de Ejecución de Erosión Hidráulica (4096x4096, Dynamic)" . title_suffix font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Tiempo de Ejecución (ms)"
set logscale y
plot erosion_grep('dynamic', 4096) using 1:4 with linespoints lw 2 title "Secuencial (Línea Base)", \
     erosion_grep('dynamic', 4096) using 1:5 with linespoints lw 2 title "Paralelo (Atómico)", \
     erosion_grep('dynamic', 4096) using 1:6 with linespoints lw 2 title "Paralelo (Buffers Locales)"
unset logscale y

# -------------------------------------------------------------
# 8. EROSIÓN: DESGLOSE DE TIEMPOS EN LOCAL BUFFERS (4096x4096, Planificador: dynamic)
# -------------------------------------------------------------
set output results_dir . '/plot/draws/erosion_local_breakdown.pdf'
set title "Desglose de Tiempos en Erosión Local Buffers (4096x4096, Dynamic)" . title_suffix font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Tiempo de Ejecución (ms)"
plot erosion_grep('dynamic', 4096) using 1:9 with linespoints lw 2 title "Asignación de Memoria (Alloc)", \
     erosion_grep('dynamic', 4096) using 1:10 with linespoints lw 2 title "Simulación de Gotas (Sim)", \
     erosion_grep('dynamic', 4096) using 1:11 with linespoints lw 2 title "Reducción de Buffers (Reduc)"
