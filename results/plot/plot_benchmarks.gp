# script gnuplot para graficar benchmarks

set datafile separator ","
set terminal pdf size 6,4 enhanced font 'Inter,11'

# Función para filtrar datos por planificador (columna 3)
filter(sched, col) = (strcol(3) eq sched ? column(col) : 1/0)
eff(sched, col_speed) = (strcol(3) eq sched ? (column(col_speed) / column(1)) * 100.0 : 1/0)

# Margen en los extremos del eje X para evitar que los puntos toquen los bordes
set offsets 0.2, 0.2, 0, 0
set grid

# -------------------------------------------------------------
# 1. RUIDO: SPEEDUP COMPARANDO TAMAÑOS (Planificador: static)
# -------------------------------------------------------------
set output 'draws/noise_speedup.pdf'
set title "Speedup de Generación de Ruido (Planificador: Static)" font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Speedup"
set key left top
plot '../benchmark_noise_256.csv' using 1:(filter("static", 6)) with linespoints lw 2 title "256x256", \
     '../benchmark_noise_512.csv' using 1:(filter("static", 6)) with linespoints lw 2 title "512x512", \
     '../benchmark_noise_1024.csv' using 1:(filter("static", 6)) with linespoints lw 2 title "1024x1024", \
     '../benchmark_noise_2048.csv' using 1:(filter("static", 6)) with linespoints lw 2 title "2048x2048", \
     '../benchmark_noise_4096.csv' using 1:(filter("static", 6)) with linespoints lw 2 title "4096x4096"

# -------------------------------------------------------------
# 2. RUIDO: COMPARACIÓN DE PLANIFICADORES (4096x4096)
# -------------------------------------------------------------
set output 'draws/noise_speedup_schedulers.pdf'
set title "Ruido 4096x4096: Static vs Dynamic" font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Speedup"
set key left top
plot '../benchmark_noise_4096.csv' using 1:(filter("static", 6)) with linespoints lw 2 title "Static", \
     '../benchmark_noise_4096.csv' using 1:(filter("dynamic", 6)) with linespoints lw 2 title "Dynamic"

# -------------------------------------------------------------
# 3. RUIDO: EFICIENCIA COMPARANDO TAMAÑOS (Planificador: static)
# -------------------------------------------------------------
set output 'draws/noise_efficiency.pdf'
set title "Eficiencia de Generación de Ruido (Planificador: Static)" font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Eficiencia (%)"
set key left bottom
plot '../benchmark_noise_256.csv' using 1:(filter("static", 7)) with linespoints lw 2 title "256x256", \
     '../benchmark_noise_512.csv' using 1:(filter("static", 7)) with linespoints lw 2 title "512x512", \
     '../benchmark_noise_1024.csv' using 1:(filter("static", 7)) with linespoints lw 2 title "1024x1024", \
     '../benchmark_noise_2048.csv' using 1:(filter("static", 7)) with linespoints lw 2 title "2048x2048", \
     '../benchmark_noise_4096.csv' using 1:(filter("static", 7)) with linespoints lw 2 title "4096x4096"

# -------------------------------------------------------------
# 4. EROSIÓN: SPEEDUP COMPARANDO PLANIFICADORES (4096x4096)
# -------------------------------------------------------------
set output 'draws/erosion_speedup.pdf'
set title "Speedup de Erosión Hidráulica (4096x4096)" font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Speedup (vs Secuencial)"
set key left top
plot '../benchmark_erosion_4096.csv' using 1:(filter("static", 7)) with linespoints lw 2 title "Atómico (Static)", \
     '../benchmark_erosion_4096.csv' using 1:(filter("dynamic", 7)) with linespoints lw 2 title "Atómico (Dynamic)", \
     '../benchmark_erosion_4096.csv' using 1:(filter("static", 8)) with linespoints lw 2 title "Local Buffers (Static)", \
     '../benchmark_erosion_4096.csv' using 1:(filter("dynamic", 8)) with linespoints lw 2 title "Local Buffers (Dynamic)"

