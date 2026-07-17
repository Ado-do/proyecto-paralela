# script gnuplot para graficar benchmarks

set datafile separator ","
set terminal pdf size 6,4 enhanced font 'Inter,11'

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
plot "< grep ',static,' ../benchmark_noise_256.csv" using 1:6 with linespoints lw 2 title "256x256", \
     "< grep ',static,' ../benchmark_noise_512.csv" using 1:6 with linespoints lw 2 title "512x512", \
     "< grep ',static,' ../benchmark_noise_1024.csv" using 1:6 with linespoints lw 2 title "1024x1024", \
     "< grep ',static,' ../benchmark_noise_2048.csv" using 1:6 with linespoints lw 2 title "2048x2048", \
     "< grep ',static,' ../benchmark_noise_4096.csv" using 1:6 with linespoints lw 2 title "4096x4096"

# -------------------------------------------------------------
# 2. RUIDO: COMPARACIÓN DE PLANIFICADORES (4096x4096)
# -------------------------------------------------------------
set output 'draws/noise_speedup_schedulers.pdf'
set title "Ruido 4096x4096: Speedupp (Static vs Dynamic)" font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Speedup"
set key left top
plot "< grep ',static,' ../benchmark_noise_4096.csv" using 1:6 with linespoints lw 2 title "Static", \
     "< grep ',dynamic,' ../benchmark_noise_4096.csv" using 1:6 with linespoints lw 2 title "Dynamic"

# -------------------------------------------------------------
# 3. RUIDO: EFICIENCIA COMPARANDO TAMAÑOS (Planificador: static)
# -------------------------------------------------------------
set output 'draws/noise_efficiency.pdf'
set title "Eficiencia de Generación de Ruido (Planificador: Static)" font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Eficiencia (%)"
set key left bottom
plot "< grep ',static,' ../benchmark_noise_256.csv" using 1:7 with linespoints lw 2 title "256x256", \
     "< grep ',static,' ../benchmark_noise_512.csv" using 1:7 with linespoints lw 2 title "512x512", \
     "< grep ',static,' ../benchmark_noise_1024.csv" using 1:7 with linespoints lw 2 title "1024x1024", \
     "< grep ',static,' ../benchmark_noise_2048.csv" using 1:7 with linespoints lw 2 title "2048x2048", \
     "< grep ',static,' ../benchmark_noise_4096.csv" using 1:7 with linespoints lw 2 title "4096x4096"

# -------------------------------------------------------------
# 4. EROSIÓN: SPEEDUP COMPARANDO PLANIFICADORES (4096x4096)
# -------------------------------------------------------------
set output 'draws/erosion_speedup.pdf'
set title "Speedup de Erosión Hidráulica (4096x4096)" font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Speedup (vs Secuencial)"
set key left top
plot "< grep ',static,' ../benchmark_erosion_4096.csv" using 1:7 with linespoints lw 2 title "Atómico (Static)", \
     "< grep ',dynamic,' ../benchmark_erosion_4096.csv" using 1:7 with linespoints lw 2 title "Atómico (Dynamic)", \
     "< grep ',static,' ../benchmark_erosion_4096.csv" using 1:8 with linespoints lw 2 title "Local Buffers (Static)", \
     "< grep ',dynamic,' ../benchmark_erosion_4096.csv" using 1:8 with linespoints lw 2 title "Local Buffers (Dynamic)"

# -------------------------------------------------------------
# 5. EROSIÓN: EFICIENCIA COMPARANDO PLANIFICADORES (4096x4096)
# -------------------------------------------------------------
set output 'draws/erosion_efficiency.pdf'
set title "Eficiencia de Erosión Hidráulica (4096x4096)" font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Eficiencia (%)"
set key left bottom
plot "< grep ',static,' ../benchmark_erosion_4096.csv" using 1:(($7 / $1) * 100.0) with linespoints lw 2 title "Atómico (Static)", \
     "< grep ',dynamic,' ../benchmark_erosion_4096.csv" using 1:(($7 / $1) * 100.0) with linespoints lw 2 title "Atómico (Dynamic)", \
     "< grep ',static,' ../benchmark_erosion_4096.csv" using 1:(($8 / $1) * 100.0) with linespoints lw 2 title "Local Buffers (Static)", \
     "< grep ',dynamic,' ../benchmark_erosion_4096.csv" using 1:(($8 / $1) * 100.0) with linespoints lw 2 title "Local Buffers (Dynamic)"

# -------------------------------------------------------------
# 6. EROSIÓN: COMPARATIVA DE TAMAÑOS (Local Buffers, Planificador: dynamic)
# -------------------------------------------------------------
set output 'draws/erosion_speedup_sizes.pdf'
set title "Erosión Local Buffers: Escalabilidad por Tamaño (Dynamic)" font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Speedup"
set key left top
plot "< grep ',dynamic,' ../benchmark_erosion_256.csv" using 1:8 with linespoints lw 2 title "256x256", \
     "< grep ',dynamic,' ../benchmark_erosion_512.csv" using 1:8 with linespoints lw 2 title "512x512", \
     "< grep ',dynamic,' ../benchmark_erosion_1024.csv" using 1:8 with linespoints lw 2 title "1024x1024", \
     "< grep ',dynamic,' ../benchmark_erosion_2048.csv" using 1:8 with linespoints lw 2 title "2048x2048", \
     "< grep ',dynamic,' ../benchmark_erosion_4096.csv" using 1:8 with linespoints lw 2 title "4096x4096"

# -------------------------------------------------------------
# 7. EROSIÓN: TIEMPO DE EJECUCIÓN COMPARACIÓN (4096x4096, Planificador: dynamic)
# -------------------------------------------------------------
set output 'draws/erosion_time.pdf'
set title "Tiempo de Ejecución de Erosión Hidráulica (4096x4096, Dynamic)" font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Tiempo de Ejecución (ms)"
set logscale y
plot "< grep ',dynamic,' ../benchmark_erosion_4096.csv" using 1:4 with linespoints lw 2 title "Secuencial (Línea Base)", \
     "< grep ',dynamic,' ../benchmark_erosion_4096.csv" using 1:5 with linespoints lw 2 title "Paralelo (Atómico)", \
     "< grep ',dynamic,' ../benchmark_erosion_4096.csv" using 1:6 with linespoints lw 2 title "Paralelo (Buffers Locales)"
unset logscale y

# -------------------------------------------------------------
# 8. EROSIÓN: DESGLOSE DE TIEMPOS EN LOCAL BUFFERS (4096x4096, Planificador: dynamic)
# -------------------------------------------------------------
set output 'draws/erosion_local_breakdown.pdf'
set title "Desglose de Tiempos en Erosión Local Buffers (4096x4096, Dynamic)" font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Tiempo de Ejecución (ms)"
plot "< grep ',dynamic,' ../benchmark_erosion_4096.csv" using 1:9 with linespoints lw 2 title "Asignación de Memoria (Alloc)", \
     "< grep ',dynamic,' ../benchmark_erosion_4096.csv" using 1:10 with linespoints lw 2 title "Simulación de Gotas (Sim)", \
     "< grep ',dynamic,' ../benchmark_erosion_4096.csv" using 1:11 with linespoints lw 2 title "Reducción de Buffers (Reduc)"
