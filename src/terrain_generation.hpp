#pragma once

#include <vector>

namespace TerrainGen {
// Función de suavizado de Ken Perlin (más suave que interpolación estándar)
inline float fade(float t) { return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); }

// Interpolación estándar
inline float lerp(float t, float a, float b) { return a + t * (b - a); }

// Convierte valor hash en un vector gradiente direccional
inline float grad(int hash, float x, float y) {
    int h = hash & 15; // últimos 4 bits
    float u = (h < 8) ? x : y;
    float v = ((h < 4) ? y : h == 12 || ((h == 14) ? x : 0.0f));
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

// --- Función Perlin principal ---
// Ruido de Perlin 2D, puro sin estados
// Toma la tabla de permutación de solo-lectura 'p', como referencia
float perlin2D(float x, float y, const std::vector<int> &p);

// --- Wrapper del Fractal Browninan Motion ---
// Combina varias capas de octavas del ruido de perlan para crear terreno rugoso.
float fbm2D(float x, float y, int octaves, float persistence, float lacunarity, const std::vector<int> &p);

// --- Generador de tabla de permutaciones ---
// Llamar una vez antes del loop paralelo
std::vector<int> generatePermutationTable(unsigned int seed);

} // namespace TerrainGen
