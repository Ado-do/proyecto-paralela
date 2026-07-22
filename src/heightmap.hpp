#pragma once
#include <vector>
#include <cstdint>

enum class ErosionMode { SEQUENTIAL, PARALLEL_ATOMIC, PARALLEL_LOCAL_BUFFERS };

// Función de hash de 32 bits (SplitMix32) para generación determinista de alta velocidad
inline uint32_t hash32(uint32_t x) {
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;
    return x;
}

// Genera las coordenadas iniciales deterministas (posX, posY) de la gota i
inline void getDropletInitialPosition(int dropletIndex, unsigned int seed, int size, float &posX, float &posY) {
    uint32_t hX = hash32(seed ^ (static_cast<uint32_t>(dropletIndex) * 2u + 1u));
    uint32_t hY = hash32(seed ^ (static_cast<uint32_t>(dropletIndex) * 2u + 2u));
    posX = (static_cast<float>(hX) / 4294967296.0f) * (static_cast<float>(size) - 1.0001f);
    posY = (static_cast<float>(hY) / 4294967296.0f) * (static_cast<float>(size) - 1.0001f);
}


// Estructura para registrar los tiempos de las sub-etapas de erosión
struct ErosionProfile {
    double totalTime = 0.0;
    double allocationTime = 0.0; // Tiempo de asignación de buffers locales
    double simulationTime = 0.0; // Tiempo de simulación de las gotas
    double reductionTime = 0.0;  // Tiempo de reducción de los buffers locales a la grilla
};

struct BenchmarkResults {
    double timeSequential;
    double timeParallel;
    double speedup;
    double efficiency;
    double cost;
    int numThreads;

    unsigned int seed;
    float scale;
    int octaves;
    float persistence;
    float lacunarity;

    double timeErosion;
    ErosionMode erosionMode;
};

class Heightmap {
public:
    Heightmap(int size, unsigned int seed, int octaves = 8);

    BenchmarkResults runBenchmark(bool quick = false);
    double applyErosion(ErosionMode mode, ErosionProfile *profile = nullptr);
    void regenerate(unsigned int newSeed);
    void setOctaves(int octaves);
    void resetGrid();

    const std::vector<float> &getData() const { return m_data; }
    int getSize() const { return m_size; }
    unsigned int getSeed() const { return m_seed; }
    int getOctaves() const { return m_octaves; }

    void generateSequential();
    void generateParallel();

private:

    int m_size;
    unsigned int m_seed;
    int m_octaves;
    std::vector<float> m_data;
    std::vector<int> m_pTable;
};
