#!/bin/sh
# Script para compilar y ejecutar las pruebas unitarias, admitiendo flags adicionales de CMake

# Asegurar que se detiene ante cualquier error
set -e

echo "* REALIZANDO TESTS"

# Configurar cmake (admite flags adicionales del usuario como -DBUILD_VISUALIZER=OFF)
echo "* Configurando compilación..."
cmake -B build -DCMAKE_BUILD_TYPE=Release "$@"

echo "* Compilando tests..."
cmake --build build --target raymap_tests --parallel

echo "* Ejecutando tests..."
cd build
ctest --output-on-failure
cd ..

echo "* TESTS FINALIZADOS"
