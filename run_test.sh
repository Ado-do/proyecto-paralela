#!/bin/sh

echo "* REALIZANDO TESTS"

echo "* Compilando tests..."
cmake --build build --target raymap_tests --parallel

echo "* Ejecutando tests..."
cd build
ctest --output-on-failure

echo "* TESTS FINALIZADOS"
