# script gnuplot para graficar benchmarks

set datafile separator ","
set terminal pdf size 6,4 enhanced font 'Inter,11'

# Margen en los extremos del eje X para evitar que los puntos toquen los bordes
set offsets 0.2, 0.2, 0, 0

# - Noise Speedup
set output 'draws/noise_speedup.pdf'
set title "Speedup de Generación de Ruido (Heightmap)" font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Speedup"
set grid
set key left top
plot '../benchmark_noise_256.csv' using 0:5:xtic(1) with linespoints lw 2 title "256x256", \
     '../benchmark_noise_512.csv' using 0:5:xtic(1) with linespoints lw 2 title "512x512", \
     '../benchmark_noise_1024.csv' using 0:5:xtic(1) with linespoints lw 2 title "1024x1024", \
     '../benchmark_noise_2048.csv' using 0:5:xtic(1) with linespoints lw 2 title "2048x2048", \
     '../benchmark_noise_4096.csv' using 0:5:xtic(1) with linespoints lw 2 title "4096x4096"

# - Erosion Speedup
set output 'draws/erosion_speedup.pdf'
set title "Speedup de Erosión Hidráulica (4096x4096)" font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Speedup (vs Secuencial)"
set grid
set key left top
plot '../benchmark_erosion_4096.csv' using 0:6:xtic(1) with linespoints lw 2 title "Paralelo (Atómico)", \
     '../benchmark_erosion_4096.csv' using 0:7:xtic(1) with linespoints lw 2 title "Paralelo (Buffers Locales)"

# - Erosion Execution Time Comparison
set output 'draws/erosion_time.pdf'
set title "Tiempo de Ejecución de Erosión Hidráulica (4096x4096)" font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Tiempo de Ejecución (ms)"
set logscale y
set grid
set key left top
plot '../benchmark_erosion_4096.csv' using 0:3:xtic(1) with linespoints lw 2 title "Secuencial (Línea Base)", \
     '../benchmark_erosion_4096.csv' using 0:4:xtic(1) with linespoints lw 2 title "Paralelo (Atómico)", \
     '../benchmark_erosion_4096.csv' using 0:5:xtic(1) with linespoints lw 2 title "Paralelo (Buffers Locales)"
