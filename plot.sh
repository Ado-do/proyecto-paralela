#!/bin/sh
# Script para generar gráficos de gnuplot (benchmarks y profiling)
set -e

# Configuración de argumentos opcionales
RESULTS_DIR=${1:-results}
PLATFORM_NAME=${2:-Laptop}

if command -v gnuplot >/dev/null 2>&1; then
    echo "* Generando gráficos consolidados con gnuplot..."
    mkdir -p "${RESULTS_DIR}/plot/draws"
    
    # Graficar benchmarks estándar (genera noise_dashboard.pdf y erosion_dashboard.pdf)
    gnuplot -c results/plot/plot_benchmarks.gp "${RESULTS_DIR}" "${PLATFORM_NAME}"
    echo "* Tableros de control (dashboards) guardados en: ${RESULTS_DIR}/plot/draws/"
    
    # Graficar desglose de profiling de erosión si existe el archivo
    if [ -f "${RESULTS_DIR}/benchmark_erosion_4096.csv" ]; then
        gnuplot -c results/plot/plot_profiling.gp "${RESULTS_DIR}" "${PLATFORM_NAME}"
        echo "* Gráficos de perfilamiento guardados en: ${RESULTS_DIR}/plot/draws/"
    fi
else
    echo "* Programa gnuplot no está instalado en el sistema. Saltando gráficos."
fi
