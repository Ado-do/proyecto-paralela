#pragma once
#include <vector>
#include "raylib.h"

struct BenchmarkResults {
    double timeSequential;
    double timeParallel;
    double speedup;
    double efficiency;
    double cost;
    int numThreads;
    
    // Parámetros del terreno utilizados
    unsigned int seed;
    float scale;
    int octaves;
    float persistence;
    float lacunarity;
};

class Terrain {
public:
    Terrain(int size, unsigned int seed, int octaves = 8);
    ~Terrain();

    // Ejecuta ambos experimentos y devuelve los resultados de tiempo
    BenchmarkResults runBenchmark();

    // Regenera los datos con una nueva semilla
    void regenerate(unsigned int newSeed);

    // Configura el número de octavas y regenera la tabla si es necesario (aunque no es estrictamente necesario, actualiza el estado)
    void setOctaves(int octaves);

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
    int m_octaves;
    std::vector<float> m_data;
    std::vector<int> m_pTable;

    Texture2D m_texDebug;   // Textura en escala de grises
    Texture2D m_texColor;   // Textura con biomas reales
    bool m_usingColor;      // Estado actual de la visualización
    bool m_texturesLoaded;
};
