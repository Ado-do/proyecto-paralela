#pragma once
#include <vector>

enum class ErosionMode {
    SEQUENTIAL,
    PARALLEL_ATOMIC,
    PARALLEL_LOCAL_BUFFERS
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
    
    BenchmarkResults runBenchmark();
    double applyErosion(ErosionMode mode);
    void regenerate(unsigned int newSeed);
    void setOctaves(int octaves);
    
    const std::vector<float>& getData() const { return m_data; }
    int getSize() const { return m_size; }
    unsigned int getSeed() const { return m_seed; }
    int getOctaves() const { return m_octaves; }

private:
    void generateSequential();
    void generateParallel();

    int m_size;
    unsigned int m_seed;
    int m_octaves;
    std::vector<float> m_data;
    std::vector<int> m_pTable;
};
