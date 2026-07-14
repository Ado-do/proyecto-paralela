#pragma once
#include "raylib.h"
#include "heightmap.hpp"

class Terrain {
public:
    Terrain(int size, unsigned int seed, int octaves = 8);
    ~Terrain();

    // Wrappers para interactuar con el motor de cálculo desacoplado
    BenchmarkResults runBenchmark() { return m_heightmap.runBenchmark(); }
    double applyErosion(ErosionMode mode) { return m_heightmap.applyErosion(mode); }
    void regenerate(unsigned int newSeed) { m_heightmap.regenerate(newSeed); }
    void setOctaves(int octaves) { m_heightmap.setOctaves(octaves); }
    const std::vector<float>& getData() const { return m_heightmap.getData(); }

    // Métodos específicos de Raylib para renderizado 3D
    Model createModel();
    void toggleTexture(Model& model);

private:
    Image createHeightImage();
    Image createColorImage();

    Heightmap m_heightmap;

    Texture2D m_texDebug;   // Textura en escala de grises
    Texture2D m_texColor;   // Textura con biomas reales
    bool m_usingColor;      // Estado actual de la visualización
    bool m_texturesLoaded;
};
