#include "noise.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <random>
#include <vector>

using namespace std;

namespace noise {

float perlin2D(float x, float y, const vector<int> &p) {
    float floorX = floor(x);
    float floorY = floor(y);

    // Determina la celda en la grilla (0-255) usando AND binario como módulo 256
    int X = static_cast<int>(floorX) & 255;
    int Y = static_cast<int>(floorY) & 255;

    // Calcula la posición relativa (0.0 a 1.0) dentro de la celda unitaria
    x -= floorX;
    y -= floorY;

    // Aplica la curva de suavizado (fade) para asegurar continuidad C2 en los bordes
    float u = fade(x);
    float v = fade(y);

    // Hashing de las 4 esquinas: combina índices de permutación para obtener valores únicos
    int A = p[X] + Y;
    int AA = p[A];
    int AB = p[A + 1];
    int B = p[X + 1] + Y;
    int BA = p[B];
    int BB = p[B + 1];

    // Interpolar linealmente (lerp) los productos punto de los gradientes en cada esquina
    return lerp(v,
                lerp(u, grad(p[AA], x, y), grad(p[BA], x - 1.0f, y)),
                lerp(u, grad(p[AB], x, y - 1.0f), grad(p[BB], x - 1.0f, y - 1.0f)));
}

float fbm2D(float x, float y, int octaves, float persistence, float lacunarity, const vector<int> &p) {
    float total = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;

    // Acumulación fractal: suma octavas con frecuencia creciente y amplitud decreciente
    for (int i = 0; i < octaves; ++i) {
        total += perlin2D(x * frequency, y * frequency, p) * amplitude;
        amplitude *= persistence; // Controla la rugosidad (comúnmente 0.5)
        frequency *= lacunarity;  // Controla el detalle (comúnmente 2.0)
    }

    // Suma de progresión geométrica para normalizar
    float maxValue = (persistence == 1.0f) ? static_cast<float>(octaves) 
                                            : (1.0f - pow(persistence, octaves)) / (1.0f - persistence);

    // Retorna el valor normalizado al rango [-1, 1]
    return total / maxValue;
}

vector<int> generatePermutationTable(unsigned seed) {
    vector<int> p(256);
    iota(p.begin(), p.end(), 0); // Inicializa con valores secuenciales del 0 al 255

    mt19937 engine(seed);
    shuffle(p.begin(), p.end(), engine); // Desordena los valores para aleatoriedad determinista

    // Duplica la tabla a 512 para evitar el uso de 'X % 256' o 'X & 255' en accesos p[X+1]
    p.insert(p.end(), p.begin(), p.end());

    return p;
}
} // namespace noise
