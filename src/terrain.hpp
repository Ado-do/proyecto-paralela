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

    // Regenera los datos con una nueva semilla
    void regenerate(unsigned int newSeed);

    // Genera el modelo 3D y las texturas (color y debug)
    Model createModel();

    // Intercambia entre textura realista y de debug
    void toggleTexture(Model& model);

private:
    void generateSequential();
    void generateParallel();
    Image createHeightImage();
    Image createColorImage(); // Nueva función para biomas

    int m_size;
    unsigned int m_seed;
    std::vector<float> m_data;
    std::vector<int> m_pTable;

    Texture2D m_texDebug;   // Textura en escala de grises
    Texture2D m_texColor;   // Textura con biomas reales
    bool m_usingColor;      // Estado actual de la visualización
    bool m_texturesLoaded;
};
