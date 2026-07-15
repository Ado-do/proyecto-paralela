#include "terrain.hpp"
#include "benchmark_config.hpp"
#include "raymap_config.hpp"
#include <iostream>

using namespace std;

Terrain::Terrain(int size, unsigned int seed, int octaves) 
    : m_heightmap(size, seed, octaves), m_usingColor(true), m_texturesLoaded(false) {
}

Terrain::~Terrain() {
    if (m_texturesLoaded) {
        UnloadTexture(m_texDebug);
        UnloadTexture(m_texColor);
    }
}

Image Terrain::createHeightImage() {
    int size = m_heightmap.getSize();
    const auto& data = m_heightmap.getData();
    Image image = GenImageColor(size, size, BLACK);
    Color* pixels = (Color*)image.data;
    for (int i = 0; i < size * size; i++) {
        unsigned char val = (unsigned char)((data[i] + 1.0f) * 0.5f * 255.0f);
        pixels[i] = (Color){ val, val, val, 255 };
    }
    return image;
}

Image Terrain::createColorImage() {
    int size = m_heightmap.getSize();
    const auto& data = m_heightmap.getData();
    Image image = GenImageColor(size, size, BLACK);
    Color* pixels = (Color*)image.data;

    for (int i = 0; i < size * size; i++) {
        float h = data[i];
        Color c;

        if (h < heightDeepWater)         c = colorDeepWater;
        else if (h < heightShallowWater) c = colorShallowWater;
        else if (h < heightSand)         c = colorSand;
        else if (h < heightGrass)        c = colorGrass;
        else if (h < heightRock)         c = colorRock;
        else c = colorSnow;

        pixels[i] = c;
    }
    return image;
}

Model Terrain::createModel() {
    if (m_texturesLoaded) {
        UnloadTexture(m_texDebug);
        UnloadTexture(m_texColor);
    }

    Image imgHeight = createHeightImage();
    Image imgColor = createColorImage();

    m_texDebug = LoadTextureFromImage(imgHeight);
    m_texColor = LoadTextureFromImage(imgColor);
    m_texturesLoaded = true;

    Mesh mesh = GenMeshHeightmap(imgHeight, (Vector3){ meshWidth, meshHeight, meshLength });
    Model model = LoadModelFromMesh(mesh);

    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m_usingColor ? m_texColor : m_texDebug;

    UnloadImage(imgHeight);
    UnloadImage(imgColor);
    return model;
}

void Terrain::toggleTexture(Model& model) {
    m_usingColor = !m_usingColor;
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m_usingColor ? m_texColor : m_texDebug;
    cout << "Modo de visualización: " << (m_usingColor ? "Realista" : "Debug") << endl;
}