# -------------------------------------------------------------
# 5. EROSIÓN: EFICIENCIA COMPARANDO PLANIFICADORES (4096x4096)
# -------------------------------------------------------------
set output 'draws/erosion_efficiency.pdf'
set title "Eficiencia de Erosión Hidráulica (4096x4096)" font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Eficiencia (%)"
set key left bottom
plot '../benchmark_erosion_4096.csv' using 1:(eff("static", 7)) with linespoints lw 2 title "Atómico (Static)", \
     '../benchmark_erosion_4096.csv' using 1:(eff("dynamic", 7)) with linespoints lw 2 title "Atómico (Dynamic)", \
     '../benchmark_erosion_4096.csv' using 1:(eff("static", 8)) with linespoints lw 2 title "Local Buffers (Static)", \
     '../benchmark_erosion_4096.csv' using 1:(eff("dynamic", 8)) with linespoints lw 2 title "Local Buffers (Dynamic)"

# -------------------------------------------------------------
# 6. EROSIÓN: COMPARATIVA DE TAMAÑOS (Local Buffers, Planificador: dynamic)
# -------------------------------------------------------------
set output 'draws/erosion_speedup_sizes.pdf'
set title "Erosión Local Buffers: Escalabilidad por Tamaño (Dynamic)" font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Speedup"
set key left top
plot '../benchmark_erosion_256.csv' using 1:(filter("dynamic", 8)) with linespoints lw 2 title "256x256", \
     '../benchmark_erosion_512.csv' using 1:(filter("dynamic", 8)) with linespoints lw 2 title "512x512", \
     '../benchmark_erosion_1024.csv' using 1:(filter("dynamic", 8)) with linespoints lw 2 title "1024x1024", \
     '../benchmark_erosion_2048.csv' using 1:(filter("dynamic", 8)) with linespoints lw 2 title "2048x2048", \
     '../benchmark_erosion_4096.csv' using 1:(filter("dynamic", 8)) with linespoints lw 2 title "4096x4096"

# -------------------------------------------------------------
# 7. EROSIÓN: TIEMPO DE EJECUCIÓN COMPARACIÓN (4096x4096, Planificador: dynamic)
# -------------------------------------------------------------
set output 'draws/erosion_time.pdf'
set title "Tiempo de Ejecución de Erosión Hidráulica (4096x4096, Dynamic)" font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Tiempo de Ejecución (ms)"
set logscale y
plot '../benchmark_erosion_4096.csv' using 1:(filter("dynamic", 4)) with linespoints lw 2 title "Secuencial (Línea Base)", \
     '../benchmark_erosion_4096.csv' using 1:(filter("dynamic", 5)) with linespoints lw 2 title "Paralelo (Atómico)", \
     '../benchmark_erosion_4096.csv' using 1:(filter("dynamic", 6)) with linespoints lw 2 title "Paralelo (Buffers Locales)"
unset logscale y

# -------------------------------------------------------------
# 8. EROSIÓN: DESGLOSE DE TIEMPOS EN LOCAL BUFFERS (4096x4096, Planificador: dynamic)
# -------------------------------------------------------------
set output 'draws/erosion_local_breakdown.pdf'
set title "Desglose de Tiempos en Erosión Local Buffers (4096x4096, Dynamic)" font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Tiempo de Ejecución (ms)"
plot '../benchmark_erosion_4096.csv' using 1:(filter("dynamic", 8)) with linespoints lw 2 title "Asignación de Memoria (Alloc)", \
     '../benchmark_erosion_4096.csv' using 1:(filter("dynamic", 9)) with linespoints lw 2 title "Simulación de Gotas (Sim)", \
     '../benchmark_erosion_4096.csv' using 1:(filter("dynamic", 10)) with linespoints lw 2 title "Reducción de Buffers (Reduc)"
