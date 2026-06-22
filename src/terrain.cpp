#include "terrain.hpp"
#include "noise.hpp"
#include "config.hpp"
#include <chrono>
#include <omp.h>
#include <iostream>

using namespace std;

Terrain::Terrain(int size, unsigned int seed, int octaves) 
    : m_size(size), m_seed(seed), m_octaves(octaves), m_data(size * size), 
      m_usingColor(true), m_texturesLoaded(false) {
    m_pTable = noise::generatePermutationTable(m_seed);
}

Terrain::~Terrain() {
    if (m_texturesLoaded) {
        UnloadTexture(m_texDebug);
        UnloadTexture(m_texColor);
    }
}

void Terrain::generateSequential() {
    for (int y = 0; y < m_size; y++) {
        for (int x = 0; x < m_size; x++) {
            float nx = (float)x / (m_size - 1);
            float ny = (float)y / (m_size - 1);
            m_data[y * m_size + x] = noise::fbm2D(nx * terrainNoiseScale, ny * terrainNoiseScale, 
                                                m_octaves, terrainPersistence, terrainLacunarity, m_pTable);
        }
    }
}

void Terrain::generateParallel() {
    // #pragma omp parallel for schedule(dynamic)
    #pragma omp parallel for collapse(2)
    for (int y = 0; y < m_size; y++) {
        for (int x = 0; x < m_size; x++) {
            float nx = (float)x / (m_size - 1);
            float ny = (float)y / (m_size - 1);
            m_data[y * m_size + x] = noise::fbm2D(nx * terrainNoiseScale, ny * terrainNoiseScale, 
                                                m_octaves, terrainPersistence, terrainLacunarity, m_pTable);
        }
    }
}

BenchmarkResults Terrain::runBenchmark() {
    cout << "Starting benchmark (Size: " << m_size << "x" << m_size << ", Threads: " << omp_get_max_threads() << ", Octaves: " << m_octaves << ")..." << endl;

    int numThreads = omp_get_max_threads();

    auto startSeq = chrono::high_resolution_clock::now();
    generateSequential();
    auto endSeq = chrono::high_resolution_clock::now();
    double tSeq = chrono::duration<double, milli>(endSeq - startSeq).count();

    auto startPar = chrono::high_resolution_clock::now();
    generateParallel();
    auto endPar = chrono::high_resolution_clock::now();
    double tPar = chrono::duration<double, milli>(endPar - startPar).count();

    double speedup = tSeq / tPar;
    double efficiency = (speedup / numThreads) * 100.0;
    double cost = numThreads * tPar;

    return { 
        tSeq, tPar, speedup, efficiency, cost, numThreads,
        m_seed, terrainNoiseScale, m_octaves, terrainPersistence, terrainLacunarity 
    };
}

void Terrain::regenerate(unsigned int newSeed) {
    m_seed = newSeed;
    m_pTable = noise::generatePermutationTable(m_seed);
}

void Terrain::setOctaves(int octaves) {
    m_octaves = octaves;
}

Image Terrain::createHeightImage() {
    Image image = GenImageColor(m_size, m_size, BLACK);
    Color* pixels = (Color*)image.data;
    for (int i = 0; i < m_size * m_size; i++) {
        unsigned char val = (unsigned char)((m_data[i] + 1.0f) * 0.5f * 255.0f);
        pixels[i] = (Color){ val, val, val, 255 };
    }
    return image;
}

Image Terrain::createColorImage() {
    Image image = GenImageColor(m_size, m_size, BLACK);
    Color* pixels = (Color*)image.data;

    for (int i = 0; i < m_size * m_size; i++) {
        float h = m_data[i];
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
    // Liberar texturas previas si existen
    if (m_texturesLoaded) {
        UnloadTexture(m_texDebug);
        UnloadTexture(m_texColor);
    }

    // Generar imágenes
    Image imgHeight = createHeightImage();
    Image imgColor = createColorImage();

    // Cargar texturas en GPU
    m_texDebug = LoadTextureFromImage(imgHeight);
    m_texColor = LoadTextureFromImage(imgColor);
    m_texturesLoaded = true;

    // Crear el Mesh usando la imagen de altura
    Mesh mesh = GenMeshHeightmap(imgHeight, (Vector3){ meshWidth, meshHeight, meshLength });
    Model model = LoadModelFromMesh(mesh);

    // Aplicar textura según estado actual
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
