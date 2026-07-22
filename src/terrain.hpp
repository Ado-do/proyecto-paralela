#pragma once
#include "raylib.h"
#include "heightmap.hpp"

class Terrain {
public:
    Terrain(int size, unsigned int seed, int octaves = 8);
    ~Terrain();

    // Wrappers para interactuar con el motor de cálculo desacoplado
    BenchmarkResults runBenchmark(bool quick = false) { return m_heightmap.runBenchmark(quick); }
    double applyErosion(ErosionMode mode) { return m_heightmap.applyErosion(mode); }
    void regenerate(unsigned int newSeed) { m_heightmap.regenerate(newSeed); }
    void setOctaves(int octaves) { m_heightmap.setOctaves(octaves); }
    const std::vector<float>& getData() const { return m_heightmap.getData(); }

    // Métodos específicos de Raylib para renderizado 3D
    Model createModel();
    void updateModel(Model& model);
    void toggleTexture(Model& model);
    bool isUsingColor() const { return m_usingColor; }

    // Getters para texturas de depuración y color
    Texture2D getTexDebug() const { return m_texDebug; }
    Texture2D getTexColor() const { return m_texColor; }

private:
    Image createHeightImage();
    Image createColorImage();

    Heightmap m_heightmap;

    Texture2D m_texDebug;   // Textura en escala de grises
    Texture2D m_texColor;   // Textura con biomas reales
    bool m_usingColor;      // Estado actual de la visualización
    bool m_texturesLoaded;
};
