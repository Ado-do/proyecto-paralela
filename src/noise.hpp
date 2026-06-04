#pragma once

#include <vector>

namespace noise {
// Suavizado de Ken Perlin
inline float fade(float t) { return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); }

// Interpolación lineal
inline float lerp(float t, float a, float b) { return a + t * (b - a); }

// Calcula el gradiente 2D según el hash
inline float grad(int hash, float x, float y) {
    int h = hash & 15;
    float u = (h < 8) ? x : y;
    float v = (h < 4) ? y : ((h == 12 || h == 14) ? x : 0.0f);
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

// Genera ruido de Perlin 2D base
float perlin2D(float x, float y, const std::vector<int> &p);

// Combina octavas de ruido para crear terreno rugoso (fBm)
float fbm2D(float x, float y, int octaves, float persistence, float lacunarity, const std::vector<int> &p);

// Crea una tabla de permutaciones aleatoria para el ruido
std::vector<int> generatePermutationTable(unsigned int seed);

} // namespace noise
