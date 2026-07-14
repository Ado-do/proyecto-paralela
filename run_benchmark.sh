#!/bin/sh
echo "* REALIZANDO BENCHMARK"

echo "* Compilando benchmark..."
cmake --build build --target raymap_benchmark --parallel

echo "* Corriendo benchmarks..."
mkdir -p results/plot/draws
./build/raymap_benchmark results/

echo "* Guardando gráficos con results/plot/draws/..."
cd results/plot
gnuplot plot_benchmarks.gp

echo "* BENCHMARK FINALIZADO"
