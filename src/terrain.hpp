#pragma once
#include <vector>
#include "raylib.h"

struct BenchmarkResults {
    double timeSequential;
    double timeParallel;
    double speedup;
};

class Terrain {
public:
    Terrain(int size, unsigned int seed);
    ~Terrain();

    // Ejecuta ambos experimentos y devuelve los resultados de tiempo
    BenchmarkResults runBenchmark();

    // Genera el modelo 3D para Raylib
    Model createModel();

private:
    void generateSequential();
    void generateParallel();
    Image createHeightImage();

    int m_size;
    unsigned int m_seed;
    std::vector<float> m_data;
    std::vector<int> m_pTable;

    Texture2D m_texture; // Guardamos la textura para poder liberarla
    bool m_textureLoaded;
};
