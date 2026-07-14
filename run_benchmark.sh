#!/bin/sh
# Script para compilar y ejecutar los benchmarks, con gráficos opcionales de gnuplot

# Asegurar que se detiene ante cualquier error en la compilación o ejecución
set -e

echo "* Compilando benchmarks..."
cmake --build build --target raymap_benchmark --parallel

echo "* Ejecutando benchmarks (esto puede tomar un par de minutos)..."
mkdir -p results/plot/draws
./build/raymap_benchmark results/

if command -v gnuplot >/dev/null 2>&1; then
    echo "* Generando gráficos con gnuplot..."
    cd results/plot
    gnuplot plot_benchmarks.gp
    cd ../..
    echo "* ¡Éxito! Gráficos guardados en resultados/plot/draws/"
else
    echo "=========================================================="
    echo "WARNING: 'gnuplot' no está instalado en el sistema."
    echo "Los datos CSV se guardaron en 'results/', pero no se"
    echo "pudieron generar los gráficos en PDF."
    echo "Puedes instalar gnuplot ejecutando: sudo apt install gnuplot"
    echo "=========================================================="
fi
