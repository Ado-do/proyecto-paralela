#!/bin/sh
cmake --build build --target raymap_benchmark --parallel && \
mkdir -p results/plot/draws && \
./build/raymap_benchmark results/ && \
cd results/plot && gnuplot plot_benchmarks.gp
