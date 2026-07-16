# script gnuplot para comparar plataformas (Local Alder Lake vs Colcura)
# Uso: gnuplot -c compare_platforms.gp <colcura_dir>

set datafile separator ","
set terminal pdf size 6,4 enhanced font 'Inter,11'

if (ARGC >= 1) {
    colcura_dir = ARG1
} else {
    colcura_dir = '../../results-colcura'
}

# Funciones de filtrado
filter(sched, col) = (strcol(3) eq sched ? column(col) : 1/0)
eff(sched, col_speed) = (strcol(3) eq sched ? (column(col_speed) / column(1)) * 100.0 : 1/0)

set offsets 0.2, 0.2, 0, 0
set grid

# 1. Comparar Speedup de Ruido (4096x4096, Static)
set output 'draws/platform_noise_speedup.pdf'
set title "Ruido 4096x4096: Speedup Laptop vs Clúster Colcura" font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Speedup"
set key left top
plot '../benchmark_noise_4096.csv' using 1:(filter("static", 6)) with linespoints lw 2 title "Laptop (12 Cores, Híbrido)", \
     colcura_dir.'/benchmark_noise_4096.csv' using 1:(filter("static", 6)) with linespoints lw 2 title "Colcura (64 Cores, Homogéneo)"

# 2. Comparar Eficiencia de Ruido (4096x4096, Static)
set output 'draws/platform_noise_efficiency.pdf'
set title "Ruido 4096x4096: Eficiencia Laptop vs Clúster Colcura" font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Eficiencia (%)"
set key left bottom
plot '../benchmark_noise_4096.csv' using 1:(filter("static", 7)) with linespoints lw 2 title "Laptop (12 Cores, Híbrido)", \
     colcura_dir.'/benchmark_noise_4096.csv' using 1:(filter("static", 7)) with linespoints lw 2 title "Colcura (64 Cores, Homogéneo)"

# 3. Comparar Speedup de Erosión (4096x4096, Local Buffers, Dynamic)
set output 'draws/platform_erosion_speedup.pdf'
set title "Erosión 4096x4096: Speedup Laptop vs Clúster Colcura" font 'Inter-Bold,14'
set xlabel "Cantidad de Hilos"
set ylabel "Speedup (vs Secuencial)"
set key left top
plot '../benchmark_erosion_4096.csv' using 1:(filter("dynamic", 8)) with linespoints lw 2 title "Laptop (Local Buffers, Dynamic)", \
     colcura_dir.'/benchmark_erosion_4096.csv' using 1:(filter("dynamic", 8)) with linespoints lw 2 title "Colcura (Local Buffers, Dynamic)", \
     '../benchmark_erosion_4096.csv' using 1:(filter("dynamic", 7)) with linespoints lw 2 title "Laptop (Atómico, Dynamic)", \
     colcura_dir.'/benchmark_erosion_4096.csv' using 1:(filter("dynamic", 7)) with linespoints lw 2 title "Colcura (Atómico, Dynamic)"
