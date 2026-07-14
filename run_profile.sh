#!/bin/sh
# Script para compilar con profiling y ejecutar gprof de forma automatizada

# Asegurar que se detiene ante cualquier error
set -e

echo "* REALIZANDO PROFILING"

# Comprobar si gprof está instalado
if ! command -v gprof >/dev/null 2>&1; then
    echo "WARNING: 'gprof' no está instalado en el sistema."
    exit 1
fi

# Configurar cmake con profiling habilitado
echo "Configurando compilación en CMake con soporte para profiling (-g -pg)..."
cmake -B build -DENABLE_PROFILING=ON -DCMAKE_BUILD_TYPE=Release

echo "Compilando benchmark..."
cmake --build build --target raymap_benchmark --parallel

echo "Ejecutando el benchmark (archivo gmon.out generado)..."
./build/raymap_benchmark results/

echo "* Generando reporte detallado con gprof..."
mkdir -p results
gprof ./build/raymap_benchmark gmon.out > results/profile_report.txt
echo "* Reporte de gprof guardado -> results/profile_report.txt"

echo "* Generando gráficos de perfilamiento..."
cd results/plot && gnuplot plot_profiling.gp && cd ../..
echo "* Gráfico de perfilamiento guardado -> results/plot/draws/erosion_local_breakdown.pdf"

# Restaurar cmake normal
echo "* Restaurando configuración de compilación estándar (sin flags de profiling)..."
cmake -B build -DENABLE_PROFILING=OFF -DCMAKE_BUILD_TYPE=Release

echo "* PROFILING FINALIZADO!"
