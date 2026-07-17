# Proyecto Semestral: Parallel Procedural Terrain Generation

Introducción a la Computación Paralela 2026-1 \
Alonso Bustos

<img width="600" height="338" alt="demo" src="https://github.com/user-attachments/assets/bcfbbf07-3d3c-4527-9760-352619709cd3" />

## Requerimientos

- Compilador C++17 (g++, clang++, etc)
- Raylib (Opcional, necesario para visualizador raymap)
- Gnuplot (Opcional, necesario para gráficar benchmarks)

## Uso

```sh
# Configurar cmake y preparar compilación
./setup.sh
#./setup -DBUILD_VISUALIZER=OFF # Agregar flag para no requerir raylib (necesario para servidores como colcura)

./run_test.sh      # Ejecutar tests
./run_raymap.sh    # Ejecutar raymap (Visualizador)
./run_benchmark.sh # Ejecutar benchmarks (no requiere raylib)
./run_profile.sh   # Ejecutar profiling
```
