#!/bin/sh
# Script para compilar y ejecutar los benchmarks, con gráficos opcionales de gnuplot

# Asegurar que se detiene ante cualquier error en la compilación o ejecución
set -e

echo "* REALIZANDO BENCHMARK"

echo "* Compilando benchmarks..."
cmake --build build --target raymap_benchmark --parallel

echo "* Ejecutando benchmarks..."
mkdir -p results/plot/draws
./build/raymap_benchmark results/

if command -v gnuplot >/dev/null 2>&1; then
    echo "* Generando gráficos con gnuplot..."
    cd results/plot
    gnuplot plot_benchmarks.gp
    cd ../..
    echo "* Gráficos guardados en resultados/plot/draws/"
else
    echo "* Programa gnuplot no está instalado en el sistema. Saltando gráficos."
fi

echo "* BENCHMARK FINALIZADO"
