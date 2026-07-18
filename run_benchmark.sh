#!/bin/sh
# Script unificado para compilar, ejecutar benchmarks y perfilar el código
set -e

# Valores por defecto
RESULTS_DIR="results"
PLATFORM_NAME="Laptop"
PROFILE=false

# Procesar argumentos
while [ $# -gt 0 ]; do
    case "$1" in
        --profile|-p)
            PROFILE=true
            shift
            ;;
        *)
            if [ -z "$RESULTS_DIR_SET" ]; then
                RESULTS_DIR="$1"
                RESULTS_DIR_SET=true
            elif [ -z "$PLATFORM_NAME_SET" ]; then
                PLATFORM_NAME="$1"
                PLATFORM_NAME_SET=true
            else
                echo "Argumento desconocido: $1"
                exit 1
            fi
            shift
            ;;
    esac
done

echo "* INICIANDO PROCESO (Perfilado: ${PROFILE}, Plataforma: ${PLATFORM_NAME}, Destino: ${RESULTS_DIR})"

if [ "$PROFILE" = true ]; then
    if ! command -v gprof >/dev/null 2>&1; then
        echo "ERROR: 'gprof' no está instalado en el sistema. Imposible perfilar."
        exit 1
    fi
    echo "* Configurando compilación con soporte para profiling (-g -pg)..."
    cmake -B build -DENABLE_PROFILING=ON -DCMAKE_BUILD_TYPE=Release
else
    echo "* Configurando compilación estándar (sin flags de profiling)..."
    cmake -B build -DENABLE_PROFILING=OFF -DCMAKE_BUILD_TYPE=Release
fi

echo "* Compilando benchmarks..."
cmake --build build --target raymap_benchmark --parallel

echo "* Ejecutando benchmarks..."
mkdir -p "${RESULTS_DIR}/plot/draws"
./build/raymap_benchmark "${RESULTS_DIR}/"

if [ "$PROFILE" = true ]; then
    echo "* Generando reporte detallado con gprof..."
    gprof ./build/raymap_benchmark gmon.out > "${RESULTS_DIR}/profile_report.txt"
    echo "* Reporte de gprof guardado -> ${RESULTS_DIR}/profile_report.txt"
    
    echo "* Restaurando configuración de compilación estándar (sin flags de profiling)..."
    cmake -B build -DENABLE_PROFILING=OFF -DCMAKE_BUILD_TYPE=Release
fi

# Invocar al script externo de graficado
if [ -f "./plot.sh" ]; then
    chmod +x ./plot.sh
    ./plot.sh "${RESULTS_DIR}" "${PLATFORM_NAME}"
else
    echo "WARNING: No se encontró plot.sh para generar los gráficos."
fi

echo "* PROCESO FINALIZADO"
