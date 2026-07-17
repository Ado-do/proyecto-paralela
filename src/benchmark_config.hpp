#pragma once
#include <vector>

// Parámetros específicos para la ejecución de benchmarks
inline const std::vector<int> benchmarkSizes = {256, 512, 1024, 2048, 4096};
inline const int benchmarkWarmups = 2;
inline const int benchmarkRuns = 5;
