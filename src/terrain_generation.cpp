#include "terrain_generation.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include <vector>

using namespace std;

namespace TerrainGen {

float perlin2D(float x, float y, const vector<int> &p) {
    // Encuentra coordenadas en grilla unitaria de tamaño 256
    int X = static_cast<int>(floor(x)) & 255;
    int Y = static_cast<int>(floor(y)) & 255;

    // Posiciones x, y relativas, dentro del cuadrado de la grilla
    x -= floor(x);
    y -= floor(y);

    // Computar lineas de suavizado
    float u = fade(x);
    float v = fade(y);

    // Coordenadas hash de las 4 esquinas
    int A = p[X] + Y;
    int AA = p[A];
    int AB = p[A + 1];
    int B = p[X + 1] + Y;
    int BA = p[B];
    int BB = p[B + 1];

    // Añade resultados combinados de las 4 esquinas
    return lerp(v,
                lerp(u, grad(p[AA], x, y), grad(p[BA], x - 1.0f, y)),
                lerp(u, grad(p[AB], x, y - 1.0f), grad(p[BB], x - 1.0f, y - 1.0f)));
}

float fbm2D(float x, float y, int octaves, float persistence, float lacunarity, const vector<int> &p) {
    float total = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float maxValue = 0.0f; // Used for normalizing the result to -1.0 to 1.0

    for (int i = 0; i < octaves; ++i) {
        total += perlin2D(x * frequency, y * frequency, p) * amplitude;

        maxValue += amplitude;

        amplitude *= persistence; // Decreases amplitude per octave (e.g., 0.5)
        frequency *= lacunarity;  // Increases frequency per octave (e.g., 2.0)
    }

    // Normalize the result so it stays within expected bounds
    return total / maxValue;
}

vector<int> generatePermutationTable(unsigned seed) {
    vector<int> p(256);
    iota(p.begin(), p.end(), 0); // LLenar con 0 - ... - 255

    mt19937 engine(seed);
    shuffle(p.begin(), p.end(), engine);

    // Duplicate the array to 512 elements.
    // This is a standard Perlin optimization to avoid having to use the expensive
    // modulo operator (%) when wrapping coordinates in the noise function.
    p.insert(p.end(), p.begin(), p.end());

    return p;
}
} // namespace TerrainGen
