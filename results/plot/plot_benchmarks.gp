# script gnuplot para graficar benchmarks

set datafile separator ","
set terminal pdf size 6,4 enhanced font 'Inter,11'

# Noise Speedup
set output 'draws/noise_speedup.pdf'
set title "Heightmap (Noise) Generation Speedup" font 'Inter-Bold,14'
set xlabel "Thread Count"
set ylabel "Speedup"
set grid
set key left top
plot '../benchmark_noise_256.csv' using 1:5 with linespoints lw 2 title "256x256", \
     '../benchmark_noise_512.csv' using 1:5 with linespoints lw 2 title "512x512", \
     '../benchmark_noise_1024.csv' using 1:5 with linespoints lw 2 title "1024x1024", \
     '../benchmark_noise_2048.csv' using 1:5 with linespoints lw 2 title "2048x2048", \
     '../benchmark_noise_4096.csv' using 1:5 with linespoints lw 2 title "4096x4096"

# Erosion Speedup
set output 'draws/erosion_speedup.pdf'
set title "Hydraulic Erosion Speedup (4096x4096)" font 'Inter-Bold,14'
set xlabel "Thread Count"
set ylabel "Speedup (vs Sequential)"
set grid
set key left top
plot '../benchmark_erosion_4096.csv' using 1:6 with linespoints lw 2 title "Parallel (Atomic)", \
     '../benchmark_erosion_4096.csv' using 1:7 with linespoints lw 2 title "Parallel (Local Buffers)"

# Erosion Execution Time Comparison
set output 'draws/erosion_time.pdf'
set title "Hydraulic Erosion Execution Time (4096x4096)" font 'Inter-Bold,14'
set xlabel "Thread Count"
set ylabel "Execution Time (ms)"
set grid
set key left top
plot '../benchmark_erosion_4096.csv' using 1:3 with linespoints lw 2 title "Sequential (Baseline)", \
     '../benchmark_erosion_4096.csv' using 1:4 with linespoints lw 2 title "Parallel (Atomic)", \
     '../benchmark_erosion_4096.csv' using 1:5 with linespoints lw 2 title "Parallel (Local Buffers)"

